#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"

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
