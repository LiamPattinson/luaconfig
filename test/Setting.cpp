// Setting.cpp
//
// Unit test for Setting.hpp
// Additionally relies on Config.hpp to read config file.

#include <luaconfig/luaconfig.hpp>
#include <cstdlib>
#include <iostream>
#include <iomanip>

int main(void){

    // Read test file
    luaconfig::Config cfg("test.lua");

    // Get from a simple Setting
    {
        luaconfig::Setting col = cfg.get<luaconfig::Setting>("color");
        auto r = col.get<double>("r");
        auto g = col.get<double>("g");
        auto b = col.get<double>("b");
        std::cout << r << std::endl;
        std::cout << g << std::endl;
        std::cout << b << std::endl;
        // defaulted get
        auto x = col.get<double>("x",13);
        std::cout << x << std::endl;
    }

    // Get a more varied Setting
    {
        luaconfig::Setting tab = cfg.get<luaconfig::Setting>("table");
        auto f = tab.get<float>("float");
        auto i = tab.get<int>("int");
        auto s = tab.get<std::string>("string");
        auto b = tab.get<bool>("bool");
        std::cout << f << std::endl;
        std::cout << i << std::endl;
        std::cout << s << std::endl;
        std::cout << std::boolalpha << b << std::endl;
        // Get a Setting from a Setting
        auto nest = tab.get<luaconfig::Setting>("table");
        auto n = nest.get<std::string>("string");
        std::cout << n << std::endl;
    }

    // Set something to a Setting
    // Additionally tests multiple Settings on one table
    {
        {
            luaconfig::Setting col = cfg.get<luaconfig::Setting>("color");
            col.set("max",1.0);
        }
        luaconfig::Setting col1 = cfg.get<luaconfig::Setting>("color");
        luaconfig::Setting col2 = cfg.get<luaconfig::Setting>("color");
        auto m1= col1.get<double>("max");
        auto m2= col2.get<double>("max");
        std::cout << m1 << '\t' << m2 << std::endl;
    }

    // Read Setting of an array
    // Remember, Lua counts from 1 to length inclusive!
    {
        luaconfig::Setting arr = cfg.get<luaconfig::Setting>("array");
        for( int i=1; i<= arr.len(); ++i){
            auto x = arr.get<double>(i);
            std::cout << x << std::endl;
        }
        for( int i=1; i<= arr.len(); ++i){
            arr.set(i,2*arr.get<double>(i));
            auto x = arr.get<double>(i);
            std::cout << x << std::endl;
        }
    }

    // Refocusing
    {
        luaconfig::Setting tab = cfg.get<luaconfig::Setting>("table");
        luaconfig::Setting sub = tab.get<luaconfig::Setting>("table");
        auto s1 = sub.get<std::string>("string");
        std::cout << "Before refocus: " << s1 << std::endl;
        tab.refocus(sub,"other_table");
        auto s2 = sub.get<std::string>("string");
        std::cout << "After refocus: " << s2 << std::endl;
    }   

    // Refocusing to read a matrix
    {
        auto mat = cfg.get<luaconfig::Setting>("matrix");
        auto row = mat.get<luaconfig::Setting>(1);
        for( int i=1; i<=mat.len(); ++i){
            if( i != 1 ) mat.refocus(row,i);
            for( int j=1; j<=row.len(); ++j){
                auto x = row.get<double>(j);
                std::cout << x << ' ';
            }
            std::cout << std::endl;
        }

    }

    return EXIT_SUCCESS;
}
