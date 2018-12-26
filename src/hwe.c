////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/hwe.c
/// \brief      Heterogeneous Word Embedding.
///
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \author     Mu Yang <<emfomy@gmail.com>>
///
/// \note       This code is modified from `word2vec.c` of Google Inc.
///

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  License of `word2vec.c`
//
//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define MAX_STRING 500
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000
#define MAX_CODE_LENGTH 40

const int vocab_hash_size = 30000000;  // Maximum 30 * 0.7 = 21M words in the vocabulary

typedef float real;                    // Precision of float numbers

struct Node {
  long long item; //word->feature or feature->word
  struct Node *next;
};

struct vocab_word {
  long long cn; //word count
  char *word;
  struct Node *List;
  int isFeature;
};

char train_file[MAX_STRING], output_file[MAX_STRING];
char save_vocab_file[MAX_STRING], read_vocab_file[MAX_STRING];
struct vocab_word *vocab;
int binary = 0, debug_mode = 2, window = 5, min_count = 5, num_threads = 12, min_reduce = 1;
int *vocab_hash;
long long vocab_max_size = 1000, vocab_size = 0, layer1_size = 100;
long long train_words = 0, word_count_actual = 0, iter = 5, file_size = 0, classes = 0;
real alpha = 0.025, starting_alpha, sample = 1e-3;
real *syn0, *syn1neg, *expTable;
clock_t start;

//feature hyper-parameter
int feature_mode = 0;
long long _NULL = -1; // NULL feature id
char knowledge_file[MAX_STRING];
int NumberOfFeature = 0;

int negative = 5;
const int table_size = 1e7;
int *table;
int *Ftable;

// Reutrn last delimiter index
int lstrchar(char *str, char d) {
  char *c = str;
  int i = 0;
  int result = -1;
  while (1) {
    if (*(c) == '\0') break;
    if (*c == d) result = i;
    c++;
    i++;
  }
  return result;
}

// Copy substring
void substrncpy(char *target, char *source, int start, int end) {
  int i = start;
  for (; i < end; i++) {
    target[i - start] = source[i];
  }
  target[i - start] = 0;
}

void InitUnigramTable() {
  int a, i;
  double train_words_pow = 0;
  double d1, power = 0.75;
  table = (int *)malloc(table_size * sizeof(int));
  for (a = 0; a < vocab_size - NumberOfFeature; a++) train_words_pow += pow(vocab[a].cn, power);
  i = 0;
  d1 = pow(vocab[i].cn, power) / train_words_pow;
  for (a = 0; a < table_size; a++) {
    table[a] = i;
    if (a / (double)table_size > d1) {
      i++;
      d1 += pow(vocab[i].cn, power) / train_words_pow;
    }
    if (i >= vocab_size - NumberOfFeature) i = vocab_size - NumberOfFeature - 1;
  }
  if (feature_mode) {
    train_words_pow = 0;
    Ftable = (int *)malloc(table_size * sizeof(int));
    for (a = vocab_size - NumberOfFeature; a < vocab_size; a++) train_words_pow += pow(vocab[a].cn, power);
    i = vocab_size - NumberOfFeature;
    d1 = pow(vocab[i].cn, power) / train_words_pow;
    for (a = 0; a < table_size; a++) {
      Ftable[a] = i;
      if (a / (double)table_size > d1) {
        i++;
        d1 += pow(vocab[i].cn, power) / train_words_pow;
      }
      if (i >= vocab_size) i = vocab_size - 1;
    }
  }
}

// Reads a single word from a file, assuming space + tab + EOL to be word boundaries
void ReadWord(char *word, FILE *fin) {
  int a = 0, ch;
  while (!feof(fin)) {
    ch = fgetc(fin);
    if ( ch <= ' ' || ch == 127 ) {
      if (a > 0) {
        if (ch == '\n') ungetc(ch, fin);
        break;
      }
      if (ch == '\n') {
        strcpy(word, (char *)"</s>");
        return;
      }
      else continue;
    }
    word[a] = ch;
    a++;
    if (a >= MAX_STRING - 1) a--;   // Truncate too long words
  }
  word[a] = 0;
}

// Returns hash value of a word
int GetWordHash(char *word) {
  unsigned long long a, hash = 0;
  for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
  hash = hash % vocab_hash_size;
  return hash;
}

