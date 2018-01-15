// Function.cpp
//
// Unit test for Function.hpp

#include <luaconfig/luaconfig.hpp>
#include <cstdlib>
#include <iostream>
#include <iomanip>

int main(void)
{

    // Open file
    luaconfig::Config cfg("test.lua");

    // Read functions
    {
        std::cout << "Testing function f(a)=a, a=12" << std::endl;
        auto f = cfg.get<luaconfig::Function<double(double)>>("f");
        std::cout << f(12) << std::endl;
        std::cout << "Retesting function for reentry, a=16" << std::endl;
        std::cout << f(16) << std::endl;
    }


    // More complicated functions
    {
        std::cout << "Testing function g(a,b)=a+b, a=3, b=5.5" << std::endl;
        auto g = cfg.get<luaconfig::Function<double(double,double)>>("g");
        std::cout << g(3,5.5) << std::endl;
        std::cout << "Retesting function for reentry, a=36.2, b=4" << std::endl;
        std::cout << g(36.2,4) << std::endl;
    }

    // concat
    {
        std::cout << "Testing function h(a,b)=a..b, a=\"string\", b=64" << std::endl;
        auto h = cfg.get<luaconfig::Function<std::string(const char*,int)>>("h");
        auto x = h("string",64);
        std::cout << x << std::endl;
    }

    // multiple return
    {
        std::cout << "Testing multiple return m(a)=(a,a+1,a+2), a=3" << std::endl;
        auto m = cfg.get<luaconfig::Function<std::tuple<int,int,int>(int)>>("m");
        auto x = m(3);
        std::cout << std::get<0>(x) << ", " << std::get<1>(x) << ", " << std::get<2>(x) << std::endl;
        std::cout << "Retesting function for reentry, a=15" << std::endl;
        auto y = m(15);
        std::cout << std::get<0>(y) << ", " << std::get<1>(y) << ", " << std::get<2>(y) << std::endl;
    }

    // std::function
    {
        std::cout << "Testing std::function, return m(a)=(a,a+1,a+2), a=1" << std::endl;
        auto m = cfg.get<std::function<std::tuple<int,int,int>(int)>>("m");
        auto x = m(1);
        std::cout << std::get<0>(x) << ", " << std::get<1>(x) << ", " << std::get<2>(x) << std::endl;
    }

    return EXIT_SUCCESS;
}
