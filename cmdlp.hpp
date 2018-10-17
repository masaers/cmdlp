#ifndef CMDLP_HPP
#define CMDLP_HPP
#include "util.hpp"
// c++
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
// c
#include <cstring>

namespace com { namespace masaers { namespace cmdlp {
  class parser;

  const char* unescape_until(const char* first, const char last, std::string& out);
  
  inline const char* unescape(const char* first, std::string& out) {
    return unescape_until(first, '\0', out);
  }


  /**
  Use an object of this class as the config-file options.
  */
  class config_files {
  public:
    config_files() = default;
    config_files(const config_files&) = default;
    config_files(config_files&&) = default;
    inline std::vector<std::string>& filenames() { return filenames_m; }
    inline const std::vector<std::string>& filenames() const { return filenames_m; }
  private:
    std::vector<std::string> filenames_m;
  }; // config_files


  template<typename T>
  struct from_cstr {
    inline void operator()(T& value, const char* cstr) const {
      std::istringstream s(cstr);
      if (s >> value) {
      // Value successfully read
      } else {
        throw std::runtime_error("Failed to read value.");
      }
    }
  };
  template<>
  struct from_cstr<std::string> {
    inline void operator()(std::string& value, const char* cstr) const {
      value = cstr;
    }
  };
  template<>
  struct from_cstr<bool> {
    inline void operator()(bool& value, const char* cstr) const {
      if (strcmp(cstr, "yes") == 0 || strcmp(cstr, "true") == 0) {
        value = true;
      } else if (strcmp(cstr, "no") == 0 || strcmp(cstr, "false") == 0) {
        value = false;
      }
    }
  };
  template<>
  struct from_cstr<config_files> {
    inline void operator()(config_files& value, const char* cstr) const {
      value.filenames().emplace_back(cstr);
    }
  };
  template<typename Key, typename Value>
  struct from_cstr<std::pair<Key, Value> > {
    inline void operator()(std::pair<Key, Value>& kv, const char* cstr) const {
      std::string key;
      const char* div = unescape_until(cstr + 1, cstr[0], key);
      if (div != nullptr && *div == cstr[0]) {
        std::string value;
        const char* end = unescape(div + 1, value);
        if (end != nullptr && *end == '\0') {
          from_cstr<Key>()(kv.first, key.c_str());
          from_cstr<Value>()(kv.second, value.c_str());
        } else {
          throw std::runtime_error("Failed to read value in key value pair.");
        }
      } else {
        throw std::runtime_error("Failed to read key in key value pair.");
      }
    }
  };


  template<typename Value>
  struct to_stream {
    inline void operator()(const Value& value, std::ostream& out) const {
      out << value;
    }
  };
  template<typename Key, typename Value>
  struct to_stream<std::pair<Key, Value> > {
    inline void operator()(const std::pair<Key, Value>& kv, std::ostream& out) const {
      out << ':';
      to_stream<typename std::decay<Key>::type>()(kv.first, out);
      out << ':';
      to_stream<typename std::decay<Value>::type>()(kv.second, out);
    }
  };


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
  
  template<typename T, typename Value>
  class option_crtp : public option_i {
    inline T& me() { return static_cast<T&>(*this); }
    inline const T& me() const { return static_cast<const T&>(*this); }
  public:
    typedef std::function<void(Value&, const char*)> read_func;
    inline option_crtp() : count_m(0), parser_ptr_m(nullptr), desc_m(), required_m(false), read_m(from_cstr<Value>()) {}
    inline option_crtp(const option_crtp&) = default;
    inline option_crtp(option_crtp&&) = default;
    virtual ~option_crtp() {}
    virtual bool need_arg() const { return true; }
    virtual void observe() { ++count_m; }
    virtual void describe(std::ostream& os) const { os << desc_m; }
    virtual bool validate() const { return ! (required() && count() == 0); }
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
    inline T& on_read(const read_func& read) {
      read_m = read;
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
    const read_func& read() const { return read_m; }
    std::size_t count_m;
    parser* parser_ptr_m;
    std::string desc_m;
    bool required_m;
    read_func read_m; 
  }; // option_crtp
  
