////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       include/hwe/util/reader.hxx
/// \brief      The word reader.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#ifndef HWE_UTIL_READER_HXX_
#define HWE_UTIL_READER_HXX_

#include <fstream>
#include <sstream>

#include <hwe/def.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The word reader class.
///
class Reader {

 protected:

  std::ifstream &fin_;

  string_t line_;

  std::istringstream iss_;

  bool good_;

 public:

  inline Reader( std::ifstream &fin ) noexcept : fin_(fin), good_(true) {}

  inline bool good() noexcept { return good_; }

  bool operator()( string_t &word ) noexcept;

};

}  // namespace hwe

#endif  // HWE_UTIL_READER_HXX_
