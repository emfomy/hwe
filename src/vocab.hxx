////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/vocab.hxx
/// \brief      The vocabulary.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#ifndef HWE_VOCAB_HXX_
#define HWE_VOCAB_HXX_

#include "def.hxx"

#include <unordered_map>
#include <vector>

#include "util.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

constexpr int kMaxVocalNum = 30000000;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The vocabulary word class.
///
struct VocabWord {
  index_t count;
  index_t id;
  VocabWord( index_t id = -1 ) : count(0), id(id) {}
};

using VocabMap     = std::unordered_map<string_t, VocabWord>;
using WordList     = std::vector<string_t>;
using VocabPair    = std::pair<const string_t, VocabWord>;
using VocabPairPtr = Ptr<VocabPair>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The vocabulary set iterator.
///
class VocabIterator : public WordList::const_iterator {

  using Base_ = WordList::const_iterator;

 public:
  using value_type = VocabPair;
  using pointer = VocabPairPtr;
  using reference = VocabPair;

 protected:

  /// The vocabulary map.
  VocabMap &_vocab_;

 public:

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief  Default constructor.
  ///
  inline VocabIterator( WordList::const_iterator it, VocabMap &vocab ) noexcept
    : Base_(it), _vocab_(vocab) {}

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief  Indirection operator.
  ///
  inline VocabPair operator*() const noexcept {
    return std::make_pair(Base_::operator*(), _vocab_.at(Base_::operator*()));
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief  Indirection operator.
  ///
  inline VocabPairPtr operator->() const noexcept {
    return VocabPairPtr(**this);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief  Subscript operator.
  ///
  inline VocabPair operator[]( const index_t n ) const noexcept {
    return *VocabIterator(*this+n, _vocab_);
  }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The vocabulary set class.
///
class VocabSet {

 protected:

  /// The vocabulary map.
  VocabMap _vocab_;

  /// The word list.
  WordList _word_;

  index_t train_words_ = 0;
  index_t min_count_   = 5;
  index_t min_reduce_  = 2;

 public:

  inline bool ok() noexcept { return _vocab_.size() == _word_.size(); }

  inline const VocabMap& _vocab() noexcept { return _vocab_; }
  inline const WordList& _word() noexcept { return _word_; }

  inline index_t train_words() noexcept { return train_words_; }
  inline index_t size() noexcept { return _vocab_.size(); }

  inline VocabWord& operator[]( const string_t &word ) noexcept { return _vocab_[word]; }
  inline VocabWord& operator[]( string_t &&word ) noexcept { return _vocab_[word]; }

  // Read/save vocabulary
  void LearnVocab( const string_t &train_file ) noexcept;
  void ReadVocab( const string_t &vocab_file ) noexcept;
  void SaveVocab( const string_t &vocab_file ) noexcept;
  string_t ReadWord( std::istream &fin ) noexcept;
  void ReduceVocab() noexcept;
  void SortVocab() noexcept;

  // Gets iterator
  inline VocabIterator begin() noexcept { return VocabIterator(_word_.begin(), _vocab_); }
  inline VocabIterator end() noexcept { return VocabIterator(_word_.end(), _vocab_); }

};

}  // namespace hwe

#endif  // HWE_VOCAB_HXX_
