// core.hpp
//
// Collection of basic lua stack manipulation functions for use throughout luaconfig.
// Highly recommended that users do not use these functions directly. Use Configs and Settings instead.

#ifndef __LUACONFIG_CORE_HPP
#define __LUACONFIG_CORE_HPP

#include "exceptions.hpp"
#include "threads.hpp" // also includes Lua libraries
#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <tuple> // std::tie
#include <functional>

namespace luaconfig {

// Predeclare main classes

class Config;
class Setting;
class FunctionBase; // Use std::base_of to test for Function
template<class T> class Function;

// Scoping policy classes

class Global {};
class Table {};

// ============================================================================
// Get Lua variable to top of stack
// Returns number of stack objects at end of lookup

// Simple lookup of single item: no dot notation

// global
// Key can only be const char *. Compiler should complain about anything else.
template< class Scope, class Key>
auto lua_to_stack_single( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Global>::value, int>::type
{
    lua_getglobal(L,key);
    return 1;
} 

// table
// Key can be either const char* or an integer
template< class Scope, class Key>
auto lua_to_stack_single( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Table>::value && std::is_same<Key,const char*>::value, int>::type
{
    lua_pushstring(L,key);
    lua_gettable(L,-2);
    return 1;
} 

template< class Scope, class Key>
auto lua_to_stack_single( lua_State* L, Key key)
    -> typename std::enable_if< std::is_same<Scope,Table>::value && std::is_integral<Key>::value, int>::type
{
    lua_geti(L,-1,key);
    return 1;
} 

// Dot-notation lookup

static const char* dot_delim = "."; // delimiter string: any compatible chars go here
static const char* nums = "0123456789";

template< class Scope, class Key>
auto lua_to_stack( lua_State* L, Key key)
    -> typename std::enable_if< !std::is_integral<Key>::value, int>::type
{
    // Need non-const char* for strtok
    char* keydup = strdup(key);
    // Get first token
    char* tk = strtok(keydup,dot_delim);
    const char* tkc = tk;
    // Perform lookup
    int n_stack = lua_to_stack_single<Scope>(L,tkc);
    // Other tokens? Note that all further lookups must occur at table scope.
    while( (tk = strtok(nullptr,dot_delim)) != nullptr ){
        const char* tkc = tk;
        // index lookup, or text key?
        char first = tkc[0];
        if( strchr(nums,first) != nullptr ){ // is the first character numeric?
            int index = atoi(tkc);
            n_stack += lua_to_stack_single<Table>(L,index);
        } else {
            n_stack += lua_to_stack_single<Table>(L,tkc);
        }
    }
    return n_stack;
} 

template< class Scope, class Key>
auto lua_to_stack( lua_State* L, Key key)
    -> typename std::enable_if< std::is_integral<Key>::value && std::is_same<Scope,Table>::value, int>::type
{
    return lua_to_stack_single<Scope>(L,key);
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
// Get variable from stack to C++, pop from stack
// Note that this does not pop the value off the stack!

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

// table (Setting) or function (Function)
template<class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< std::is_same<T,Setting>::value || std::is_base_of<FunctionBase,T>::value, T>::type
{
    // Create new thread
    lua_State* p_thread;
    int thread_id;
    std::tie(p_thread,thread_id) = new_thread(L);
    // Move table from top of stack to new thread
    lua_xmove(L,p_thread,1);
    // Build and return new Setting    
    return T(p_thread,thread_id);
}

// function (std::function)
template<class T>
auto stack_to_cpp( lua_State* L)
    -> typename std::enable_if< is_function<T>::value, T>::type
{
    using sig = typename is_function<T>::sig;
    T func( std::move(stack_to_cpp<luaconfig::Function<sig>>(L)));
    return func;
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

// function
template<class T>
auto is_type( lua_State* L)
    -> typename std::enable_if< std::is_base_of<FunctionBase,T>::value || std::is_function<T>::value, bool>::type
{
    return lua_isfunction(L,-1);
}

// nil
inline bool is_nil( lua_State* L){
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

// function
template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< std::is_base_of<FunctionBase,T>::value, void>::type
{
   if( !lua_isfunction(L,-1)) throw TypeMismatchException(key,"function (as luaconfig Function)",luaL_typename(L,-1));
}

template< class T, class K>
auto type_test( lua_State* L, K key)
    -> typename std::enable_if< is_function<T>::value, void>::type
{
   if( !lua_isfunction(L,-1)) throw TypeMismatchException(key,"function (as std::function)",luaL_typename(L,-1));
}

// ============================================================================
// Test existance of a variable

template< class Scope, class K>
bool exists( lua_State* L, K key){
    int stack_size = lua_to_stack<Scope>(L,key);
    bool result = !is_nil(L);
    lua_pop(L,stack_size);
    return result;
}

// ============================================================================
// Get from Lua to stack, type check, get from stack to C++

// throwing version
template< class T, class Scope, class K>
T read( lua_State* L, K key){
    int stack_size = lua_to_stack<Scope>(L,key);
    type_test<T>(L,key);
    T result = stack_to_cpp<T>(L);
    lua_pop(L,stack_size-1);
    return result;
}

// non-throwing version with default
template< class T, class Scope, class K>
T read( lua_State* L, K key, T def){ 
    int stack_size = lua_to_stack<Scope>(L,key);
    T result;
    if ( is_type<T>(L) ){
        result = stack_to_cpp<T>(L);
        lua_pop(L,stack_size-1);
    } else {
        result = def;
        lua_pop(L,stack_size);
    }
    return result;
}

// iterable version
template<class itype, class Scope, class K,
         class rtype = decltype(*std::declval<itype>()) >
inline void read( lua_State* L, K key, itype it, itype end)
{
    int stack_size = lua_to_stack<Scope>(L,key);
    type_test<Setting>(L,key); // There should be a Lua table on the stack
    for( int idx=1; it != end; ++idx, ++it){
        lua_geti(L,-1,idx);
        type_test<typename std::remove_reference<rtype>::type>(L,idx);
        *it = stack_to_cpp<typename std::remove_reference<rtype>::type>(L);
    }
    lua_pop(L,stack_size);
}

// ===========================================================================
// Get length of named object

template<class Scope, class K>
std::size_t len( lua_State* L, K key){
    int stack_size = lua_to_stack<Scope>(L,key);
    lua_len(L,-1);
    auto size = stack_to_cpp<std::size_t>(L);
    lua_pop(L,stack_size);
    return size;
}

// ============================================================================
// Get from C++ to stack, type check, get from C++ to Lua

template< class Scope, class K, class T>
void write( lua_State* L, K key, T t){
    cpp_to_stack( L, t);
    type_test<T>( L, key);
    stack_to_lua<Scope>( L, key);
}

// ============================================================================
// Look up something on State_1, use it to overwrite top of State_2 stack
// Allows reuse of Setting and Function objects without rebuilding a Lua State

template<class T, class Scope, class K>
void refocus( lua_State* from, lua_State* to, K key){
        // drop top of 'to'
        lua_pop(to,1);
        // get new T
        lua_to_stack<Scope>(from,key);
        type_test<T>(from,key);
        // transfer
        lua_xmove( from, to, 1);
}

// ============================================================================
// Populate a tuple from the Lua stack

template<class...T>
struct TupleFromStackImpl;


template< class Head, class... Tail>
struct TupleFromStackImpl<Head,Tail...>{
    static std::tuple<Head,Tail...> get( lua_State* L){
        auto result = stack_to_cpp<Head>(L);
        return std::tuple_cat( TupleFromStackImpl<Tail...>::get(L), std::make_tuple(result));
    }
};

template<class Tail>
struct TupleFromStackImpl<Tail>{
    static std::tuple<Tail> get(lua_State* L){
        auto result = stack_to_cpp<Tail>(L);
        return std::make_tuple(result);
    }
};

template<class...T>
std::tuple<T...> tuple_from_stack( lua_State* L){
    return TupleFromStackImpl<T...>::get(L);
}


} // end namespace
#endif
