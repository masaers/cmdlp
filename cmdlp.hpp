#ifndef CMDLP_HPP
#define CMDLP_HPP
#include "util.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace cmdlp {
  class parser;

  class option {
  public:
    virtual ~option() {}
    virtual bool need_arg() const = 0;
    virtual void observe() { ++count_m; }
    virtual void assign(const char* str) = 0;
    virtual void describe(std::ostream& os) const = 0;
    virtual void evaluate(std::ostream& os) const = 0;
    std::size_t& count() { return count_m; }
    const std::size_t& count() const { return count_m; }
    parser* const& parser_ptr() const { return parser_ptr_m; }
    parser*& parser_ptr() { return parser_ptr_m; }
  private:
    std::size_t count_m;
    parser* parser_ptr_m;
  }; // option
  
  template<typename T>
  class option_crtp {
  public:
    template<typename U>
    T& desc(U&& str) {
      desc_m = std::forward<U>(str);
      return static_cast<T&>(*this);
    }
    template<typename... U>
    T& name(U&&... args) {
      static_cast<T&>(*this).parser_ptr()->name(static_cast<option*>(static_cast<T*>(this)), std::forward<U>(args)...);
      return static_cast<T&>(*this);
    }
    const std::string& desc() const { return desc_m; }
    std::string& desc() { return desc_m; }
  protected:
    std::string desc_m;
  }; // option_crtp
  
  template<typename T>
  class value_option : public option, public option_crtp<value_option<T> > {
  public:
    value_option(T& value)
      : option(), option_crtp<value_option<T> >(), value_m(&value)
    {}
    virtual ~value_option() {}
    virtual void assign(const char* str) {
      if (str == NULL) {
        throw std::runtime_error("Nothing to read value from!");
      }
      std::istringstream s(str);
      if (s >> *value_m) {
      } else {
        throw std::runtime_error("Failed to read value.");
      }
    }
    virtual bool need_arg() const { return true; }
    virtual void describe(std::ostream& os) const {
      os << this->desc();
    }
    virtual void evaluate(std::ostream& os) const {
      os << *value_m;
    }
    inline value_option& fallback() {
      ++option::count();
      return *this;
    }
    template<typename U> inline value_option& fallback(U&& value) {
      *value_m = std::forward<U>(value);
      return fallback();
    }
    template<typename U> inline value_option& preset(U&& value) {
      *value_m = std::forward<U>(value);
      return *this;
    }
    const T& value() const { return *value_m; }
  private:
    T* value_m;
  }; // value_option
  
  template<>
  class value_option<bool> : public option, public option_crtp<value_option<bool> > {
  public:
    value_option(bool& value)
    : option(), option_crtp<value_option<bool> >(), value_m(&value)
    {}
    virtual ~value_option() {}
    virtual void observe() {
      if (this->count() == 0) {
        *value_m = ! *value_m;
      }
      option::observe();
    }
    virtual bool need_arg() const { return false; }
    virtual void assign(const char* str) {}
    virtual void describe(std::ostream& os) const {
      os << this->desc();
    }
    virtual void evaluate(std::ostream& os) const {
      os << (*value_m ? "yes" : "no");
    }
  private:
    bool* value_m;
  }; // value_option<bool>

  template<typename T>
  inline value_option<T> make_option(T& value) {
    return value_option<T>(value);
  }
  inline value_option<bool> make_switch(bool& value) {
    value = false;
    return value_option<bool>(value);
  }
  inline value_option<bool> make_onswitch(bool& value) {
    value = false;
    return value_option<bool>(value);
  }
  inline value_option<bool> make_offswitch(bool& value) {
    value = true;
    return value_option<bool>(value);
  }

  class parser {
  public:
    ~parser();
    std::string usage() const;
    std::string help() const;
    std::string summary() const { return std::string(); }
    
    template<typename arg_it_T = null_output_iterator, typename erros_T = std::ostream&>
    std::size_t parse(const int argc,
                      const char** argv,
                      arg_it_T&& arg_it = arg_it_T(),
                      erros_T&& erros = std::cerr) const;

    template<typename opt_T>
    typename std::decay<opt_T>::type& add(opt_T&& opt) {
      typedef typename std::decay<opt_T>::type opt_type;
      opt_type* opt_ptr = new opt_type(std::forward<opt_T>(opt));
      options_m.push_back(opt_ptr);
      opt_ptr->parser_ptr() = this;
      return *opt_ptr;
    }
    void name(option* opt, const char flag, const char* const name) {
      bind(opt, flag);
      bind(opt, name);
    }
    void name(option* opt, const char* const name) {
      bind(opt, name);
    }
    void name(option* opt, const char flag) {
      bind(opt, flag);
    }
    bool bind(option* opt, const char flag);
    bool bind(option* opt, const char* const name) {
      return bind(opt, std::string(name));
    }
    bool bind(option* opt, const std::string& name);
  private:
    static void print_call(std::ostream& s, const std::vector<std::string>& names, std::vector<char> flags, bool print_all);
    std::vector<option*> options_m;
    std::unordered_map<option*, std::pair<std::vector<std::string>, std::vector<char> > > bindings_m;
    std::unordered_map<char, option*> flags_m;
    std::unordered_map<std::string, option*> names_m;
  }; // parser
  
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
    inline operator bool() const { return error_count_m == 0; }
    bool help = false;
    bool summarize = false;
    std::vector<std::string> args;
  private:
    std::size_t error_count_m;
  }; // options
  
  
} // namespace cmdlp


