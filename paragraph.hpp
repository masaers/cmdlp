#ifndef COM_MASAERS_CMDLP_PARAGRAPH_HPP
#define COM_MASAERS_CMDLP_PARAGRAPH_HPP
#include <iostream>
#include <sstream>
#include <string>

#define LOGVAR(expr) std::clog << __FILE__ << ':' << __LINE__ << ' ' << #expr << "='" << (expr) << "'" << std::endl;

namespace com { namespace masaers { namespace cmdlp {

  template<typename Char>
  class pbuf : public std::basic_stringbuf<Char> {
  public:
    typedef std::basic_string<Char> string_type;
  protected:
    std::basic_streambuf<Char>* buf_m;
    std::basic_ostream<Char>* stream_m;
    std::size_t width_m;
    string_type pbreak_m;
    string_type margin_m;
    bool break_at_first_m;
    bool no_overflow_m;
    bool first_paragraph_m;
    virtual int sync() override {
      stream_m->rdbuf(buf_m);
      ragged_right(*stream_m, this->str());
      stream_m->flush();
      this->str("");
      stream_m->rdbuf(this);
      return 0;
    }
    void ragged_right(std::basic_ostream<Char>& os, const string_type& str);
  public:
    inline pbuf(std::basic_ostream<Char>& stream,
      const std::size_t& width,
      const string_type& pbreak,
      const string_type& margin,
      bool break_at_first,
      bool no_overflow)
    : buf_m(stream.rdbuf())
    , stream_m(&stream)
    , width_m(width)
    , pbreak_m(pbreak)
    , margin_m(margin)
    , break_at_first_m(break_at_first)
    , no_overflow_m(no_overflow)
    , first_paragraph_m(true)
    {
      stream_m->rdbuf(this);
    }
    inline pbuf(const pbuf&) = default;
    inline pbuf(pbuf&&) = default;
    inline pbuf& operator=(const pbuf&) = default;
    inline pbuf& operator=(pbuf&&) = default;
    inline ~pbuf() {
      stream_m->rdbuf(buf_m);
      *stream_m << this->str();
      stream_m->flush();
    }
  };

  template<typename Char>
  inline pbuf<Char> paragraph(std::basic_ostream<Char>& stream, const std::size_t width, std::size_t margin) {
    return pbuf<Char>(stream,
                      width,
                      typename pbuf<Char>::string_type(margin, ' '),
                      typename pbuf<Char>::string_type(margin, ' '),
                      false,
                      false);
  }
  template<typename Char>
  inline pbuf<Char> paragraph(std::basic_ostream<Char>& stream, const std::size_t width, std::size_t margin, int pbreak_margin_delta) {
    if (margin + pbreak_margin_delta < 0 || width <= margin || width <= margin + pbreak_margin_delta) {
      throw std::runtime_error("Negative or zero space in paragraph!");
    }
    return pbuf<Char>(stream,
                      width,
                      typename pbuf<Char>::string_type(margin + pbreak_margin_delta, ' '),
                      typename pbuf<Char>::string_type(margin, ' '),
                      false,
                      false);
  }
  template<typename Char>
  inline pbuf<Char> hanging_list(std::basic_ostream<Char>& stream, const std::size_t width, const std::size_t indent) {
    return pbuf<Char>(stream,
                      width,
                      "",
                      typename pbuf<Char>::string_type(indent, ' '),
                      true,
                      false);
  }

} } }


/// \todo Handle unsynched newlines.
template<typename Char>
void com::masaers::cmdlp::pbuf<Char>::ragged_right(std::basic_ostream<Char>& os,
                                                   const typename com::masaers::cmdlp::pbuf<Char>::string_type& str) {
  std::size_t offset = 0;
  while (offset < str.size()) {
    std::size_t indent_offset = 0;
    if (offset == 0 && (break_at_first_m || ! first_paragraph_m)) {
      os << pbreak_m;
      indent_offset = pbreak_m.size();
    } else {
      os << margin_m;
      indent_offset = margin_m.size();
    }
    std::size_t next_offset = offset + width_m - indent_offset;
    if (next_offset < str.size()) {
      for (/**/; next_offset < str.size() && ! isspace(str.at(next_offset)); --next_offset);
      if (next_offset <= offset) {
        next_offset = offset + width_m - indent_offset;
        if (! no_overflow_m) {
          for (/**/; next_offset < str.size() && ! isspace(str.at(next_offset)); ++next_offset);
        }
      }
      for (/**/; next_offset < str.size() &&   isspace(str.at(next_offset)); ++next_offset);
    } else {
      next_offset = str.size();
    }
    std::size_t last_offset = next_offset;
    for (/**/; isspace(str.at(last_offset - 1)); --last_offset);
    os.write(str.c_str() + offset, last_offset - offset);
    os << '\n';
    offset = next_offset;
  }
  first_paragraph_m = false;
}

#endif
