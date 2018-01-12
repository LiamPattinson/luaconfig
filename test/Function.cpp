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
        auto f = cfg.get<luaconfig::Function<double,double>>("f");
        std::cout << f(12) << std::endl;
        std::cout << "Retesting function for reentry, a=12" << std::endl;
        std::cout << f(12) << std::endl;
    }


    // More complicated functions
    {
        std::cout << "Testing function g(a,b)=a+b, a=3, b=5.5" << std::endl;
        auto g = cfg.get<luaconfig::Function<double,double,double>>("g");
        std::cout << g(3,5.5) << std::endl;
        std::cout << "Retesting function for reentry, a=36.2, b=4" << std::endl;
        std::cout << g(36.2,4) << std::endl;
    }

    // concat
    {
        std::cout << "Testing function h(a,b)=a..b, a=\"string\", b=64" << std::endl;
        auto h = cfg.get<luaconfig::Function<std::string,const char*,int>>("h");
        auto x = h("string",64);
        std::cout << x << std::endl;
    }


    return EXIT_SUCCESS;
}
