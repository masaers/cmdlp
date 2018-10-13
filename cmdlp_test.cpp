#include "cmdlp.hpp"
#include <iostream>

struct local_options {
  int alpha;
  bool flag;
  bool neg_flag;
  void init(cmdlp::parser& p) {
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
    neg_flag = true;
    p.add(cmdlp::value_option<bool>(neg_flag))
    .desc("A negative flag.")
    .name("neg_flag")
    .name('F')
    .name("FLAG")
    ;
    return;
  }
};

int main(const int argc, const char** argv) {
  using namespace std;
  cmdlp::options<local_options> o(argc, argv);
  if (! o) {
    return EXIT_FAILURE;
  } else if (o.help) {
    return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}


