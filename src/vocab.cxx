////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/vocab.cxx
/// \brief      The vocabulary (implementation).
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
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

#include "def.hxx"
#include "vocab.hxx"

#include <iostream>
#include <fstream>

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Learn vocabulary from training file.
///
void VocabSet::LearnVocab( const string_t &train_file ) noexcept {
  _vocab_.clear();
  _word_.clear();
  train_words_ = 0;

  std::ifstream fin;
  fin.open(train_file);
  if ( !fin.is_open() ) {
    std::cerr << "ERROR: training data file not found!" << std::endl;
    exit(1);
  }

  _vocab_["</s>"];
  string_t word;

  while ( true ) {
    word = ReadWord(fin);
    if ( word == "" ) break;
    if ( (debug_mode > 1) && (train_words_ % 100000 == 0) ) {
      std::cout << (train_words_/1000) << "K\r" << std::flush;
    }

    ++_vocab_[word].count;
    train_words_++;

    if ( _vocab_.size() > kMaxVocalNum ) {
      ReduceVocab();
    }
  }

  SortVocab();

  if ( debug_mode > 0 ) {
    std::cout << "Vocab size:          " << _vocab_.size() << std::endl
              << "Words in train file: " << train_words_  << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Read vocabulary from file.
///
void VocabSet::ReadVocab( const string_t &vocab_file ) noexcept {
  _vocab_.clear();
  _word_.clear();
  train_words_ = 0;

  std::ifstream fin;
  fin.open(vocab_file);
  if ( !fin.is_open() ) {
    std::cerr << "ERROR: Vocabulary file not found!" << std::endl;
    exit(1);
  }

  _vocab_["</s>"];
  string_t word;
  index_t count;

  while ( true ) {
    word = ReadWord(fin);
    if ( word == "" ) break;

    fin >> count;
    _vocab_[word].count = count;
    train_words_ += count;
    fin.get();
  }

  SortVocab();

  if ( debug_mode > 0 ) {
    std::cout << "Vocab size:          " << _vocab_.size() << std::endl
              << "Words in train file: " << train_words_  << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Save vocabulary into file.
///
void VocabSet::SaveVocab( const string_t &vocab_file ) noexcept {
  assert(_vocab_.size() == _word_.size());

  std::ofstream fout;
  fout.open(vocab_file);
  for ( const auto &word : _word_ ) {
    fout << word << " " << _vocab_.at(word).count << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Reads a single word from a file, assuming space + tab + EOL to be word boundaries.
///
string_t VocabSet::ReadWord( std::istream &fin ) noexcept {
  string_t word;
  while ( fin.good() ) {
    char ch = fin.peek();
    if ( ch == '\n' ) {
      fin.get();
      word = "</s>";
      break;
    } else if ( isspace(ch) ) {
      fin.get();
      continue;
    }
    else {
      fin >> word;
      break;
    }
  }
  return word;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Reduces the vocabulary by removing infrequent tokens.
///
void VocabSet::ReduceVocab() noexcept {
  for ( auto it = _vocab_.cbegin(); it != _vocab_.cend(); ){
    if ( it->second.count < min_reduce_ ) {
      train_words_ -= it->second.count;
      it = _vocab_.erase(it);
    } else {
      ++it;
    }
  }
  ++min_reduce_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sorts the vocabulary by frequency using word counts.
///
void VocabSet::SortVocab() noexcept {
  for ( auto it = _vocab_.cbegin(); it != _vocab_.cend(); ){
    if ( it->second.count < min_count_ ) {
      train_words_ -= it->second.count;
      it = _vocab_.erase(it);
    } else {
      ++it;
    }
  }

  _word_.reserve(_vocab_.size());
  _word_.push_back("</s>");
  for ( const auto pair : _vocab_ ) {
    if ( pair.first != "</s>" ) {
      _word_.push_back(pair.first);
    }
  }

  auto comp = [&]( const string_t &a, const string_t &b ) {
    return (_vocab_[a].count != _vocab_[b].count) ? (_vocab_[a].count > _vocab_[b].count) : (a > b);
  };
  std::sort(_word_.begin()+1, _word_.end(), comp);
}

}  // namespace hwe
