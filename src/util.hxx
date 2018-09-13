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

#include "def.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

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

}  // namespace hwe

#endif  // HWE_UTIL_HXX_
