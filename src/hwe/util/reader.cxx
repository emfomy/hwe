////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       src/hwe/util/reader.cxx
/// \brief      The word reader (implementation).
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>

#include <hwe/util/reader.hxx>

#include <fstream>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The HWE namespace
//
namespace hwe {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Reads a single word from a file, assuming space + tab + EOL to be word boundaries.
///
bool Reader::operator()( string_t &word ) noexcept {
  if ( !(iss_ >> word) ) {
    if ( std::getline(fin_, line_) ) {
      word = "</s>";
      iss_ = std::istringstream(line_);
    } else {
      good_ = false;
      word = "";
    }
  }
  return good_;
}

}  // namespace hwe
