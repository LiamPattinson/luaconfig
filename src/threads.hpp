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
#include <tuple>
#include <utility>

namespace luaconfig {

// Name of thread pool
static const char* thread_pool = "luaconfigthreadpool";

// create new thread, place in registry, return pointer to lua_State and id
inline std::pair<lua_State*,int> new_thread( lua_State* L){
    // Side notes follow stack. R=Registry, T=ThreadTable, k=key, t=thread
    // Lookup thread table
    lua_pushstring(L,thread_pool);                       // +1, [k], add key to stack
    lua_gettable(L,LUA_REGISTRYINDEX);                   // +1, [T], pop string, push T = R[k]
    // If not found, create
    if( !lua_istable(L,-1) ){
        lua_pop(L,1);                                    // +0, [],  pop whatever was found
        lua_pushstring(L,thread_pool);                   // +1, [k], push key
        lua_newtable(L);                                 // +2, [k,T], push new table
        lua_settable(L,LUA_REGISTRYINDEX);               // +0, [], set R[k] = T, pop k and T
        lua_pushstring(L,thread_pool);                   // +1, [k], add key to stack
        lua_gettable(L,LUA_REGISTRYINDEX);               // +1, [T], pop string, push T = R[k]
    }
    // With thread table on top of stack, find first nil
    int id=1;
    while(true){
        lua_pushnumber(L,id);                            // +2, [T,id], push id
        lua_gettable(L,-2);                              // +2, [T,v], with v = T[id]
        bool is_nil = lua_isnil(L,-1);                   // +2, [T,v]
        lua_pop(L,1);                                    // +1, [T], pop v
        if( is_nil ) break; else ++id;
    }
    lua_pushnumber(L,id);                                // +2, [T,id], push id
    // Create new thread, create pointer
    lua_newthread(L);                                    // +3, [T,id,t], new thread
    lua_State* p_thread = lua_tothread(L,-1);            // +3, [T,id,t]
    // Add thread to thread pool
    lua_settable(L,-3);                                  // +1, [T], set T[id] = t
    // Clean up
    lua_pop(L,1);                                        // +0, [], pop thread table
    // Return pointer to new thread and associated id
    return std::make_pair(p_thread,id);
}

// Create new thread with same stack
inline std::pair<lua_State*,int> copy_thread( lua_State* L){
    // Get new thread
    lua_State* p_new; int id;
    std::tie(p_new,id) = new_thread(L);
    // Copy things over
    int stack_size = lua_gettop(L);
    lua_pushnil(L);
    for( int i=1; i<=stack_size; ++i){
        lua_copy(L,i,-1);
        lua_xmove(L,p_new,1);
    }
    return std::make_pair(p_new,id);
}

// kill a thread by setting its value to nil
inline void kill_thread( lua_State* L, int thread_id){
    // assume thread pool already exists
    lua_pushstring(L,thread_pool);
    lua_gettable(L,LUA_REGISTRYINDEX);
    lua_pushnumber(L,thread_id);
    lua_pushnil(L);
    lua_settable(L,-3);
}


} //end namespace
#endif
