#include "cmdlp.hpp"

cmdlp::parser::~parser() {
  using namespace std;
  for (auto it = begin(options_m); it != end(options_m); ++it) {
    delete *it;
    *it = NULL;
  }
}

std::string cmdlp::parser::help() const {
  using namespace std;
  ostringstream s;
  for (auto it = begin(options_m); it != end(options_m); ++it) {
    auto jt = bindings_m.find(*it);
    if (jt != bindings_m.end()) {
      print_call(s, jt->second.first, jt->second.second, true);
    } else {
      s << "n/n";
    }
    s << '=';
    (**it).evaluate(s);
    s << endl;
    (**it).describe(s);
    s << endl;
  }
  return s.str();
}

bool cmdlp::parser::bind(option* opt, const char flag) {
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

bool cmdlp::parser::bind(option* opt, const std::string& name) {
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

