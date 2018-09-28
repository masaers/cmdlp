#include "cmdlp.hpp"
#include <iostream>

int main(const int argc, const char** argv) {
  using namespace std;
  int alpha = 10;
  bool flag;
  bool neg_flag = true;
  cmdlp::parser p;
  p.add(cmdlp::value_option<int>(alpha))
    .desc("The alpha value.")
    .name('a', "alpha")
    .name("ALPHA")
    .fallback()
    ;
  p.add(cmdlp::value_option<bool>(flag))
    .desc("A flag.")
    .name('f')
    .name("flag")
    ;
  p.add(cmdlp::value_option<bool>(neg_flag))
    .desc("A negative flag.")
    .name("neg_flag")
    .name('F')
    .name("FLAG")
    ;
    
  cout << p.help() << endl;
  p.parse(argc, argv);
  cout << p.help() << endl;
  
  return EXIT_SUCCESS;
}


