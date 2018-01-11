// Config.cpp
//
// Unit test for Config.hpp

#include <luaconfig/luaconfig.hpp>
#include <cstdlib>
#include <iostream>
#include <iomanip>

int main(void)
{

    // Open file
    luaconfig::Config cfg("test.lua");

    // Test simple reading
    {
        auto x = cfg.get<double>("x");
        auto y = cfg.get<double>("y");
        auto z = cfg.get<double>("z");
        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;
    }

    // Test exceptions
    try{
        luaconfig::Config cfg("not_a_file.lua");
    } catch( const luaconfig::FileException& e){
        std::cout << e.what() << std::endl;
    }

    try{
        auto a = cfg.get<int>("a");
    } catch( const luaconfig::TypeMismatchException& e){
        std::cout << e.what() << std::endl;
    }

    // Test default get
    {
        auto x = cfg.get<double>("not_a_variable",17);
        std::cout << x << std::endl;
    }


    // Now, put SFINAE bits through their paces
    // (compile time tests only)
    // floats
    {auto x = cfg.get<float>("x"); std::cout<<x<<std::endl;}
    // ints
    {char i = cfg.get<char>("i"); std::cout << i << std::endl;}
    {auto i = cfg.get<short>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<unsigned short>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<int>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<unsigned>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<long>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<unsigned long>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<long long>("i"); std::cout<<i<<std::endl;}
    {auto i = cfg.get<unsigned long long>("i"); std::cout<<i<<std::endl;}
    // bool
    {auto b = cfg.get<bool>("b"); std::cout<<std::boolalpha<<b<<std::endl;}
    // string
    {auto s = cfg.get<std::string>("s"); std::cout<<s<<std::endl;}

    // string test
    {auto x = cfg.get<double>(std::string{"x"}); std::cout << x << std::endl;}

    // set globals
    { cfg.set("m",36); auto m = cfg.get<int>("m"); std::cout << m << std::endl;}
    { cfg.set("m",36.2); auto m = cfg.get<float>("m"); std::cout << m << std::endl;}
    { cfg.set("m",true); auto m = cfg.get<bool>("m"); std::cout << m << std::endl;}
    { cfg.set("m","cstr"); auto m = cfg.get<std::string>("m"); std::cout << m << std::endl;}
    { cfg.set("m",std::string{"stdstr"}); auto m = cfg.get<std::string>("m"); std::cout << m << std::endl;}

    // Make Setting (no tests of whether Settings actually work)
    {luaconfig::Setting col = cfg.get<luaconfig::Setting>("color");}

    return EXIT_SUCCESS;
}