  template<typename T>
  class value_option : public option_crtp<value_option<T>, T> {
    typedef option_crtp<value_option<T>, T> base_class;
  public:
    value_option(T& value) : base_class(), value_m(&value) {}
    virtual ~value_option() {}
    virtual void assign(const char* str) {
      base_class::read()(*value_m, str);
    }
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
  
  template<typename Container, typename T = typename Container::value_type>
  class container_option : public option_crtp<container_option<Container, T>, T> {
    typedef option_crtp<container_option<Container, T>, T> base_class;
  public:
    container_option(Container& container) : base_class(), container_m(&container) {}
    virtual ~container_option() {}
    virtual void assign(const char* str) {
      T v;
      base_class::read()(v, str);
      container_m->insert(container_m->end(), v);
    }
    virtual void evaluate(std::ostream& os) const {
      os << '[';
      for (auto it = container_m->begin(); it != container_m->end(); ++it) {
        if (it != container_m->begin()) {
          os << ',';
        }
        to_stream<typename Container::value_type>()(*it, os); // os << *it;
      }
      os << ']';
    }
  private:
    Container* container_m;
  }; // container_option

  template<>
  class value_option<bool> : public option_crtp<value_option<bool>, bool> {
    typedef option_crtp<value_option<bool>, bool> base_class;
  public:
    value_option(bool& value, std::size_t max_count = -1): base_class(), value_m(&value), max_count_m(max_count) {}
    virtual ~value_option() {}
    virtual bool need_arg() const { return false; }
    virtual void observe() {
      if (this->count() < max_count_m) {
        *value_m = ! *value_m;
      }
      base_class::observe();
    }
    virtual void assign(const char* str) {
      base_class::read()(*value_m, str);
    }
    // virtual void assign(const char* str) {}
    virtual void evaluate(std::ostream& os) const {
      os << (*value_m ? "yes" : "no");
    }
  private:
    bool* value_m;
    std::size_t max_count_m;
  }; // value_option<bool>