// Returns position of a word in the vocabulary; if the word is not found, returns -1
int SearchVocab(char *word) {
  unsigned int hash = GetWordHash(word);
  while (1) {
    if (vocab_hash[hash] == -1) return -1;
    if (!strcmp(word, vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
    hash = (hash + 1) % vocab_hash_size;
  }
  return -1;
}

// Reads a word and returns its index in the vocabulary
int ReadWordIndex(FILE *fin) {
  char word[MAX_STRING];
  ReadWord(word, fin);
  if (feof(fin)) return -1;
  return SearchVocab(word);
}

// Reads a pair and returns their index in the vocabulary
void *ReadItemIndex(FILE *fin, long long *IndexOfPair) {
  char item[MAX_STRING];
  char pair[2][MAX_STRING]; // 0 is word, 1 is feature
  ReadWord(item, fin);

  if (feof(fin)) {
    IndexOfPair[0] = -1;
    return IndexOfPair;
  }

  if (strcmp(item, "</s>") != 0) {
    int delimiter_index = lstrchar(item, '(');
    if (delimiter_index == -1 || delimiter_index + 1 == strlen(item) - 1 || item[strlen(item) - 1] != ')') {
      IndexOfPair[0] = -1;
    }
    else {
      substrncpy(pair[0], item, 0, delimiter_index);
      substrncpy(pair[1], item, delimiter_index + 1, strlen(item) - 1);
      IndexOfPair[0] = SearchVocab(pair[0]);
      IndexOfPair[1] = SearchVocab(pair[1]);
    }
  }
  else IndexOfPair[0] = 0;

  return IndexOfPair;
}

// Adds a word to the vocabulary
int AddWordToVocab(char *word) {
  unsigned int hash, length = strlen(word) + 1;
  if (length > MAX_STRING) length = MAX_STRING;
  vocab[vocab_size].word = (char *)calloc(length, sizeof(char));
  strcpy(vocab[vocab_size].word, word);
  vocab[vocab_size].cn = 0;
  vocab_size++;
  // Reallocate memory if needed
  if (vocab_size + 2 >= vocab_max_size) {
    vocab_max_size += 1000;
    vocab = (struct vocab_word *)realloc(vocab, vocab_max_size * sizeof(struct vocab_word));
  }
  hash = GetWordHash(word);
  while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
  vocab_hash[hash] = vocab_size - 1;
  return vocab_size - 1;
}

// Used later for sorting by word counts
int VocabCompare(const void *a, const void *b) {
  return ((struct vocab_word *)b)->cn - ((struct vocab_word *)a)->cn;
}

// for sorting by type
int FeatureCompare(const void *a, const void *b) {
  return ((((struct vocab_word *)a)->isFeature - ((struct vocab_word *)b)->isFeature));
}

// Sorts the vocabulary by frequency using word counts
void SortVocab() {
  int a, size;
  unsigned int hash;
  // Sort the vocabulary and keep </s> at the first position
  qsort(&vocab[1], vocab_size - 1, sizeof(struct vocab_word), VocabCompare);
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  size = vocab_size;
  train_words = 0;
  for (a = 0; a < size; a++) {
    // Words occuring less than min_count times will be discarded from the vocab
    if ((vocab[a].cn < min_count) && (a != 0)) {
      vocab_size--;
      free(vocab[a].word);
    }
  }
  vocab = (struct vocab_word *)realloc(vocab, (vocab_size + 1) * sizeof(struct vocab_word));
  qsort(&vocab[1], vocab_size - 1, sizeof(struct vocab_word), FeatureCompare);

  for (a = 0; a < vocab_size; a++) {
    // Hash will be re-computed, as after the sorting it is not actual
    hash = GetWordHash(vocab[a].word);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = a;
    if (!vocab[a].isFeature) train_words += vocab[a].cn;
    else {
      NumberOfFeature++;
    }
  }
}

// Reduces the vocabulary by removing infrequent tokens
void ReduceVocab() {
  int a, b = 0;
  unsigned int hash;
  for (a = 0; a < vocab_size; a++) if (vocab[a].cn > min_reduce) {
    vocab[b].cn = vocab[a].cn;
    vocab[b].word = vocab[a].word;
    b++;
  }
  else free(vocab[a].word);
  vocab_size = b;
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  for (a = 0; a < vocab_size; a++) {
    // Hash will be re-computed, as it is not actual
    hash = GetWordHash(vocab[a].word);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = a;
  }
  fflush(stdout);
  min_reduce++;
}

void LearnVocabFromTrainFile() {
  char word[MAX_STRING];
  char feature[MAX_STRING];
  char temp[MAX_STRING];
  FILE *fin;
  FILE *fin2;
  long long a, i;
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  fin = fopen(train_file, "rb");
  if (fin == NULL) {
    printf("ERROR: training data file not found!\n");
    exit(1);
  }
  vocab_size = 0;
  AddWordToVocab((char *)"</s>");
  while (1) {
    ReadWord(word, fin);
    if (feof(fin)) break;
    if (feature_mode == 1) {
      if (strcmp(word, "</s>") != 0) {
        int delimiter_index = lstrchar(word, '('); //format apple(NN) banana(NN)
        if (delimiter_index == -1 || delimiter_index + 1 == strlen(word) - 1 || word[strlen(word) - 1] != ')') exit(0);
        substrncpy(temp, word, 0, delimiter_index);
        substrncpy(feature, word, delimiter_index + 1, strlen(word) - 1);
        strcpy(word, temp);
        i = SearchVocab(feature);
        if (i == -1) {
          a = AddWordToVocab(feature);
          vocab[a].cn = 1;
          vocab[a].List = NULL;
          vocab[a].isFeature = 1;
        }
        else vocab[i].cn++;
      }
    }
    train_words++;
    if ((debug_mode > 1) && (train_words % 100000 == 0)) {
      printf("%lldK\r", train_words / 1000);
      fflush(stdout);
    }
    i = SearchVocab(word);
    if (i == -1) {
      a = AddWordToVocab(word);
      vocab[a].cn = 1;
      vocab[a].List = NULL;
      vocab[a].isFeature = 0;
    }
    else vocab[i].cn++;
    if (vocab_size > vocab_hash_size * 0.7) ReduceVocab();
  }
  printf("\n");

  if (feature_mode == 2) {
    fin2 = fopen(knowledge_file, "rb");
    if (fin2 == NULL) {
      printf("ERROR: KnowledgeFile not found!\n");
      exit(1);
    }
    int idx = 0;
    int WordNum = 0;
    long long TempFeatureID = 0;
    while (1) { //Create Feature in Dict
      ReadWord(word, fin2);
      if (feof(fin2)) break;
      i = SearchVocab(word);
      if (i == -1) {
        if (idx == 0) { //is Feature
          a = AddWordToVocab(word);
          vocab[a].cn = 0;
          vocab[a].List = NULL;
          vocab[a].isFeature = 1;
          TempFeatureID = a;
        }
        idx++;
      }
      else {
        if (i != 0) { //is word
          if (vocab[i].cn >= min_count) {
            vocab[TempFeatureID].cn += vocab[i].cn;
            WordNum++;
          }
          idx++;
        }
        else {  // is </s>
          if (WordNum < 2) vocab[TempFeatureID].cn = 0;
          idx = 0; //reset
          WordNum = 0;
        }
      }
      if (vocab_size > vocab_hash_size * 0.7) ReduceVocab();
    }
    SortVocab(); //Remove less Feature
    fseek(fin2, 0, SEEK_SET);
    idx = 0;
    long long FeatureID = 0;
    while (1) { //Establish Word Link to Feature
      ReadWord(word, fin2);
      if (feof(fin2)) break;
      i = SearchVocab(word);
      if (i != -1) {
        if (idx == 0) { //is Feature
          if (i != 0) {
            FeatureID = i;
          }
        }
        else {
          if (vocab[i].cn >= min_count && i != 0) {
            struct Node* FeatureNode = (struct Node *)malloc(sizeof(struct Node));
            FeatureNode->item = FeatureID;
            FeatureNode->next = NULL;

            struct Node* temp = vocab[i].List;
            if (!temp)
              vocab[i].List = FeatureNode;
            else {
              while (1) {
                if (temp->next == NULL) {
                  temp->next = FeatureNode;
                  break;
                }
                temp = temp->next;
              }
            }
          }
        }
        if (i == 0) {  // is </s>
          idx = 0; //reset
        }
        else idx++;
      }
    }
    fclose(fin2);
  }
  else {
    SortVocab();
  }

  _NULL = SearchVocab("NULL");
  printf("NULL ID: %lld\n", _NULL);

  if (debug_mode > 0) {
    printf("Vocab size: %lld\n", vocab_size);
    printf("Words in train file: %lld\n", train_words);
  }
  file_size = ftell(fin);
  fclose(fin);
}

void SaveVocab() {
  long long i;
  FILE *fo = fopen(save_vocab_file, "wb");
  for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
  fclose(fo);
}

void ReadVocab() {
  long long a, i = 0;
  char c;
  char word[MAX_STRING];
  FILE *fin = fopen(read_vocab_file, "rb");
  if (fin == NULL) {
    printf("Vocabulary file not found\n");
    exit(1);
  }
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  vocab_size = 0;
  while (1) {
    ReadWord(word, fin);
    if (feof(fin)) break;
    a = AddWordToVocab(word);
    fscanf(fin, "%lld%c", &vocab[a].cn, &c);
    i++;
  }
  SortVocab();
  if (debug_mode > 0) {
    printf("Vocab size: %lld\n", vocab_size);
    printf("Words in train file: %lld\n", train_words);
  }
  fin = fopen(train_file, "rb");
  if (fin == NULL) {
    printf("ERROR: training data file not found!\n");
    exit(1);
  }
  fseek(fin, 0, SEEK_END);
  file_size = ftell(fin);
  fclose(fin);
}

#ifdef WIN32
int posix_memalign(void **memptr,
  size_t alignment,
  size_t size) {
  *memptr = _aligned_malloc(size, alignment);
  if (errno != 0)
    return errno;
  else
    return 0;
}
#endif

void InitNet() {
  long long a, b;
  unsigned long long next_random = 1;
  a = posix_memalign((void **)&syn0, 128, (long long)vocab_size * layer1_size * sizeof(real));
  if (syn0 == NULL) { printf("Memory allocation failed\n"); exit(1); }
  if (negative>0) {
    a = posix_memalign((void **)&syn1neg, 128, (long long)vocab_size * layer1_size * sizeof(real));
    if (syn1neg == NULL) { printf("Memory allocation failed\n"); exit(1); }
    for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++)
      syn1neg[a * layer1_size + b] = 0;
  }
  //initial word vector
  for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++) {
    next_random = next_random * (unsigned long long)25214903917 + 11;
    syn0[a * layer1_size + b] = (((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size;
  }
}

void *TrainModelThread(void *id) {
  long long a, b, d, word, last_word, sentence_length = 0, sentence_position = 0, feature = 0, *IndexOfPair;
  long long word_count = 0, last_word_count = 0, sen[MAX_SENTENCE_LENGTH + 1], sen_pos[MAX_SENTENCE_LENGTH + 1];
  long long l1, l2, c, target, label, local_iter = iter;
  unsigned long long next_random = (long long)id;
  real f, g;
  clock_t now;
  real *neu1 = (real *)calloc(layer1_size, sizeof(real));
  real *neu1e = (real *)calloc(layer1_size, sizeof(real));

  FILE *fi = fopen(train_file, "rb");
  fseek(fi, file_size / (long long)num_threads * (long long)id, SEEK_SET);
  IndexOfPair = (long long *)calloc(2, sizeof(long long)); // 0 is word_id, 1 is feature_id

  while (1) {
    if (word_count - last_word_count > 10000) {
      word_count_actual += word_count - last_word_count;
      last_word_count = word_count;
      if ((debug_mode > 1)) {
        now = clock();
        printf("\rAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", alpha,
          word_count_actual / (real)(iter * train_words + 1) * 100,
          word_count_actual / ((real)(now - start + 1) / (real)CLOCKS_PER_SEC * 1000));
        fflush(stdout);
      }
      alpha = starting_alpha * (1 - word_count_actual / (real)(iter * train_words + 1));
      if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
    }
    if (sentence_length == 0) {
      while (1) {
        if (feature_mode == 1) {  //feature
          ReadItemIndex(fi, IndexOfPair);
          word = IndexOfPair[0];
          feature = IndexOfPair[1];
        }
        else word = ReadWordIndex(fi);
        if (feof(fi)) break;
        if (word == -1) continue;
        word_count++;
        if (word == 0) break;
        // The subsampling randomly discards frequent words while keeping the ranking same
        if (sample > 0) {
          real ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) / vocab[word].cn;
          next_random = next_random * (unsigned long long)25214903917 + 11;
          if (ran < (next_random & 0xFFFF) / (real)65536) continue;
        }
        sen[sentence_length] = word;
        if (feature_mode == 1) sen_pos[sentence_length] = feature; //feature
        sentence_length++;
        if (sentence_length >= MAX_SENTENCE_LENGTH) break;
      }
      sentence_position = 0;
    }
    if (feof(fi) || (word_count > train_words / num_threads)) {
      word_count_actual += word_count - last_word_count;
      local_iter--;
      if (local_iter == 0) break;
      word_count = 0;
      last_word_count = 0;
      sentence_length = 0;
      fseek(fi, file_size / (long long)num_threads * (long long)id, SEEK_SET);
      continue;
    }
    word = sen[sentence_position];

    if (word == -1) continue;
    if (feature_mode == 1) feature = sen_pos[sentence_position]; //feature

    for (c = 0; c < layer1_size; c++) neu1[c] = 0;
    for (c = 0; c < layer1_size; c++) neu1e[c] = 0;

    next_random = next_random * (unsigned long long)25214903917 + 11;
    b = next_random % window;

    //train skip-gram
    for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
      c = sentence_position - window + a;
      if (c < 0) continue;
      if (c >= sentence_length) continue;
      last_word = sen[c];
      if (last_word == -1) continue;
      l1 = last_word * layer1_size;
      for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
      // NEGATIVE SAMPLING
      if (negative > 0) {
        for (d = 0; d < negative + 1; d++) {
          if (d == 0) {
            target = word;
            label = 1;
          }
          else {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            target = table[(next_random >> 16) % table_size];
            if (target == 0) target = next_random % (vocab_size - 1 - NumberOfFeature) + 1;
            if (target == word) continue;
            label = 0;
          }
          l2 = target * layer1_size;
          f = 0;
          for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
          if (f >= MAX_EXP) g = (label - 1) * alpha;
          else if (f <= -MAX_EXP) g = (label - 0) * alpha;
          else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
          for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];
        }
        // Learn weights input from hidden
        for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];
      }
    }

    if (feature_mode == 1) {

      if (feature != _NULL) {

        l1 = word * layer1_size;
        for (c = 0; c < layer1_size; c++) neu1e[c] = 0;

        //center word predict self-feature
        for (d = 0; d < negative + 1; d++) {
          if (d == 0) {
            target = feature;
            label = 1;
          }
          else {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            target = Ftable[(next_random >> 16) % table_size];
            if (target == feature) continue;
            label = 0;
          }
          l2 = target * layer1_size;
          f = 0;
          for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
          if (f >= MAX_EXP) g = (label - 1) * alpha;
          else if (f <= -MAX_EXP) g = (label - 0) * alpha;
          else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
          for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];
        }

        // Learn weights input from hidden
        for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];

      }
    }
    else if (feature_mode == 2) {

      l1 = word * layer1_size;
      for (c = 0; c < layer1_size; c++) neu1e[c] = 0;

      struct Node *temp;
      //center word predict self-feature
      temp = vocab[word].List;
      while (temp != NULL) {
        long long feature = temp->item;
        for (d = 0; d < negative + 1; d++) {
          if (d == 0) {
            target = feature;
            label = 1;
          }
          else {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            target = Ftable[(next_random >> 16) % table_size];
            if (target == feature) continue;
            label = 0;
          }
          l2 = target * layer1_size;
          f = 0;
          for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
          if (f > MAX_EXP) g = (label - 1) * alpha;
          else if (f < -MAX_EXP) g = (label - 0) * alpha;
          else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
          for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];
        }
        temp = temp->next;
      }

      // Learn weights input from hidden
      for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];

    }

    sentence_position++;
    if (sentence_position >= sentence_length) {
      sentence_length = 0;
      continue;
    }
  }
  fclose(fi);
  free(neu1);
  free(neu1e);
  pthread_exit(NULL);
  return NULL;
}

