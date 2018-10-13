#include "cmdlp.hpp"
#include <iostream>

struct local_options {
  int alpha;
  int beta;
  bool flag;
  bool neg_flag;
  void init(cmdlp::parser& p) {
    using namespace cmdlp;
    p.add(make_option(alpha))
    .desc("The alpha value.")
    .name('a', "alpha")
    .name("ALPHA")
    .fallback()
    ;
    p.add(make_option(beta))
    .desc("The beta value.")
    .name('b', "beta")
    .is_required()
    ;
    p.add(make_onswitch(flag))
    .desc("A flag.")
    .name('f')
    .name("flag")
    ;
    p.add(make_offswitch(neg_flag))
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


