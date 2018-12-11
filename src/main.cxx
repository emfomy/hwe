////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/main.cxx
/// \brief      The main function.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#include "def.hxx"
#include "model/model.hxx"

using namespace hwe;

int main() {

  Model model;

  model.train_file      = "enwik8.txt";
  model.output_file     = "enwik8.emb";
  model.save_vocab_file = "enwik8.vocab";

  model.num_threads = 32;

  model.TrainModel();

  return 0;
}
