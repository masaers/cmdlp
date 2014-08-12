#include "cmdlp.hpp"
#include <iostream>

int main(const int argc, const char** argv) {
  using namespace std;
  int alpha = 10;
  bool flag;
  cmdlp::parser p;
  p.add(cmdlp::value_option<int>(alpha))
    .desc("The alpha value.")
    .name('a', "alpha")
    .name("ALPHA")
    //    .name('a')
    .fallback()
    ;
  p.add(cmdlp::value_option<bool>(flag))
    .desc("A flag.")
    .name('f')
    .name("flag")
    ;
  
  cout << p.help() << endl;
  p.parse(argc, argv);
  cout << p.help() << endl;
  
  return EXIT_SUCCESS;
}