  template<>
  class value_option<config_files> : public option_crtp<value_option<config_files>, config_files> {
    typedef option_crtp<value_option<config_files>, config_files> base_class;
  public:
    value_option(config_files& config_files) : base_class(), config_files_m(&config_files), error_count_m(0) {}
    virtual ~value_option() {}
    virtual void assign(const char* str);
    virtual void evaluate(std::ostream& os) const;
  private:
    config_files* config_files_m;
    std::size_t error_count_m;
  }; // value_option<config_files>

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
  inline value_option<T> make_knob(T& value) {
    return value_option<T>(value);
  }
  template<typename T, typename Alloc>
  inline container_option<std::vector<T, Alloc> > make_knob(std::vector<T, Alloc>& container) {
    return container_option<std::vector<T, Alloc> >(container);
  }
  template<typename T, typename Alloc>
  inline container_option<std::deque<T, Alloc> > make_knob(std::deque<T, Alloc>& container) {
    return container_option<std::deque<T, Alloc> >(container);
  }
  template<typename T, typename Alloc>
  inline container_option<std::list<T, Alloc> > make_knob(std::list<T, Alloc>& container) {
    return container_option<std::list<T, Alloc> >(container);
  }
  template<typename Key, typename Comp, typename Alloc>
  inline container_option<std::set<Key, Comp, Alloc> > make_knob(std::set<Key, Comp, Alloc>& container) {
    return container_option<std::set<Key, Comp, Alloc> >(container);
  }
  template<typename Key, typename Comp, typename Alloc>
  inline container_option<std::multiset<Key, Comp, Alloc> > make_knob(std::multiset<Key, Comp, Alloc>& container) {
    return container_option<std::multiset<Key, Comp, Alloc> >(container);
  }
  template<typename Key, typename Value, typename Comp, typename Alloc>
  inline container_option<std::map<Key, Value, Comp, Alloc>, std::pair<Key, Value> > make_knob(std::map<Key, Value, Comp, Alloc>& container) {
    return container_option<std::map<Key, Value, Comp, Alloc>, std::pair<Key, Value> >(container);
  }
  template<typename Key, typename Value, typename Comp, typename Alloc>
  inline container_option<std::multimap<Key, Value, Comp, Alloc>, std::pair<Key, Value> > make_knob(std::multimap<Key, Value, Comp, Alloc>& container) {
    return container_option<std::multimap<Key, Value, Comp, Alloc>, std::pair<Key, Value> >(container);
  }
  template<typename Key, typename Hash, typename Eq, typename Alloc>
  inline container_option<std::unordered_set<Key, Hash, Eq, Alloc> > make_knob(std::unordered_set<Key, Hash, Eq, Alloc>& container) {
    return container_option<std::unordered_set<Key, Hash, Eq, Alloc> >(container);
  }
  template<typename Key, typename Hash, typename Eq, typename Alloc>
  inline container_option<std::unordered_multiset<Key, Hash, Eq, Alloc> > make_knob(std::unordered_multiset<Key, Hash, Eq, Alloc>& container) {
    return container_option<std::unordered_multiset<Key, Hash, Eq, Alloc> >(container);
  }
  template<typename Key, typename Value, typename  Hash, typename Eq, typename Alloc>
  inline container_option<std::unordered_map<Key, Value, Hash, Eq, Alloc>, std::pair<Key, Value> > make_knob(std::unordered_map<Key, Value, Hash, Eq, Alloc>& container) {
    return container_option<std::unordered_map<Key, Value, Hash, Eq, Alloc>, std::pair<Key, Value> >(container);
  }
  template<typename Key, typename Value, typename  Hash, typename Eq, typename Alloc>
  inline container_option<std::unordered_multimap<Key, Value, Hash, Eq, Alloc>, std::pair<Key, Value> > make_knob(std::unordered_multimap<Key, Value, Hash, Eq, Alloc>& container) {
    return container_option<std::unordered_multimap<Key, Value, Hash, Eq, Alloc>, std::pair<Key, Value> >(container);
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
    parser() : options_m(), bindings_m(), flags_m(), names_m(), erros_m(&std::cerr) {}
    parser(std::ostream& erros) : options_m(), bindings_m(), flags_m(), names_m(), erros_m(&erros) {}
    ~parser();
    std::string usage() const;
    std::string help() const;
    std::string summary() const;
    
    template<typename arg_it_T = null_output_iterator>
    std::size_t parse(const int argc,
                      const char** argv,
                      arg_it_T&& arg_it = arg_it_T()) const;
    std::size_t parse_file(const char* filename) const;
    std::size_t validate() const;

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
    std::ostream* erros_m;
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
  
  
} } } // namespace com::masaers::cmdlp


template<typename arg_it_T>
std::size_t com::masaers::cmdlp::parser::parse(const int argc, const char** argv, arg_it_T&& arg_it) const {
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
              if (first != last) {
                try {
                  opt->assign(*first);
                } catch(const std::exception& e) {
                  *erros_m << e.what() << std::endl;
                  ++error_count;
                } // try
                ++first;
              } else {
                *erros_m << "Attempted to read argument from empty string." << std::endl;
                ++error_count;
              } // if
            } // if
          } else {
            *erros_m << "Unknown command line parameter: '--" << i << "'." << std::endl;
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
                    i = nullptr;
                  } else {
                    i = *first;
                  }
                } // if
                if (i != nullptr) {
                  try {
                    opt->assign(i);
                  } catch (const std::exception& e) {
                    *erros_m << e.what() << std::endl;
                    ++error_count;
                  } // try
                } else {
                  *erros_m << "Attempted to read argument from empty string." << std::endl;
                  ++error_count;
                }
                i = &null_str;
              } // if
            } else {
              *erros_m << "Unknown command line parameter: '-" << *i << "'." << std::endl;
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
com::masaers::cmdlp::options<options_T...>::options(const int argc, const char** argv) : options_T()..., help(false), summarize(false), error_count_m(0) {
  using namespace std;
  parser p(std::cerr);
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
