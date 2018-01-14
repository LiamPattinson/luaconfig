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

    // Refocus setting
    {
        auto set = cfg.get<luaconfig::Setting>("color");
        auto r = set.get<double>("r");
        std::cout << "Before refocus:" << r << std::endl;
        cfg.refocus( set, "array");
        auto x = set.get<double>(1);
        std::cout << "After refocus:" << x << std::endl;
    }

    // Dot notation
    {
        auto x = cfg.get<double>("color.r");
        auto y = cfg.get<std::string>("table.string");
        auto z = cfg.get<std::string>("table.table.string");
        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;
    }

    // Integer indexed dot notation
    {
        auto x = cfg.get<double>("array.1");
        auto y = cfg.get<double>("array.2");
        auto z = cfg.get<double>("array.3");
        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;
        auto m = cfg.get<double>("matrix.2.2");
        std::cout << m << std::endl;
    }

    // exists
    {
        bool x = cfg.exists("array");
        bool y = cfg.exists("array.1");
        bool z = cfg.exists("qwerty");
        std::cout << std::boolalpha << x << std::endl;
        std::cout << std::boolalpha << y << std::endl;
        std::cout << std::boolalpha << z << std::endl;
    }

    return EXIT_SUCCESS;
}
