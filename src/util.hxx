////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/util.hxx
/// \brief      The utility.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#ifndef HWE_UTIL_HXX_
#define HWE_UTIL_HXX_

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "def.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The exp table
///
class ExpTable {

 public:

  /// The object.
  const index_t max_exp;

 protected:

  /// The table.
  std::vector<real_t> exp_table;

 public:

  ExpTable( const index_t max_exp, const index_t size ) noexcept : max_exp(max_exp), exp_table(size+1) {
    for ( auto i = 0; i < (size+1); i++ ) {
      exp_table[i] = 1.0 / (1.0 + std::exp((i / (2 * size) - 1) * -max_exp));
    }
  }

  inline real_t operator()( const real_t f ) {
    if ( f >= max_exp ) {
      return 1.0;
    } else if ( f <= -max_exp ) {
      return 0.0;
    } else {
      return exp_table[int((f + max_exp) * (exp_table.size() / max_exp / 2))];
    }
  }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The pointer type wrapper.
///
template<class _Type>
class Ptr {

 protected:

  /// The object.
  _Type obj_;

 public:

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief  Construct with given object.
  ///
  Ptr( const _Type &obj ) noexcept : obj_(obj) {}

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief  Indirection operator.
  ///
  inline _Type* operator->() noexcept { return &obj_; }


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @copydoc  operator->()
  ///
  inline const _Type* operator->() const noexcept { return &obj_; }

};

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

inline timepoint_t GetTime() {
  return std::chrono::high_resolution_clock::now();
}

inline double GetTimeDuration( const timepoint_t &cstart, const timepoint_t &cend ) {
  return std::chrono::duration<double>(cend-cstart).count();
}

inline void DisplayDuration( std::ostream &os, const string_t &name, const timepoint_t &cstart, const timepoint_t &cend ) {
  os << name << ": " << std::fixed << std::setprecision(8) << GetTimeDuration(cstart, cend) << " seconds." << std::endl;
}

}  // namespace hwe

#endif  // HWE_UTIL_HXX_