template<typename arg_it_T, typename erros_T>
std::size_t cmdlp::parser::parse(const int argc, const char** argv, arg_it_T&& arg_it, erros_T&& erros) const {
  static const char null_str = '\0';
  std::size_t error_count = 0;
  const char** first = argv;
  const char** last = argv + argc;
  while (first != last) {
    const char* i = *first;
    if (*i == '-') {
      // '-*' flag, long name or ignore-rest
      ++i;
      if (*i == '-') {
        // '--*' long name
        ++i;
        if (*i == '\0') {
          // '--' ignore-rest
          first = last;
          i = &null_str;
        } else {
          // '--+' long name
          ++first;
          auto it = names_m.find(i);
          if (it != names_m.end()) {
            option* opt = it->second;
            opt->observe();
            if (opt->need_arg()) {
              try {
                opt->assign(*first);
              } catch(const std::exception& e) {
                erros << e.what() << std::endl;
                ++error_count;
              } // try
              ++first;
            } // if
          } else {
            erros << "Unknown command line parameter: '--" << i << "'." << std::endl;
            ++error_count;
          } // if
          i = *first;
        } // if
      } else {
        // '-*' flags
        if (*i == '\0') {
          // free argument
          *arg_it = *first;
          ++first;
          i = *first;
        } else {
          // '-+' one or more flags
          while (*i != '\0') {
            auto it = flags_m.find(*i);
            if (it != flags_m.end()) {
              ++i;
              option* opt = it->second;
              opt->observe();
              if (opt->need_arg()) {
                if (*i == '\0') {
                  ++first;
                  if (first == last) {
                    i = NULL;
                  } else {
                    i = *first;
                  }
                } // if
                try {
                  opt->assign(i);
                } catch (const std::exception& e) {
                  erros << e.what() << std::endl;
                  ++error_count;
                } // try
                i = &null_str;
              } // if
            } else {
              erros << "Unknown command line parameter: '-" << *i << "'." << std::endl;
              ++error_count;
              ++i;
            } // if
          } // while
          if (first != last) {
            ++first;
            i = *first;
          }
        } // if
      } //  if
    } else {
      // free argument
      *arg_it = *first;
      ++first;
      i = *first;
    } // if
  } // while
  return error_count;
}

template<typename... options_T> 
cmdlp::options<options_T...>::options(const int argc, const char** argv) : options_T()..., help(false), summarize(false), error_count_m(0) {
  using namespace std;
  parser p;
  options_helper::init_bases<options<options_T...>, parser, options_T...>(*this, p);
  p.add(value_option<bool>(help))
  .name('h', "help")
  .desc("Prints the help message and exits normally.")
  ;
  p.add(value_option<bool>(summarize))
  .name("summarize")
  .desc("Prints a summary of the parameters as undestood "
    "by the program before running the program.")
  ;
  error_count_m = p.parse(argc, argv, back_inserter(args));
  if (error_count_m != 0 || help) {
    cerr << endl << "usage: " << argv[0] << p.usage() << endl << endl << p.help() << endl;
  } if (summarize) {
    cerr << endl << p.summary() << endl;
  } else {
        // keep calm and continue as usual
  }
}

#endif
