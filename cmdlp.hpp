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

  class option_i {
  public:
    virtual ~option_i() {}
    virtual bool need_arg() const = 0;
    virtual void observe() = 0;
    virtual void assign(const char* str) = 0;
    virtual void describe(std::ostream& os) const = 0;
    virtual void evaluate(std::ostream& os) const = 0;
    virtual bool validate() const = 0;
    virtual bool required() const = 0;
  }; // option_i
  
  template<typename T>
  class option_crtp : public option_i {
    inline T& me() { return static_cast<T&>(*this); }
    inline const T& me() const { return static_cast<const T&>(*this); }
  public:
    inline option_crtp() : count_m(0), parser_ptr_m(nullptr), desc_m(), required_m(false) {}
    inline option_crtp(const option_crtp&) = default;
    inline option_crtp(option_crtp&&) = default;
    virtual ~option_crtp() {}
    virtual bool validate() const {
      return ! (required() && count() == 0);
    }
    virtual void observe() { ++count_m; }
    virtual bool required() const { return required_m; }
    template<typename U> inline T& desc(U&& str) {
      desc_m = std::forward<U>(str);
      return me();
    }
    template<typename... U> inline T& name(U&&... args) {
      me().parser_ptr()->name(static_cast<option_i*>(&me()), std::forward<U>(args)...);
      return me();
    }
    inline T& is_required() {
      required_m = true;
      return me();
    }
    inline std::size_t& count() { return count_m; }
    inline const std::size_t& count() const { return count_m; }
    inline parser* const& parser_ptr() const { return parser_ptr_m; }
    inline parser*& parser_ptr() { return parser_ptr_m; }
    inline const std::string& desc() const { return desc_m; }
    inline std::string& desc() { return desc_m; }
    inline bool& required() { return required_m; }
  protected:
    std::size_t count_m;
    parser* parser_ptr_m;
    std::string desc_m;
    bool required_m;
  }; // option_crtp
  
  template<typename T>
  class value_option : public option_crtp<value_option<T> > {
    typedef option_crtp<value_option<T> > base_class; 
  public:
    value_option(T& value) : option_crtp<value_option<T> >(), value_m(&value) {}
    virtual ~value_option() {}
    virtual void assign(const char* str) {
      if (str == NULL) {
        throw std::runtime_error("Nothing to read value from!");
      }
      std::istringstream s(str);
      if (s >> *value_m) {
        // Value successfully read
      } else {
        throw std::runtime_error("Failed to read value.");
      }
    }
    virtual bool need_arg() const { return true; }
    virtual void describe(std::ostream& os) const { os << this->desc(); }
    virtual void evaluate(std::ostream& os) const { os << *value_m; }
    inline value_option& fallback() {
      ++base_class::count();
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
  
  template<typename Container>
  class container_option : public option_crtp<container_option<Container> > {
    typedef option_crtp<container_option<Container> > base_class;
  public:
    container_option(Container& container) : base_class(), container_m(&container) {}
    virtual ~container_option() {}
    virtual void assign(const char* str) {
      if (str == nullptr) {
        throw std::runtime_error("Expected a parameter, got an empty string.");
      }
      std::istringstream s(str);
      typename Container::value_type v;
      if (s >> v) {
        container_m->insert(container_m->end(), v);
      } else {
        throw std::runtime_error("Failed to read value.");
      }
    }
    virtual bool need_arg() const { return true; }
    virtual void describe(std::ostream& os) const { os << this->desc(); }
    virtual void evaluate(std::ostream& os) const {
      os << '[';
      for (auto it = container_m->begin(); it != container_m->end(); ++it) {
        if (it != container_m->begin()) {
          os << ',';
        }
        os << *it;
      }
      os << ']';
    }
  private:
    Container* container_m;
  }; // container_option

  template<>
  class value_option<bool> : public option_crtp<value_option<bool> > {
    typedef option_crtp<value_option<bool> > base_class;
  public:
    value_option(bool& value, std::size_t max_count = -1): option_crtp<value_option<bool> >(), value_m(&value), max_count_m(max_count) {}
    virtual ~value_option() {}
    virtual bool need_arg() const { return false; }
    virtual void observe() {
      if (this->count() < max_count_m) {
        *value_m = ! *value_m;
      }
      base_class::observe();
    }
    virtual void assign(const char* str) {}
    virtual void describe(std::ostream& os) const {
      os << this->desc();
    }
    virtual void evaluate(std::ostream& os) const {
      os << (*value_m ? "yes" : "no");
    }
  private:
    bool* value_m;
    std::size_t max_count_m;
  }; // value_option<bool>

  /**
  The specialization of value_option for C strings is intentinally left undefined.
  Use a proper C++ std::string instead! Take control of your memory!
  */
  template<> class value_option<char*>;
  template<> class value_option<const char*>;

  /**
  Creates an option tied to the provided value.
  */
  template<typename T>
  inline value_option<T> make_option(T& value) {
    return value_option<T>(value);
  }


  /**
  A switch option flips a Boolean every time it is given.
  */
  inline value_option<bool> make_switch(bool& value) {
    return value_option<bool>(value, -1);
  }
  /**
  An on-switch "turns on" a Boolean (false -> true) when flipped (regardless of how many times).
  */
  inline value_option<bool> make_onswitch(bool& value) {
    value = false;
    return value_option<bool>(value, 1);
  }
  /**
  An off-switch "turns off" a Boolean (true -> false) when flipped (regardless of how many times).
  */
  inline value_option<bool> make_offswitch(bool& value) {
    value = true;
    return value_option<bool>(value, 1);
  }

  /**
  Inserts values to a container everytime the option is set.
  */
  template<typename Container>
  inline container_option<Container> make_container_option(Container& container) {
    return container_option<Container>(container);
  }


  class parser {
  public:
    ~parser();
    std::string usage() const;
    std::string help() const;
    std::string summary() const;
    
    template<typename arg_it_T = null_output_iterator, typename erros_T = std::ostream&>
    std::size_t parse(const int argc,
                      const char** argv,
                      arg_it_T&& arg_it = arg_it_T(),
                      erros_T&& erros = std::cerr) const;
    template<typename erros_T = std::ostream&>
    std::size_t validate(erros_T&& erros = std::cerr) const;

    template<typename opt_T>
    inline typename std::decay<opt_T>::type& add(opt_T&& opt) {
      typedef typename std::decay<opt_T>::type opt_type;
      opt_type* opt_ptr = new opt_type(std::forward<opt_T>(opt));
      options_m.push_back(opt_ptr);
      opt_ptr->parser_ptr() = this;
      return *opt_ptr;
    }
    void name(option_i* opt, const char flag, const char* const name) {
      bind(opt, flag);
      bind(opt, name);
    }
    void name(option_i* opt, const char* const name) {
      bind(opt, name);
    }
    void name(option_i* opt, const char flag) {
      bind(opt, flag);
    }
    bool bind(option_i* opt, const char flag);
    bool bind(option_i* opt, const char* const name) {
      return bind(opt, std::string(name));
    }
    bool bind(option_i* opt, const std::string& name);
  private:
    static void print_call(std::ostream& s, const std::vector<std::string>& names, std::vector<char> flags, bool print_all);
    std::vector<option_i*> options_m;
    std::unordered_map<option_i*, std::pair<std::vector<std::string>, std::vector<char> > > bindings_m;
    std::unordered_map<char, option_i*> flags_m;
    std::unordered_map<std::string, option_i*> names_m;
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
            option_i* opt = it->second;
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
              option_i* opt = it->second;
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


template<typename erros_T>
std::size_t cmdlp::parser::validate(erros_T&& erros) const {
  std::size_t result = 0;
  for (const auto& opt : options_m) {
    if (! opt->validate()) {
      auto it = bindings_m.find(opt);
      if (it != bindings_m.end()) {
        erros << "Required option '";
        print_call(erros, it->second.first, it->second.second, false);
        erros << "' not set." << std::endl;
        ++result;
      }
    }
  }
  return result;
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
  error_count_m += p.parse(argc, argv, back_inserter(args));
  error_count_m += p.validate();
  if (error_count_m != 0 || help) {
    cerr << endl << "usage: " << argv[0] << p.usage() << endl << endl << p.help() << endl;
  } if (summarize) {
    cerr << p.summary();
  } else {
    // keep calm and continue as usual
  }
}

#endif
