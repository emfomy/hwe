////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       include/hwe/model/model.hxx
/// \brief      The abstract model.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#ifndef HWE_MODEL_MODEL_HXX_
#define HWE_MODEL_MODEL_HXX_

#include <vector>

#include <hwe/def.hxx>
#include <hwe/util.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

constexpr int kExpTableSize      = 1000;
constexpr int kMaxSentenceLength = 1000;
constexpr int kTableSize         = 1e8;
constexpr real_t kMaxExp         = 6;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The abstract model.
///
class Model {

 public:

  VocabSet vocab;

 protected:

  ExpTable exp_table_;

  std::vector<real_t> syn0_;
  std::vector<real_t> syn1_;
  std::vector<real_t> syn1neg_;
  std::vector<int> unigram_table_;

  index_t train_words_       = 0;
  index_t word_count_actual_ = 0;
  index_t file_size_         = 0;

  timepoint_t cstart_;
  timepoint_t cend_;

 public:

  // File path
  string_t train_file;
  string_t output_file;
  string_t save_vocab_file;
  string_t read_vocab_file;

  // Setting
  int binary      = 0;
  int debug_mode  = 2;
  int window      = 5;
  int min_count   = 5;
  int num_threads = 12;
  int min_reduce  = 1;
  int negative    = 5;

  index_t layer1_size = 100;
  index_t num_iter    = 5;

  real_t alpha = 0.025;
  real_t starting_alpha;
  real_t sample = 1e-3;

 public:

  Model() noexcept;

  void TrainModel() noexcept;

 protected:

  void TrainModelThread( const index_t id ) noexcept;
  void InitUnigramTable() noexcept;
  void InitNet() noexcept;
  void SaveEmbed() noexcept;
  void CheckFile() noexcept;

};

}  // namespace hwe

#endif  // HWE_MODEL_MODEL_HXX_
