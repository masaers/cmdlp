#ifndef COM_MASAERS_CMDLP_IOOPTION_HPP
#define COM_MASAERS_CMDLP_IOOPTION_HPP
#include "cmdlp.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

namespace com { namespace masaers { namespace cmdlp {
	const char stdio_filename[] = "-";

	template<typename Char, typename Traits = std::char_traits<Char> >
	class basic_ifile {
	public:
		basic_ifile() : filename_m(), ifsp_m(nullptr), sp_m(nullptr) {}
		basic_ifile(const std::string& filename) : filename_m(filename), ifsp_m(nullptr), sp_m(nullptr) {} 
		basic_ifile(const basic_ifile&) = default;
		basic_ifile(basic_ifile&&) = default;
		basic_ifile& operator=(const basic_ifile&) = default;
		basic_ifile& operator=(basic_ifile&&) = default;
		template<typename C, typename T>
		friend inline std::basic_istream<C, T>&
		operator>>(std::basic_istream<C, T>& is, basic_ifile& x) {
		  return is >> x.filename_m;
		}
		template<typename C, typename T>
		friend inline std::basic_ostream<C, T>&
		operator<<(std::basic_ostream<C, T>& os, const basic_ifile& x) {
		  return os << x.filename_m;
		}
		std::basic_istream<Char, Traits>& stream() const { return *sp_m; }
		inline bool validate() {
			bool result = false;
			if (! filename_m.empty()) {
				if (filename_m == stdio_filename) {
					sp_m = &std::cin;
					result = true;
				} else {
					ifsp_m.reset(new std::basic_ifstream<Char, Traits>(filename_m));
					sp_m = ifsp_m.get();
					result = true;
				}
			}
			return result;
		}
	protected:
		std::string filename_m;
		std::shared_ptr<std::basic_ifstream<Char, Traits> > ifsp_m;
		std::basic_istream<Char, Traits>* sp_m;
	}; // basic_ifile

	template<bool IsOptional, typename Char, typename Traits = std::char_traits<Char> >
	class basic_ofile {
	public:
		basic_ofile() : filename_m(), ofsp_m(nullptr), sp_m(nullptr) {}
		basic_ofile(const std::string& filename) : filename_m(filename), ofsp_m(nullptr), sp_m(nullptr) {}
		basic_ofile(const basic_ofile&) = default;
		basic_ofile(basic_ofile&&) = default;
		basic_ofile& operator=(const basic_ofile&) = default;
		basic_ofile& operator=(basic_ofile&) = default;
		template<typename C, typename T>
		friend inline std::basic_istream<C, T>&
		operator>>(std::basic_istream<C, T>& is, basic_ofile<IsOptional, Char, Traits>& x) {
			return is >> x.filename_m;
		}
		template<typename C, typename T>
		friend inline std::basic_ostream<C, T>&
		operator<<(std::basic_ostream<C, T>& os, const basic_ofile<IsOptional, Char, Traits>& x) {
			return os << x.filename_m;
		}
		std::ostream& stream() const { return *sp_m; }
		inline bool validate() {
			bool result = IsOptional;
			if (! filename_m.empty()) {
				if (filename_m == stdio_filename) {
					sp_m = &std::cout;
					result = true;
				} else {
					ofsp_m.reset(new std::basic_ofstream<Char, Traits>(filename_m));
					sp_m = ofsp_m.get();
					result = true;
				}
			}
			return result;
		}
		inline bool is_set() const { return sp_m != nullptr; }
	protected:
		std::string filename_m;
		std::shared_ptr<std::basic_ofstream<Char, Traits> > ofsp_m;
		std::basic_ostream<Char, Traits>* sp_m;
	}; //basic_ofile


	using ifile = basic_ifile<char>;
	using ofile = basic_ofile<false, char>;
	using optional_ofile = basic_ofile<true, char>;


	template<typename Char, typename Traits>
	inline value_option<basic_ifile<Char, Traits> > make_knob(basic_ifile<Char, Traits>& value) {
		return value_option<basic_ifile<Char, Traits> >(value).validator([](basic_ifile<Char, Traits>& x) { return x.validate(); });
	}

	template<bool IsOptional, typename Char, typename Traits>
	inline value_option<basic_ofile<IsOptional, Char, Traits> > make_knob(basic_ofile<IsOptional, Char, Traits>& value) {
	  return value_option<basic_ofile<IsOptional, Char, Traits> >(value).validator([](basic_ofile<IsOptional, Char, Traits>& x) { return x.validate(); });
	}

} } }

#endif
