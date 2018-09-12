////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/skipgram.cxx
/// \brief      The Skip-Gram Model.
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

#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>

#include <omp.h>
#include "hwe.hxx"
#include "vocab.hxx"

using namespace hwe;

int main() {

  string_t train_file  = "alice.txt";
  string_t vocab_file  = "vocab.txt";
  string_t vocab_file2 = "vocab2.txt";

  VocabSet vocab;

  vocab.LearnVocab(train_file);
  vocab.SaveVocab(vocab_file);
  vocab.ReadVocab(vocab_file);
  vocab.SaveVocab(vocab_file2);

  return 0;
}