void TrainModel() {
  long a, b, c, d;
  FILE *fo, *fo_i, *fo_o;
  pthread_t *pt = (pthread_t *)malloc((num_threads) * sizeof(pthread_t));
  printf("Starting training using file %s\n", train_file);
  starting_alpha = alpha;
  if (read_vocab_file[0] != 0) ReadVocab();
  else LearnVocabFromTrainFile();
  if (save_vocab_file[0] != 0) SaveVocab();
  if (output_file[0] == 0) return;
  InitNet();
  if (negative > 0) InitUnigramTable();
  start = clock();

  for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, TrainModelThread, (void *)a);
  for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);
  printf("\n");

  char inputvec[MAX_STRING] = {'\0'};
  char outputvec[MAX_STRING] = {'\0'};

  strcat(inputvec, output_file);
  strcat(inputvec, ".syn0");
  strcat(outputvec, output_file);
  strcat(outputvec, ".syn1neg");

  fo_i = fopen(inputvec, "wb");
  fo_o = fopen(outputvec, "wb");

  if (classes == 0) {

    // Save the word vectors(syn0)
    fprintf(fo_i, "%lld %lld\n", vocab_size - NumberOfFeature, layer1_size);
    for (a = 0; a < vocab_size - NumberOfFeature; a++) {
      fprintf(fo_i, "%s ", vocab[a].word);
      if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn0[a * layer1_size + b], sizeof(real), 1, fo_i);
      else for (b = 0; b < layer1_size; b++) fprintf(fo_i, "%lf ", syn0[a * layer1_size + b]);
      fprintf(fo_i, "\n");
    }

    fclose(fo_i);

    // Save the word vectors(syn1neg)
    fprintf(fo_o, "%lld %lld\n", vocab_size, layer1_size);
    for (a = 0; a < vocab_size; a++) {
      fprintf(fo_o, "%s ", vocab[a].word);
      if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn1neg[a * layer1_size + b], sizeof(real), 1, fo_o);
      else for (b = 0; b < layer1_size; b++) fprintf(fo_o, "%lf ", syn1neg[a * layer1_size + b]);
      fprintf(fo_o, "\n");
    }

    fclose(fo_o);
  }
  else {

    fo = fopen(output_file, "wb");

    // Run K-means on the word vectors
    int clcn = classes, iter = 10, closeid;
    int *centcn = (int *)malloc(classes * sizeof(int));
    int *cl = (int *)calloc(vocab_size - NumberOfFeature, sizeof(int));
    real closev, x;
    real *cent = (real *)calloc(classes * layer1_size, sizeof(real));
    for (a = 0; a < vocab_size - NumberOfFeature; a++) cl[a] = a % clcn;
    for (a = 0; a < iter; a++) {
      for (b = 0; b < clcn * layer1_size; b++) cent[b] = 0;
      for (b = 0; b < clcn; b++) centcn[b] = 1;
      for (c = 0; c < vocab_size - NumberOfFeature; c++) {
        for (d = 0; d < layer1_size; d++) cent[layer1_size * cl[c] + d] += syn0[c * layer1_size + d];
        centcn[cl[c]]++;
      }
      for (b = 0; b < clcn; b++) {
        closev = 0;
        for (c = 0; c < layer1_size; c++) {
          cent[layer1_size * b + c] /= centcn[b];
          closev += cent[layer1_size * b + c] * cent[layer1_size * b + c];
        }
        closev = sqrt(closev);
        for (c = 0; c < layer1_size; c++) cent[layer1_size * b + c] /= closev;
      }
      for (c = 0; c < vocab_size - NumberOfFeature; c++) {
        closev = -10;
        closeid = 0;
        for (d = 0; d < clcn; d++) {
          x = 0;
          for (b = 0; b < layer1_size; b++) x += cent[layer1_size * d + b] * syn0[c * layer1_size + b];
          if (x > closev) {
            closev = x;
            closeid = d;
          }
        }
        cl[c] = closeid;
      }
    }
    // Save the K-means classes
    for (a = 0; a < vocab_size - NumberOfFeature; a++) fprintf(fo, "%s %d\n", vocab[a].word, cl[a]);
    free(centcn);
    free(cent);
    free(cl);

    fclose(fo);
  }
}

