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

The lifetime of a Lua State depends uniquely on its encapsulating `Config` class. Copying is not permitted, but they may be moved.


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

Unlike `Config`, the `Setting` class is copyable. However, as copying a `Setting` will spawn a new Lua thread and duplicate the stack, this will inflict a significant performance penalty as opposed to moving or passing references.

### The Function class

A Lua function may be encapsulated in a `Function` object. This is templated over both the return type and the arguments it takes. As an example, if we have the following simple Lua function:

```
function f(a,b)
    return a..b
end
```

(Where the `..` operator means 'concatenate'). As an unusual application of this in C++, we may try to append an integer to a C-string and convert to a `std::string`:

```
auto f = cfg.get< luaconfig::Function< std::string (const char*,int) >>("f");
auto x = f("string",64);    // Gives std::string{"string64"}
```

`Function` is also capable of returning multiple types via `std::tuple`:

```
-- Lua
function f(a)
    return a, a+1, a+2
end

// C++
auto f = cfg.get<luaconfig::Function< std::tuple<int,int,int>(int)>>("f");
auto x = f(1); // Gives std::tuple<int,int,int>{1,2,3}
```

If the user wishes, they may use `std::function` instead of `luaconfig::Function`:

```
auto f = cfg.get<std::function< std::tuple<int,int,int>(int)>>("f");
```

Internally, this will create a new `luaconfig::Function`, copy it into a `std::function` wrapper, and dispose of the original `luaconfig::Function`. Since this can be a fairly costly procedure, the direct use of `luaconfig::Function` is recommended unless you require the additional capabilities of a `std::function`.

## Other Features

### Dot notation

A lookup can be performed through multiple levels of nested tables using 'dot notation'. This avoids the creation of new `Setting` objects for each nested table, so offers significant performance benefits. For example, in the fairly contrived example of:

```
table = { x = { y = { z = { str = "Hello world!" }}}}
```

One can lookup the inner string directly from global scope:

```
auto str = cfg.get<std::string>("table.x.y.z.str");
```

It is also possible to use integer indexing. For a configuration of:

```
x = { {y=1}, {z=2} }
```

Each of `y` and `z` may be accessed via:

```
auto y = cfg.get<double>("x.1.y");
auto z = cfg.get<double>("x.2.z");
```

### Default values

For both `Config` and `Setting` objects, it is possible to provide a default value when calling `get`. This will be selected if the requested variable doesn't exist or is an unexpected type. This feature is best used to access optional fields in your configuration files. If a default value is not provided and a lookup fails, `get` will throw a `TypeMismatchException` (where a match to 'nil' usually means a variable doesn't exist).

```
auto x = cfg.get<int>("x",0);
```

### Existance Testing

Sometimes, we may not be interested in the details of a given setting, but instead are only concerned whether it exists or not. For that, we may use the member function `exists` with either `Config` or `Setting`.

```
bool x1 = cfg.exists("path.to.setting.x.1");
```

### Reading to Iterables

A table containing homogeneous types may be efficiently read into C++ using iterator methods. For example, if we wish to read and print the following matrix:

```
matrix = {
    { 1.0 , 2.0 , 3.0 },
    { 4.0 , 5.0 , 6.0 },
    { 7.0 , 8.0 , 9.0 },
}
```

This may be performed with: 

```
auto mat = cfg.get<luaconfig::Setting>("matrix");
std::vector<double> v( mat.len(1) ); // mat.len(1) gets number of columns
for( int i=1; i<=mat.len(); ++i){    // mat.len() gets number of rows
    mat.get( i, v.begin(), v.end()); // Reads row i into v
    for(auto&& x : v) std::cout << x << ' '; // Print v
    std::cout << '\n';
}
```


### Refocusing

When creating a new `Setting`, a new Lua thread is spawned with a table at the top of its virtual stack. The lifetime of this thread is determined by the lifetime of the `Setting`. To avoid the performance penalty of repeatedly building and destroying new threads, it is possible to reuse a `Setting` by 'refocusing'. Going back to our matrix example, an alternative way to read it may be:

```
auto mat = cfg.get<luaconfig::Setting>("matrix");
auto row = mat.get<luaconfig::Setting>(1);
for( int i=1; i<=mat.len(); ++i){
    if( i != 1 ) mat.refocus(row,i);
    for( int j=1; j<=row.len(); ++j){
        auto x = row.get<double>(j);
        std::cout << x << ' ';
    }
    std::cout << '\n';
}
```

In this case, it would be more efficient to use iterator methods, but refocusing will still work in cases where nested tables do not contain homogenous types (i.e. a mixture of numbers and strings).

## Licensing

This project is licensed under the MIT License -- see the [LICENSE.md](LICENSE.md) file for details. If you're using Luaconfig in your own work, there's no need to provide credit, though it would be highly appreciated.
