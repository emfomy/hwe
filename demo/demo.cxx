////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file       demo/demo.cxx
/// \brief      The demo code.
///
/// \author     Mu Yang <<emfomy@gmail.com>>
/// \author     Fann Jhih-Sheng <<fann1993814@gmail.com>>
/// \copyright  Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng. All rights reserved.
///

#include <hwe.hxx>

#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>

using namespace hwe;

using namespace std::string_literals;

int main() {

  Model model;

  model.train_file      = "alice.txt";
  model.output_file     = "alice.emb";
  model.save_vocab_file = "alice.vocab";

  model.num_threads = 32;;

  int a = 0;
  Word word;
  for (int i = 0; i < 1; ++i) {
    // std::ifstream fin(model.train_file);
    FILE *fin = fopen(model.train_file.c_str(), "rb");
    while ( true ) {
      word.read(fin);
      if ( !word ) break;
      a ^= word.Hash();
    }
    printf("%d\t%d\r", i, a);
  }
  printf("\n");

  // long long a = 0;
  // string_t word;
  // for (int i = 0; i < 10; ++i) {
  //   std::ifstream fin(model.train_file);
  //   Reader reader(fin);
  //   while ( reader(word) ) {
  //     a += word.length();
  //   }
  //   printf("%d\t%lld\r", i, a);
  // }
  // printf("\n");

  // long long a = 0;
  // string_t word;
  // for (int i = 0; i < 10; ++i) {
  //   std::ifstream fin(model.train_file);
  //   while (true) {
  //     model.vocab.ReadWord(word, fin);
  //     if ( !fin.good() ) break;
  //     a += word.length();
  //   }
  //   printf("%d\t%lld\r", i, a);
  // }
  // printf("\n");
  // std::cout << word << "\n";

  // long long a = 0;
  // string_t word, line;
  // std::istream_iterator<string_t> itend;
  // for (int i = 0; i < 10; ++i) {
  //   std::ifstream fin(model.train_file);
  //   while ( std::getline(fin, line) ) {
  //     std::istringstream iss(line);
  //     std::istream_iterator<string_t> it(iss);
  //     while ( it != itend ) {
  //       a += it->length();
  //       ++it;
  //     }
  //     a += "</s>"s.length();
  //   }
  //   printf("%d\t%lld\r", i, a);
  // }
  // printf("\n");

  // long long a = 0;
  // string_t word, line;
  // std::istream_iterator<string_t> itend;
  // for (int i = 0; i < 10; ++i) {
  //   std::ifstream fin(model.train_file);
  //   while ( fin >> word ) {
  //     a += word.length();
  //   }
  //   fin.clear();
  //   fin.seekg(0, fin.beg);
  //   a += std::count(std::istreambuf_iterator<char>(fin),
  //                   std::istreambuf_iterator<char>(), '\n') * "</s>"s.length();
  //   printf("%d\t%lld\r", i, a);
  // }
  // printf("\n");

  // long long a = 0;
  // string_t word, line;
  // std::istream_iterator<string_t> itend, it2;
  // for (int i = 0; i < 10; ++i) {
  //   std::ifstream fin(model.train_file);
  //   std::istream_iterator<string_t> it(fin);
  //   while ( it != itend ) {
  //     a += it->length();
  //     ++it;
  //   }
  //   fin.clear();
  //   fin.seekg(0, fin.beg);
  //   a += std::count(std::istreambuf_iterator<char>(fin),
  //                   std::istreambuf_iterator<char>(), '\n') * "</s>"s.length();
  //   printf("%d\t%lld\r", i, a);
  // }
  // printf("\n");

  // string_t line = "originated as a term of abuse first used against early\n";
  // std::istringstream iss(line);
  // std::copy(std::istream_iterator<string_t>(iss),
  //           std::istream_iterator<string_t>(),
  //           std::ostream_iterator<string_t>(std::cout, "\n"));

  // std::istream_iterator<string_t> itend;
  // std::istream_iterator<string_t> it(fin);
  // for (int i = 0; i < 100; ++i) {
  //   std::cout << *it << "\n";
  //   ++it;
  // }
  // // while ( it != itend ) {
  // //   std::cout << *it << "\n";
  // //   ++it;
  // // }

  return 0;
}
