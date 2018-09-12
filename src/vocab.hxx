////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/vocab.hxx
/// \brief      The vocabulary.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
///

#include "hwe.hxx"

#include <unordered_map>
#include <vector>

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The vocabulary set class.
///
class VocabSet {

 protected:
  std::unordered_map<string_t, VocabWord> vocab_;
  std::vector<string_t> word_;

  index_t train_words_ = 0;
  index_t min_count_   = 5;
  index_t min_reduce_  = 2;
  index_t debug_mode_  = 2;

 public:

  inline index_t debug_mode() noexcept { return debug_mode_; }
  inline void debug_mode( index_t debug_mode ) noexcept { debug_mode_ = debug_mode; }

  inline index_t train_words() noexcept { return train_words_; }

  inline VocabWord& operator[]( const string_t &word ) noexcept;
  inline VocabWord& operator[]( string_t &&word ) noexcept;

  void LearnVocab( const string_t &train_file ) noexcept;
  void ReadVocab( const string_t &vocab_file ) noexcept;
  void SaveVocab( const string_t &vocab_file ) noexcept;

  string_t ReadWord( std::istream &fin ) noexcept;
  void ReduceVocab() noexcept;
  void SortVocab() noexcept;

};

}  // namespace hwe
