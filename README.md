# Luaconfig

Luaconfig is a header-only library that simplifies the use of Lua as a configuration language for C++. It handles stack management and provides a clean interface inspired by [libconfig](https://github.com/hyperrealm/libconfig).

## Getting Started

For Linux users, run the install script provided which will copy all headers to the directory `/usr/local/include`.

Alternatively, simply ensure that the project root directory is located within your compiler include path.

Luaconfig requires at least C++11 and Lua 5.3.

## Using Luaconfig

To include Luaconfig, add the following line to the top of your C++ files:

```
#include <luaconfig/luaconfig.hpp>
```

### The Config class

The namespace `luaconfig` contains the class `Config`, which provides access to Lua's global scope. To create a `Config`, it must be supplied with an appropriate filename:

```
luaconfig::Config cfg("my_lua_script.lua");
```

From here, you may access any Lua variables using the function `Config::get`. The desired type must be provided as a template argument. For example, if your Lua file reads:

```
-- my_lua_script.lua
a = 2
b = 2.6
c = true
d = "hello world"
```

we may access these variables in C++ with:

```
luaconfig::Config cfg("my_lua_script.lua");
auto a = cfg.get<int>("a");         // a = 2
auto b = cfg.get<float>("b");       // b = 2.6
auto c = cfg.get<bool>("c");        // c = true
auto d = cfg.get<std::string>("d"); // d = "hello world"
```

Note that when retrieving integers, `get` may return any integral type, e.g. `get<short>`, `get<long unsigned long>`, etc. Similarly, either `get<float>` or `get<double>` may be used.

New variables may be written to Lua's global scope in the following way:

```
cfg.set("x",6);
```

### The Setting class

Tables are accessible using the class `Setting`. These provide similar get/set functions as `Config`, though they provide lookup relative to a table rather than global scope and also allow integer indexing. For example, if we modify our Lua script to include the following:

```
table = { a = "hello world" }
array = { 1, 2, 3 }
```

we may access these variables with the following:

```
// get table.a
auto table = cfg.get<luaconfig::Setting>("table");
auto a = table.get<std::string>("a");

// print array contents
auto array = cfg.get<luaconfig::Setting>("array");
for( int i=1; i <= array.len(); ++i){
    std::cout << array.get<int>(i) << std::endl;
}
```

### Default values

For both `Config` and `Setting` objects, it is possible to provide a default value when calling `get`. This will be selected if the requested variable doesn't exist or is an unexpected type. This feature is best used to access optional fields in your configuration files. If a default value is not provided and a lookup fails, `get` will throw a `TypeMismatchException` (where a match to 'nil' usually means a variable doesn't exist).

```
auto x = cfg.get<int>("x",0);
```

## Upcoming Features

* `Function` class that allows simplified access to Lua functions from C++
* "Dot notation" lookup, such as `cfg.get<int>("path.to.var")`

## Licensing

This project is licensed under the MIT License -- see the [LICENSE.md](LICENSE.md) file for details. If you're using Luaconfig in your own work, there's no need to provide credit, though it would be highly appreciated.
