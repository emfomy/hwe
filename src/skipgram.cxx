////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/skipgram.cxx
/// \brief      The Skip-Gram Model.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
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

#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>

#include <omp.h>
#include "def.hxx"
#include "vocab.hxx"
#include "util.hxx"

using namespace hwe;

constexpr int kExpTableSize      = 1000;
constexpr int kMaxSentenceLength = 1000;
constexpr int kTableSize         = 1e8;
constexpr real_t kMaxExp         = 6;

string_t train_file, output_file, save_vocab_file, read_vocab_file;

std::vector<real_t> syn0, syn1, syn1neg;
std::vector<int> unigram_table;

index_t layer1_size = 100, train_words = 0, word_count_actual = 0, iter = 5, file_size = 0;
int binary = 0, cbow = 1, debug_mode = 2, window = 5, min_count = 5, num_threads = 12, min_reduce = 1, negative = 5;
real_t alpha = 0.025, starting_alpha, sample = 1e-3;
timepoint_t cstart, cend;

VocabSet vocab;
ExpTable exp_table(kMaxExp, kExpTableSize);

void InitUnigramTable() {
  double train_words_pow = 0.0, power = 0.75;

  unigram_table.resize(kTableSize);

  auto it = vocab.begin();
  for ( ++it; it != vocab.end(); ++it ) {
    train_words_pow += std::pow(it->second.count, power);
  }

  index_t i = 1;
  double d1 = std::pow(vocab[i].count, power) / train_words_pow;
  for ( auto a = 0u; a < unigram_table.size(); ++a ) {
    unigram_table[a] = i;
    if (a / (double)(unigram_table.size()) > d1) {
      ++i;
      d1 += std::pow(vocab[i].count, power) / train_words_pow;
      if (i >= vocab.size()) i = vocab.size() - 1;
    }
  }
}

void InitNet() {
  assert(vocab.ok());

  syn0.resize(vocab.size() * layer1_size);
  syn1.resize(vocab.size() * layer1_size, 0.0);
  syn1neg.resize(vocab.size() * layer1_size, 0.0);

  std::minstd_rand engine;
  std::uniform_real_distribution<> dist(-0.5/layer1_size, 0.5/layer1_size);
  engine.seed(0);
  std::generate(syn0.begin(), syn0.end(), [&](){ return dist(engine); });
}

void SaveEmbed() {
  std::ofstream fout(output_file);
  if ( !fout.is_open() ) {
    std::cerr << "ERROR: Enable to open output file '" << output_file << "'!" << std::endl;
    exit(1);
  }

  // Save the word vectors
  fout << vocab.size() << " " << layer1_size << '\n';
  for ( const auto &pair : vocab ) {
    fout << pair.first;
    if ( binary ) {
      fout << " ";
      fout.write(static_cast<char*>(static_cast<void*>(&syn0[pair.second.idx * layer1_size])), sizeof(real_t) * layer1_size);
    } else {
      for ( auto j = 0; j < layer1_size; ++j) {
        fout << " " << syn0[pair.second.idx * layer1_size + j];
      }
    }
    fout << "\n";
  }
  fout << std::flush;
}

void CheckFile() {
  std::ifstream fin(train_file, std::ios::binary | std::ios::ate);
  if ( !fin.is_open() ) {
    std::cerr << "ERROR: Enable to open training file '" << train_file << "'!" << std::endl;
    exit(1);
  }
  file_size = fin.tellg();

  std::ofstream fout(output_file);
  if ( !fout.is_open() ) {
    std::cerr << "ERROR: Enable to open output file '" << output_file << "'!" << std::endl;
    exit(1);
  }
}

