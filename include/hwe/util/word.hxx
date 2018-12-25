////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       include/hwe/util/word.hxx
/// \brief      The word.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#ifndef HWE_UTIL_WORD_HXX_
#define HWE_UTIL_WORD_HXX_

#include <cstring>

#include <hwe/def.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

constexpr int kMaxWordLen  = 100;
constexpr int kMaxWordHash = 30000000;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The word type.
///
enum class WordType : int8_t {
  STR = 0,  ///< Normal string.
  EOS = 1,  ///< End of sentence.
  NUL = -1, ///< Null string.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The word reader class.
///
class Word final {

 private:

  size_t len_ = 0;

  WordType type_ = WordType::NUL;

  char str_[kMaxWordLen];

 public:

  // Constructor

  inline Word() noexcept {};
  inline Word( const char *str ) noexcept {
    len_ = strlen(str);
    if ( len_ >= kMaxWordLen ) len_ = kMaxWordLen-1;
    memcpy(str_, str, len_);
    str_[len_] = 0;
  }

  inline static Word EOS() { Word word; word.type_ = WordType::EOS; return word; }
  inline static Word NUL() { Word word; word.type_ = WordType::NUL; return word; }

  // Member

  inline size_t length() const noexcept { return len_; }
  inline WordType type() const noexcept { return type_; }
  inline const char* c_str() const noexcept { return str_; }
  inline operator bool() const noexcept { return type_ != WordType::NUL; }

  // Read

  inline void read( std::ifstream &fin ) noexcept {
    len_ = 0;
    type_ = WordType::NUL;

    while ( fin.good() ) {
      char ch = fin.get();
      if ( ch == '\n' ) {
        type_ = WordType::EOS;
        return;
      } else if ( IsDelimiter(ch) ) {
        continue;
      } else {
        type_ = WordType::STR;
        while ( fin.good() && !IsDelimiter(ch) ) {
          str_[len_++] = ch;
          if ( len_ >= kMaxWordLen ) len_ = kMaxWordLen-1;
          ch = fin.get();
        }
        if ( ch == '\n' ) fin.unget();
        str_[len_] = 0;
        return;
      }
    }
  }

  inline void read( FILE *fin ) noexcept {
    len_ = 0;
    type_ = WordType::NUL;
    char ch;

    // while (!feof(fin)) {
    //   ch = fgetc(fin);
    //   if ( IsDelimiter(ch) ) {
    //     if (len_ > 0) {
    //       if (ch == '\n') ungetc(ch, fin);
    //       break;
    //     }
    //     if (ch == '\n') {
    //       type_ = WordType::EOS;
    //       return;
    //     } else {
    //       continue;
    //     }
    //   }
    //   str_[len_] = ch;
    //   len_++;
    //   if (len_ >= kMaxWordLen - 1) len_--;
    // }
    // if ( len_ > 0 ) {
    //   type_ = WordType::STR;
    //   str_[len_] = 0;
    // }

    while ( !feof(fin) ) {
      ch = fgetc(fin);
      if ( IsDelimiter(ch) ) {
        if ( ch == '\n' ) {
          type_ = WordType::EOS;
          len_ = 4;
          strcpy(str_, "</s>");
          break;
        }
        continue;
      } else {
        type_ = WordType::STR;
        while ( !feof(fin) && !IsDelimiter(ch) ) {
          str_[len_++] = ch;
          if ( len_ >= kMaxWordLen ) len_ = kMaxWordLen-1;
          ch = fgetc(fin);
        }
        if ( ch == '\n' ) ungetc(ch, fin);
        str_[len_] = 0;
        break;
      }
    }
  }

  // Utility

  inline int Hash() const noexcept {
    size_t hash = 0;
    for ( const char *pc = str_; pc < str_+len_; ++pc ) {
      hash = hash * 257 + *pc;
    }
    hash %= kMaxWordHash;
    return hash;
  }

  inline size_t Pred( const Word &other ) const noexcept {
    return strcmp(this->str_, other.str_) == 0;
  }

  inline std::string String() const noexcept {
    switch ( type_ ) {
      case WordType::EOS: {
        return "</s>";
      }
      case WordType::NUL: {
        return "";
      }
      default: {
        return std::string(str_, len_);
      }
    }
  }

};

}  // namespace hwe

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The STL namespace
//
namespace std {

template<>
struct hash<hwe::Word> {
  inline size_t operator()( const hwe::Word &obj ) const noexcept {
    return obj.Hash();
  }
};

template<>
struct equal_to<hwe::Word> {
  inline size_t operator()( const hwe::Word &lhs, const hwe::Word &rhs ) const noexcept {
    return lhs.Pred(rhs);
  }
};

static inline ostream &operator<<( ostream &os, hwe::Word obj ) {
  return os << obj.String();
}

}  // namespace std

#endif  // HWE_UTIL_WORD_HXX_
