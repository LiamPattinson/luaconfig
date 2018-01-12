// utils.hpp
//
// Collection of reusable code bits for luaconfig

#ifndef __LUACONFIG_UTILS_HPP
#define __LUACONFIG_UTILS_HPP

#include "core.hpp"

namespace luaconfig {

// Look up something on State1, use it to overwrite top of State2 stack
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


} // end namespace
#endif
