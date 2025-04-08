#ifndef UTIL_CPP
#define UTIL_CPP

#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"
#include <algorithm>

namespace detail {
template <class... Fs>
struct overload;

template <class F>
struct overload<F> : public F {
  explicit overload(F f) : F(f) {
  }
};
template <class F, class... Fs>
struct overload<F, Fs...>
    : public overload<F>
    , public overload<Fs...> {
  overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {
  }
  using overload<F>::operator();
  using overload<Fs...>::operator();
};
}  // namespace detail

/*
 * Thats an absolute magic function, that allows 
 * to conveniently create casts
 */
template <class... F>
auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}


namespace td_api = td::td_api;

using Object = td_api::object_ptr<td_api::Object>;
using Function = td_api::object_ptr<td_api::Function>;


bool isError(Object& obj) { return obj->get_id() == td_api::error::ID; }

void log_error(Object& obj) {
    auto& err = (td_api::error&) *obj;
    std::cout << "Error: [" << err.code_ << "] " << err.message_ << "\n";
}

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

#endif