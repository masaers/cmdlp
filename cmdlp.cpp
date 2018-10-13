#include "cmdlp.hpp"
#include <sstream>

cmdlp::parser::~parser() {
  using namespace std;
  for (auto it = begin(options_m); it != end(options_m); ++it) {
    delete *it;
    *it = NULL;
  }
}

std::string cmdlp::parser::usage() const {
  using namespace std;
  ostringstream s;
  for (const auto& opt : options_m) {
    if (opt->required()) {
      auto it = bindings_m.find(opt);
      if (it != bindings_m.end()) {
        s << ' ';
        print_call(s, it->second.first, it->second.second, false);
      }
    }
  }
  return s.str();
}

std::string cmdlp::parser::help() const {
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

std::string cmdlp::parser::summary() const {
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

bool cmdlp::parser::bind(option_i* opt, const char flag) {
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

bool cmdlp::parser::bind(option_i* opt, const std::string& name) {
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

void cmdlp::parser::print_call(std::ostream& s, const std::vector<std::string>& names, std::vector<char> flags, bool print_all) {
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

