#include "cmdlp.hpp"
#include <sstream>
#include <fstream>

const char* com::masaers::cmdlp::unescape_until(const char* first, const char* terminators, std::string& out) {
  static const char* quotes = "'\"";
  for (/**/; first != nullptr && *first != '\0' && strchr(terminators, *first) == nullptr; ++first) {
    if (strchr(quotes, *first) == nullptr) {
      if (*first == '\\') {
        ++first;
      }
      if (*first != '\0') {
        out.push_back(*first);
      } else {
        first = nullptr;
      }
    } else {
      const char quote = *first++;
      for (/**/; first != nullptr && *first != '\0' && *first != quote; ++ first) {
        out.push_back(*first);
      }
      if (*first != quote) {
        first = nullptr;
      }
    }
  }
  return first;
}

void com::masaers::cmdlp::escape_str(const char quote, const std::string& str, std::ostream& out) {
  if (quote == '\'' || quote == '"') {
    out << quote;
    for (const char& c : str) {
      if (c == quote) {
        out << '\\';
      }
      out << c;
    }
    out << quote;
  } else {
    for (const char& c : str) {
      if (c == ' ') {
        out << '\\';
      }
      out << c;
    }
  }
}


void com::masaers::cmdlp::value_option<com::masaers::cmdlp::config_files>::assign(const char* str) {
  base_class::read()(*config_files_m, str);
  error_count_m += base_class::parser_ptr()->parse_file(config_files_m->filenames().back().c_str());
}

void com::masaers::cmdlp::value_option<com::masaers::cmdlp::config_files>::evaluate(std::ostream& os) const {
  const auto& fns = config_files_m->filenames();
  os << '[';
  for (auto it = fns.begin(); it != fns.end(); ++it) {
    if (it != fns.begin()) {
      os << ',';
    }
    os << *it;
  }
  os << ']';
}

com::masaers::cmdlp::parser::~parser() {
  using namespace std;
  for (auto it = begin(options_m); it != end(options_m); ++it) {
    delete *it;
    *it = NULL;
  }
}

std::string com::masaers::cmdlp::parser::usage() const {
  using namespace std;
  ostringstream s;
  for (const auto& opt : options_m) {
    if (opt->in_usage()) {
      auto it = bindings_m.find(opt);
      if (it != bindings_m.end()) {
        s << ' ';
        print_call(s, it->second.first, it->second.second, false);
      }
    }
  }
  return s.str();
}

std::string com::masaers::cmdlp::parser::help() const {
  using namespace std;
  ostringstream s;
  for (const auto& opt : options_m) {
    auto it = bindings_m.find(opt);
    if (it != bindings_m.end()) {
      print_call(s, it->second.first, it->second.second, true);
    } else {
      s << "n/n";
    }    
    s << '=';
    opt->evaluate(s);
    s << endl << "    ";
    opt->describe(s);
    s << endl;
  }
  return s.str();
}

std::string com::masaers::cmdlp::parser::summary() const {
  using namespace std;
  ostringstream s;
  for (const auto& opt : options_m) {
    auto it = bindings_m.find(opt);
    if (it != bindings_m.end()) {
      if (! it->second.first.empty()) {
        s << it->second.first.front();
      } else {
        s << it->second.second.front();
      }
    } else {
      s << "<unnamed option>";
    }
    s << '=';
    opt->evaluate(s);
    s << endl;
  }
  return s.str();
}

bool com::masaers::cmdlp::parser::bind(option_i* opt, const char flag) {
  auto p = flags_m.insert(std::make_pair(flag, opt));
  if (! p.second) {
    std::ostringstream s;
    s << "Failed to bind option to flag '-" << flag << "'. "
      << "It already exists.";
    throw std::runtime_error(s.str().c_str());
  } else {
    bindings_m[opt].second.push_back(flag);
  }
  return p.second;
}

bool com::masaers::cmdlp::parser::bind(option_i* opt, const std::string& name) {
  auto p = names_m.insert(std::make_pair(name, opt));
  if (! p.second) {
    std::ostringstream s;
    s << "Failed to bind option to name '--" << name << "'. "
      << "It already exists.";
    throw std::runtime_error(s.str().c_str());
  } else {
    bindings_m[opt].first.push_back(name);
  }
  return p.second;
}

void com::masaers::cmdlp::parser::print_call(std::ostream& s, const std::vector<std::string>& names, std::vector<char> flags, bool print_all) {
  using namespace std;
  for (auto it = begin(names); it != end(names); ++it) {
    if (it != begin(names)) {
      s << "|";
    }
    const auto pos = find(begin(flags), end(flags), it->front());
    if (pos != end(flags)) {
      flags.erase(pos);
      s << "-[-" << it->front() << "]" << it->substr(1);
    } else {
      s << "--" << *it;
    }
  }
  for (auto it = begin(flags); it != end(flags); ++it) {
    if (names.size() != 0 || it != begin(flags)) {
      s << "|";
    }
    s << "-" << *it;
  }
}


std::size_t com::masaers::cmdlp::parser::parse_file(const char* filename) const {
  using namespace std;
  size_t error_count = 0;
  ifstream ifs(filename);
  if (ifs) {
    for (std::string line; getline(ifs, line); ) {
      const auto sep = line.find('=');
      const auto it = names_m.find(line.substr(0, sep));
      if (it != names_m.end()) {
        option_i* opt = it->second;
        opt->observe();
        try {
          string unesc;
          const char* last = unescape_until(line.c_str() + sep + 1, " \t", unesc);
          if (last == nullptr || *last != '\0') {
            throw std::runtime_error("Malformed value");
          }
          opt->assign(unesc.c_str());
        } catch(const std::exception& e) {
          *erros_m << e.what() << std::endl;
          ++error_count;
        } // try
      }
    }
  } else {
    *erros_m << "Failed to open file: '" << filename << "'" << endl;
    ++error_count;
  }
  return error_count;
}

std::size_t com::masaers::cmdlp::parser::validate() const {
  std::size_t result = 0;
  for (const auto& opt : options_m) {
    if (! opt->validate()) {
      auto it = bindings_m.find(opt);
      if (it != bindings_m.end()) {
        *erros_m << "Required option '";
        print_call(*erros_m, it->second.first, it->second.second, false);
        *erros_m << "' not set." << std::endl;
        ++result;
      }
    }
  }
  return result;
}
