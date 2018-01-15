// utils.hpp
//
// Collection of reusable code bits for luaconfig

#ifndef __LUACONFIG_UTILS_HPP
#define __LUACONFIG_UTILS_HPP

#include <tuple>
#include <functional>

namespace luaconfig {

// Is type T a std::tuple?
template<class T>
struct is_tuple {
    static const bool value = false;
};

template<class... Args>
struct is_tuple<std::tuple<Args...>> {
    static const bool value = true;
};

// Is type T a std::function?
// Additionally provide function signature.
template<class T>
struct is_function {
    static const bool value = false;
};

template<class Arg>
struct is_function<std::function<Arg>> {
    static const bool value = true;
    using sig = Arg;
};

} // end namespace
#endif
