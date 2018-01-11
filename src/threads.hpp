// threads.hpp
//
// Functions for managing threads for luaconfig.
// Luaconfig does not use mutliple threads for the purpose of multi-tasking.
// Instead, they are used to generate new Lua States with separate execution stacks.

#ifndef __LUACONFIG_THREADS_HPP
#define __LUACONFIG_THREADS_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "exceptions.hpp"
#include <utility>

namespace luaconfig {

// Name of thread pool
const char* thread_pool = "luaconfigthreadpool";

// create new thread, place in registry, return pointer to lua_State and id
std::pair<lua_State*,int> new_thread( lua_State* L){
    // Side notes follow stack. R=Registry, T=ThreadTable, k=key, l=len, t=thread
    // Lookup thread table
    lua_pushstring(L,thread_pool);                                                 // +1, [k], add key to stack
    lua_gettable(L,LUA_REGISTRYINDEX);                                             // +1, [T], pop string, push T = R[k]
    // If not found, create
    if( !lua_istable(L,-1) ){
        lua_pop(L,1);                                                              // +0, [],  pop whatever was found
        lua_pushstring(L,thread_pool);                                             // +1, [k], push key
        lua_newtable(L);                                                           // +2, [k,T], push new table
        lua_settable(L,LUA_REGISTRYINDEX);                                         // +0, [], set R[k] = T, pop k and T
        lua_pushstring(L,thread_pool);                                             // +1, [k], add key to stack
        lua_gettable(L,LUA_REGISTRYINDEX);                                         // +1, [T], pop string, push T = R[k]
    }
    // With thread table on top of stack, find length
    lua_len(L,-1);                                                                 // +2, [T,l], push len(T)
    // Add one to get thread pool index
    lua_pushnumber(L,1);                                                           // +3, [T,l,1], push 1
    lua_arith(L,LUA_OPADD);                                                        // +2, [T,l+1], add last two on stack
    int id = lua_tonumber(L,-1);
    // Create new thread, create pointer
    lua_newthread(L);                                                              // +3, [T,l+1,t], new thread
    lua_State* p_thread = lua_tothread(L,-1);                                      // +3, [T,l+1,t]
    // Add thread to thread pool
    lua_settable(L,-3);                                                            // +1, [T], set T[l+1] = t
    // Clean up
    lua_pop(L,1);                                                                  // +0, [], pop thread table
    // Return pointer to new thread and associated id
    return std::make_pair(p_thread,id);
}

// kill a thread by setting its value to nil
void kill_thread( lua_State* L, int thread_id){
    // assume thread pool already exists
    lua_pushstring(L,thread_pool);
    lua_gettable(L,LUA_REGISTRYINDEX);
    lua_pushnumber(L,thread_id);
    lua_pushnil(L);
    lua_settable(L,-3);
}


} //end namespace
#endif
