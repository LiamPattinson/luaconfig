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

// is_iterable trait class
// Borrows from Stack Overflow:
// * jarod42's answer to question 13830158
// * jrok's answer to question 87372

namespace is_iterable_ns {

    using std::begin;
    using std::end;

    template<class T>
    class is_iterable{

        template<typename U=T>
        static constexpr auto impl(int) -> decltype(
            // test begin() and end() exist, and test != comparison
            begin(std::declval<U&>()) != end(std::declval<U&>()), 
            // Deal with overloaded comma operator
            void(),
            // test prefix ++ exists
            ++std::declval<decltype(begin(std::declval<U&>()))&>(),
            // test dereference operator exists 
            *begin(std::declval<U&>()),
            // Finally, return type.
            std::true_type{});

        template<typename U=T>
        static constexpr std::false_type impl(...);

        using impltype = decltype(impl(0));

    public:

        static constexpr bool value = impltype::value;
    };
}

template<class T>
struct is_iterable{
    static constexpr bool value = is_iterable_ns::is_iterable<T>::value;
};

} // end namespace
#endif
