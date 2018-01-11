// core.hpp
//
// Collection of basic lua stack manipulation functions for use throughout luaconfig.
// Highly recommended that users do not use these functions directly. Use Configs and Settings instead.

#ifndef __LUACONFIG_CORE_HPP
#define __LUACONFIG_CORE_HPP

#include "exceptions.hpp"
#include "threads.hpp" // also includes Lua libraries

#include <type_traits>
#include <tuple> // std::tie

namespace luaconfig {

// Predeclare main classes

class Config;
class Setting;

// Scoping policy classes

class Global {};
class Table {};

// ============================================================================
// Get Lua variable to top of stack

// global
// Key can only be const char *. Compiler should complain about anything else.
template< class Scope, class Key>
auto lua_to_stack( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Global>::value, void>::type
{
   lua_getglobal(L,key);
} 

// table
// Key can be either const char* or an integer
template< class Scope, class Key>
auto lua_to_stack( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Table>::value && std::is_same<Key,const char*>::value, void>::type
{
    lua_pushstring(L,key);
    lua_gettable(L,-2);
} 

template< class Scope, class Key>
auto lua_to_stack( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Table>::value && std::is_integral<Key>::value, void>::type
{
    lua_pushnumber(L,key);
    lua_gettable(L,-2);
} 

// ============================================================================
// Get stack variable to Lua

// global
template<class Scope, class Key>
auto stack_to_lua( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Global>::value, void>::type
{
   lua_setglobal(L,key);
} 

// table
// Key can be either const char* or an integer
template< class Scope, class Key>
auto stack_to_lua( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Table>::value && std::is_same<Key,const char*>::value, void>::type
{
    lua_pushstring(L,key);
    lua_rotate(L,-2,1);
    lua_settable(L,-3);
} 

template< class Scope, class Key>
auto stack_to_lua( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Table>::value && std::is_integral<Key>::value, void>::type
{
    lua_pushnumber(L,key);
    lua_rotate(L,-2,1);
    lua_settable(L,-3);
} 

// ============================================================================
// Get C++ variable to top of stack

// float
template< class T>
auto cpp_to_stack( lua_State* L, T value)
    -> typename std::enable_if< std::is_floating_point<T>::value, void>::type
{
    lua_pushnumber(L,value);
}

// integer
template< class T>
auto cpp_to_stack( lua_State* L, T value)
    -> typename std::enable_if< std::is_integral<T>::value && !std::is_same<T,bool>::value, void>::type
{
    lua_pushinteger(L,value);
}

// bool
template<class T>
auto cpp_to_stack( lua_State* L, T value)
    -> typename std::enable_if< std::is_same<T,bool>::value, void>::type
{
    lua_pushboolean(L,value);
}

// const char*
template<class T>
auto cpp_to_stack( lua_State* L, const T* value)
    -> typename std::enable_if< std::is_same<T,char>::value, void>::type
{
    lua_pushstring(L,value);
    
}

// std::string
template<class T>
auto cpp_to_stack( lua_State* L, const T& value)
    -> typename std::enable_if< std::is_same<T,std::string>::value, void>::type
{
    lua_pushstring(L,value.c_str());
}

// ============================================================================
// Get variable from stack to C++

// float
template< class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< std::is_floating_point<T>::value, T>::type
{
    lua_Number result = lua_tonumber(L,-1);
    lua_pop(L,1);
    return static_cast<T>(result);
}

// integer
template< class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< std::is_integral<T>::value && !std::is_same<T,bool>::value, T>::type
{
    lua_Integer result = lua_tointeger(L,-1);
    lua_pop(L,1);
    return static_cast<T>(result);
}

// boolean
template< class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< std::is_same<T,bool>::value, T>::type
{
    bool result = lua_toboolean(L,-1);
    lua_pop(L,1);
    return static_cast<T>(result);
}

// string
template<class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< std::is_same<T,std::string>::value, T>::type
{
    std::string result(lua_tostring(L,-1));
    lua_pop(L,1);
    return static_cast<T>(result);

}

// table (Setting)
template<class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< std::is_same<T,luaconfig::Setting>::value, T>::type
{
    // Create new thread
    lua_State* p_thread;
    int thread_id;
    std::tie(p_thread,thread_id) = new_thread(L);
    // Move table from top of stack to new thread
    lua_xmove(L,p_thread,1);
    // Build and return new Setting    
    return Setting(p_thread,thread_id);;

}

// ============================================================================
// Type checking on the stack

// Non-throwing -- return bool if type matches

// is_type<T>

// float
template<class T>
auto is_type( lua_State* L)
    -> typename std::enable_if< std::is_floating_point<T>::value, bool>::type
{
    return lua_isnumber(L,-1);
}

// int
template< class T>
auto is_type( lua_State* L)
    -> typename std::enable_if< std::is_integral<T>::value && !std::is_same<T,bool>::value, bool>::type
{
   return lua_isinteger(L,-1);
}

// boolean
template< class T>
auto is_type( lua_State* L)
    -> typename std::enable_if< std::is_same<T,bool>::value, bool>::type
{
   return lua_isboolean(L,-1);
}

template< class T>
auto is_type( lua_State* L)
    -> typename std::enable_if< std::is_same<T,std::string>::value || std::is_same<T,const char*>::value, bool>::type
{
    return lua_isstring(L,-1);
}

// table
template< class T>
auto is_type( lua_State* L)
    -> typename std::enable_if< std::is_same<T,Setting>::value, bool>::type
{
    return lua_istable(L,-1);
}

// nil
bool is_nil( lua_State* L){
    return lua_isnoneornil(L,-1);
}

// Throwing -- throw a custom exception if not matched

// float
template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< std::is_floating_point<T>::value, void>::type
{
   if( !lua_isnumber(L,-1)) throw TypeMismatchException(key,"number",luaL_typename(L,-1));
}

// int
template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< std::is_integral<T>::value && !std::is_same<T,bool>::value, void>::type
{
   if( !lua_isinteger(L,-1)) throw TypeMismatchException(key,"number (integer)",luaL_typename(L,-1));
}

// boolean
template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< std::is_same<T,bool>::value, void>::type
{
   if( !lua_isboolean(L,-1)) throw TypeMismatchException(key,"boolean",luaL_typename(L,-1));
}

// string
template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< std::is_same<T,std::string>::value || std::is_same<T,const char*>::value, void>::type
{
   if( !lua_isstring(L,-1)) throw TypeMismatchException(key,"string",luaL_typename(L,-1));
}

// table
template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< std::is_same<T,Setting>::value, void>::type
{
   if( !lua_istable(L,-1)) throw TypeMismatchException(key,"table (as luaconfig Setting)",luaL_typename(L,-1));
}

// ============================================================================
// Get from Lua to stack, type check, get from stack to C++

// throwing version
template< class T, class Scope, class K>
T read( lua_State* L, K key){
    lua_to_stack<Scope>(L,key);
    type_test<T>(L,key);
    return stack_to_cpp<T>(L);
}

// non-throwing version with default
template< class T, class Scope, class K>
T read( lua_State* L, K key, T def){ 
    lua_to_stack<Scope>(L,key);
    return (is_type<T>(L)) ? stack_to_cpp<T>(L) : def;
}

// ============================================================================
// Get from C++ to stack, type check, get from C++ to Lua

template< class Scope, class K, class T>
void write( lua_State* L, K key, T t){
    cpp_to_stack( L, t);
    type_test<T>( L, key);
    stack_to_lua<Scope>( L, key);
}

} // end namespace
#endif