int ArgPos(char *str, int argc, char **argv) {
  int a;
  for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
    if (a == argc - 1) {
      printf("Argument missing for %s\n", str);
      exit(1);
    }
    return a;
  }
  return -1;
}

int main(int argc, char **argv) {
  int i;
  if (argc == 1) {
    printf("Heterogeneous Word Embedding\n\n");
    printf("Options:\n");
    printf("Parameters for training:\n");
    printf("\t-train <file>\n");
    printf("\t\tUse text data from <file> to train the model\n");
    printf("\t-output <file>\n");
    printf("\t\tUse <file> to save the resulting word vectors / word clusters\n");
    printf("\t-size <int>\n");
    printf("\t\tSet size of word vectors; default is 100\n");
    printf("\t-window <int>\n");
    printf("\t\tSet max skip length between words; default is 5\n");
    printf("\t-sample <float>\n");
    printf("\t\tSet threshold for occurrence of words. Those that appear with higher frequency in the training data\n");
    printf("\t\twill be randomly down-sampled; default is 1e-3, useful range is (0, 1e-5)\n");
    printf("\t-negative <int>\n");
    printf("\t\tNumber of negative examples; default is 5, common values are 3 - 10 (0 = not used)\n");
    printf("\t-threads <int>\n");
    printf("\t\tUse <int> threads (default 12)\n");
    printf("\t-iter <int>\n");
    printf("\t\tRun more training iterations (default 5)\n");
    printf("\t-min-count <int>\n");
    printf("\t\tThis will discard words that appear less than <int> times; default is 5\n");
    printf("\t-alpha <float>\n");
    printf("\t\tSet the starting learning rate; default is 0.025 for skip-gram and 0.05 for CBOW\n");
    printf("\t-classes <int>\n");
    printf("\t\tOutput word classes rather than word vectors; default number of classes is 0 (vectors are written)\n");
    printf("\t-debug <int>\n");
    printf("\t\tSet the debug mode (default = 2 = more info during training)\n");
    printf("\t-binary <int>\n");
    printf("\t\tSave the resulting vectors in binary moded; default is 0 (off)\n");
    printf("\t-save-vocab <file>\n");
    printf("\t\tThe vocabulary will be saved to <file>\n");
    printf("\t-read-vocab <file>\n");
    printf("\t\tThe vocabulary will be read from <file>, not constructed from the training data\n");
    printf("\t-fmode <int>\n");
    printf("\t\tEnable the Feature mode (default = 0 = only using skip-gram, "
                                                  "1 = predicting self-feature of sequential feature tag, "
                                                  "2 = predicting self-feature of global feature table)\n");
    printf("\t-knfile <file>\n");
    printf("\t\tThe sense-words file will be read from <file>\n");
    printf("\nExamples:\n");
    printf("%s -train data.txt -output vec.txt -size 200 -window 5 -sample 1e-4 -negative 5 -binary 0 "
              "-fmode 2 -knfile senses.txt -iter 3\n\n", argv[0]);
    return 0;
  }
  output_file[0] = 0;
  save_vocab_file[0] = 0;
  read_vocab_file[0] = 0;
  if ((i = ArgPos((char *)"-size", argc, argv)) > 0) layer1_size = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-train", argc, argv)) > 0) strcpy(train_file, argv[i + 1]);
  if ((i = ArgPos((char *)"-save-vocab", argc, argv)) > 0) strcpy(save_vocab_file, argv[i + 1]);
  if ((i = ArgPos((char *)"-read-vocab", argc, argv)) > 0) strcpy(read_vocab_file, argv[i + 1]);
  if ((i = ArgPos((char *)"-debug", argc, argv)) > 0) debug_mode = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-binary", argc, argv)) > 0) binary = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-alpha", argc, argv)) > 0) alpha = atof(argv[i + 1]);
  if ((i = ArgPos((char *)"-output", argc, argv)) > 0) strcpy(output_file, argv[i + 1]);
  if ((i = ArgPos((char *)"-window", argc, argv)) > 0) window = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-sample", argc, argv)) > 0) sample = atof(argv[i + 1]);
  if ((i = ArgPos((char *)"-negative", argc, argv)) > 0) negative = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-iter", argc, argv)) > 0) iter = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-min-count", argc, argv)) > 0) min_count = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-classes", argc, argv)) > 0) classes = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-fmode", argc, argv)) > 0) feature_mode = atoi(argv[i + 1]);
  if ((i = ArgPos((char *)"-knfile", argc, argv)) > 0) strcpy(knowledge_file, argv[i + 1]);

  vocab = (struct vocab_word *)calloc(vocab_max_size, sizeof(struct vocab_word));
  vocab_hash = (int *)calloc(vocab_hash_size, sizeof(int));
  expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
  for (i = 0; i <= EXP_TABLE_SIZE; i++) {
    expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
    expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
  }
  TrainModel();
  return 0;
}
