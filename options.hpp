#ifndef COM_MASAERS_CMDLP_OPTIONS_HPP
#define COM_MASAERS_CMDLP_OPTIONS_HPP
#include "cmdlp.hpp"
// c++
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
// c
#include <cstdlib>

namespace com { namespace masaers { namespace cmdlp {
  namespace options_helper {
    template<typename me_T, typename parser_T>
    inline void init_bases(me_T&, parser_T&) {}
    template<typename me_T, typename parser_T, typename T, typename... Ts>
    inline void init_bases(me_T& me, parser_T& p) {
      static_cast<T&>(me).init(p);
      init_bases<me_T, parser_T, Ts...>(me, p);
    }
  } // namespace options_helper
  
  template<typename... options_T>
  class options : public options_T... {
  public:
    inline options(const int argc, const char** argv);
    inline options(const int argc, char** argv) : options(argc, (const char**)argv) {}
    inline operator bool() const { return ! help_needed(); }
    inline int exit_code() const { return error_count_m == 0 ? EXIT_SUCCESS : EXIT_FAILURE; }
    std::vector<std::string> args;
  private:
    inline bool help_needed() const { return help_requested_m || error_count_m != 0; }
    bool help_requested_m;
    std::size_t error_count_m;
  }; // options
  
} } }

template<typename... options_T> 
com::masaers::cmdlp::options<options_T...>::options(const int argc, const char** argv) : options_T()..., help_requested_m(false), error_count_m(0) {
  using namespace std;
  string dumpto;
  config_files configs;
  parser p(cerr);
  options_helper::init_bases<options<options_T...>, parser, options_T...>(*this, p);
  p.add(make_knob(dumpto))
  .name("dumpto")
  .desc("Dumps the parameters, as undestood by the program, to a config file "
    "that can later be used to rerun with the same settings. Leave empty to not dump. "
    "Use '-' to dump to standard output.")
  .fallback()
  .is_meta()
  ;
  p.add(make_knob(configs))
  .name("config")
  .desc("Read parameters from the provided file as if they were provided in the same position on the command line.")
  ;
  p.add(make_onswitch(help_requested_m))
  .name('h', "help")
  .desc("Prints the help message and exits normally.")
  .is_meta()
  ;
  error_count_m += p.parse(argc, argv, back_inserter(args));
  error_count_m += p.validate();
  if (help_needed()) {
    cerr << endl << "usage: " << argv[0] << p.usage() << endl << endl << p.help() << endl;
  } if (! dumpto.empty()) {
    ofstream ofs;
    ostream* out = nullptr;
    if (dumpto == "-") {
      out = &cout;
    } else {
      ofs.open(dumpto);
      if (ofs) {
        out = &ofs;
      } else {
        cerr << "Failed to open file '" << dumpto << "' for dumping parameters." << endl; 
        ++error_count_m;
      }
    }
    if (out != nullptr) {
      p.dumpto_stream(*out, false);
    }
  } else {
    // keep calm and continue as usual
  }
}

#endif