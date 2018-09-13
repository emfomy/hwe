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

#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>

#include <omp.h>
#include "def.hxx"
#include "vocab.hxx"

using namespace hwe;

constexpr int kTableSize = 1e8;

VocabSet vocab;

string_t train_file, output_file, save_vocab_file, read_vocab_file;

std::vector<real_t> syn0, syn1, syn1neg, expTable;
std::vector<int> table;

index_t layer1_size = 100, file_size = 0;
int negative = 5;

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

void CheckTrainFile( const string_t &train_file ) {

  std::ifstream fin(train_file);
  if ( !fin.is_open() ) {
    std::cerr << "ERROR: Vocabulary file not found!" << std::endl;
    exit(1);
  }
  file_size = fin.tellg();
}

void TrainModel() {
  std::cout << "Starting training using file " << train_file << std::endl;

  CheckTrainFile(train_file);
  if (read_vocab_file.size()) vocab.ReadVocab(read_vocab_file); else vocab.LearnVocab(train_file);
  if (save_vocab_file.size()) vocab.SaveVocab(save_vocab_file);
  if (!output_file.size()) return;
  InitNet();
}

int main() {

  train_file      = "alice.txt";
  output_file     = "alice.emb";
  save_vocab_file = "vocab.txt";

  TrainModel();

  // for ( auto it = vocab.begin(); it != vocab.end()-3; ++it ) {
  //   std::cout << it[3].first << "\t" << it[3].second.count << std::endl;
  // }

  // for ( auto it = vocab.begin(); it != vocab.end(); ++it ) {
  //   std::cout << it->first << "\t" << it->second.count << std::endl;
  // }

  // for ( const auto pair : vocab ) {
  //   std::cout << pair.first << "\t" << pair.second.count << std::endl;
  // }

  return 0;
}
