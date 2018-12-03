#include "options.hpp"
#include "magic_enum.hpp"
#include <iostream>
#include <vector>
#include <set>


/**
  Cryptic read function.
*/
void robber_lang(std::string& to, const char* from) {
  for (const char* c = from; *c != '\0'; ++c) {
    switch (*c) {
      case 'B': case 'C': case 'D': case 'F': case 'G': case 'H': case 'J':
      case 'K': case 'L': case 'M': case 'N': case 'P': case 'Q': case 'R':
      case 'S': case 'T': case 'V': case 'W': case 'X': case 'Y': case 'Z':
      to.push_back(*c);
      to.push_back('O');
      to.push_back(*c);
      break;
      case 'b': case 'c': case 'd': case 'f': case 'g': case 'h': case 'j':
      case 'k': case 'l': case 'm': case 'n': case 'p': case 'q': case 'r':
      case 's': case 't': case 'v': case 'w': case 'x': case 'y': case 'z':
      to.push_back(*c);
      to.push_back('o');
      to.push_back(*c);
      break;
      default:
      to.push_back(*c);
    }
  }
}

struct local_options {
  int alpha;
  int beta;
  bool flip;
  bool on;
  bool off;
  std::set<std::string> strings;
  std::vector<std::string> cipher;
  std::map<std::string, float> constants;
  knobs::magic_level::value magic_level;
  // const char* cstr;
  void init(com::masaers::cmdlp::parser& p) {
    using namespace com::masaers::cmdlp;
    p.add(make_knob(alpha))
    .desc("The alpha value.")
    .name('a', "alpha")
    .name("ALPHA")
    .fallback(10)
    ;
    p.add(make_knob(beta))
    .desc("The beta value.")
    .name('b', "beta")
    ;
    p.add(make_switch(flip)).desc("A switch").name('f').name("flip");
    p.add(make_onswitch(on)).desc("Turns on").name("on");
    p.add(make_offswitch(off)).desc("Turns off").name("off");
    p.add(make_knob(strings))
    .desc("Some input strings")
    .name('s', "str")
    ;
    // p.add(make_knob(cstr)).desc("C-strings are bad, mmmkay.").name("cstr");
    p.add(make_knob(cipher))
    .desc("Encrypts the provided strings.")
    .name("cipher")
    .on_read(robber_lang)
    ;
    p.add(make_knob(constants))
    .desc("Named numeric constants.")
    .name('c', "const")
    .fallback({ {"pi", 3.14} })
    ;
    p.add(make_knob(magic_level))
    .desc(knobs::magic_level::desc())
    .name('m', "magic")
    ;
    return;
  }
};

int main(const int argc, const char** argv) {
  using namespace std;
  com::masaers::cmdlp::options<local_options> o(argc, argv);
  if (! o) {
    return o.exit_code();
  }

  switch (o.magic_level) {
    case knobs::magic_level::no_magic:
    break;
    case knobs::magic_level::less_magic:
    break;
    case knobs::magic_level::more_magic:
    break;
    default:
    ;
  }

  return EXIT_SUCCESS;
}