void TrainModelThread( const index_t id) {
  index_t sentence_length = 0, sentence_position = 0;
  index_t word_count = 0, last_word_count = 0, sen[kMaxSentenceLength + 1];
  index_t l1, l2, label, local_iter = iter;

  std::minstd_rand engine;
  std::uniform_real_distribution<> dist;
  engine.seed(id);

  std::vector<real_t> neu1(layer1_size);
  std::vector<real_t> neu1e(layer1_size);

  std::ifstream fin(train_file);

  fin.seekg((file_size/(index_t)num_threads) * id);

  while (true) {

    if ( word_count - last_word_count > 10000 ) {
      word_count_actual += word_count - last_word_count;
      last_word_count = word_count;

      if ( debug_mode > 1 ) {
        auto cnow = GetTime();
        printf("\rAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ",
          alpha,
          word_count_actual / (real_t)(iter * train_words + 1) * 100.0,
          word_count_actual / GetTimeDuration(cstart, cnow) / 1000.0);
        fflush(stdout);
      }
      alpha = starting_alpha * (1 - word_count_actual / (real_t)(iter * train_words + 1));
      if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
    }

    if (sentence_length == 0) {
      while (1) {
        auto word = vocab.ReadWord(fin);
        if ( word == "" ) break;
        if ( vocab._vocab().count(word) == 0 ) continue;
        word_count++;
        if ( word == "</s>" ) break;
        auto &vw = vocab[word];

        // The subsampling randomly discards frequent words while keeping the ranking same
        if ( sample > 0 ) {
          auto ran = (std::sqrt(vw.count / (sample * train_words)) + 1) * (sample * train_words) / vw.count;
          if (ran < dist(engine)) continue;
        }
        sen[sentence_length] = vw.idx;
        sentence_length++;
        if ( sentence_length >= kMaxSentenceLength ) break;
      }
      sentence_position = 0;
    }

    if ( !fin.good() || (word_count > train_words / num_threads) ) {
      word_count_actual += word_count - last_word_count;
      local_iter--;
      if ( local_iter == 0 ) break;
      word_count = 0;
      last_word_count = 0;
      sentence_length = 0;
      fin.seekg((file_size / (index_t)num_threads) * id);
      continue;
    }

    auto &word_idx = sen[sentence_position];
    std::fill(neu1.begin(),  neu1.end(),  0.0);

    // Skip-gram
    const auto window_size = int(dist(engine) * window);

    for (auto a = window_size; a < window * 2 + 1 - window_size; a++) if (a != window) {
      auto c = sentence_position - window + a;
      if (c < 0) continue;
      if (c >= sentence_length) continue;
      auto &last_word_idx = sen[c];
      l1 = last_word_idx * layer1_size;
      std::fill(neu1e.begin(), neu1e.end(), 0.0);

      // Negative Sampling
      index_t target_idx;
      for (auto d = 0; d < negative + 1; d++) {
        if ( d == 0 ) {
          target_idx = word_idx;
          label = 1;
        } else {
          target_idx = unigram_table[int(dist(engine) * unigram_table.size())];
          if (target_idx == word_idx) continue;
          label = 0;
        }
        l2 = target_idx * layer1_size;
        auto f = 0.0;
        for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
        auto g = (label - exp_table(f)) * alpha;
        for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
        for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];

        // Learn weights input -> hidden
        for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];
      }
    }

    sentence_position++;
    if (sentence_position >= sentence_length) {
      sentence_length = 0;
      continue;
    }
  }
}

void TrainModel() {
  assert(negative > 0);
  starting_alpha = alpha;

  std::cout << "Starting training using file " << train_file << std::endl;

  CheckFile();

  cstart = GetTime();
  if (read_vocab_file.size()) vocab.ReadVocab(read_vocab_file); else vocab.LearnVocab(train_file);
  cend = GetTime();
  if ( debug_mode > 0 ) DisplayDuration(std::cout, "Load vocabulary", cstart, cend);

  if (save_vocab_file.size()) vocab.SaveVocab(save_vocab_file);
  if (!output_file.size()) return;

  InitNet();
  InitUnigramTable();

  train_words = vocab.train_words();

  cstart = GetTime();
  #pragma omp parallel for
  for ( auto id = 0; id < num_threads; ++id) {
    TrainModelThread(id);
  }
  std::cout << std::endl;

  SaveEmbed();
}

int main() {
  vocab.debug_mode(debug_mode);
  vocab.min_count(min_count);

  train_file      = "enwik8.txt";
  output_file     = "enwik8.emb";
  save_vocab_file = "enwik8.vocab";

  TrainModel();

  // for ( auto it = vocab.begin(); it != vocab.end()-3; ++it ) {
  //   std::cout << it[3].first << "\t" << it[3].second.count << std::endl;
  // }

  // for ( auto it = vocab.begin(); it != vocab.end(); ++it ) {
  //   std::cout << it->first << "\t" << it->second.count << std::endl;
  // }

  // for ( const auto &pair : vocab ) {
  //   std::cout << pair.first << "\t" << pair.second.count << std::endl;
  // }

  return 0;
}
