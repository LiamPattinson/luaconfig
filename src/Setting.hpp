// Setting.hpp
//
// A Setting object encapsulates a lua_State* with a permanent table on its stack.
// It offers similar methods to Config, such as get/set, though these act within a table
// rather than at global scope. It additionally allows integer-indexing for get/set.
//
// It is implemented using Lua threads; on creation, a new thread is created and the desired table
// is moved onto it. This thread resides at global scope in Lua. The lifetime of the thread is determined
// by the lifetime of the Setting; once the Setting moves out of scope, its corresponding Lua thread is
// set to the value 'nil'.
// 
// It is highly recommended that the user does not call a Setting's constructor directly. Instead, it is
// recommended to create them using Config::get or Setting::get.

#ifndef __LUACONFIG_SETTING_HPP
#define __LUACONFIG_SETTING_HPP

#include "core.hpp"

namespace luaconfig {

class Setting
{
    private:

    lua_State* _L;
    int _thread_id;

    using Scope = Table;

    public:

    // ====================================================
    // Constructor and Destructor

    Setting( lua_State* p_thread, int thread_id) : _L(p_thread), _thread_id(thread_id) {}

    ~Setting(){
        if( _L != nullptr ){
            // Remove table from stack
            lua_pop(_L,1);
            // Delete thread
            kill_thread(_L,_thread_id);
        }
    }

    // ====================================================
    // Copy constructor, assignment operator
    // Both are deleted, as a Setting object has unique control over the lifetime of a lua_State*.

    Setting( const Setting& ) = delete;
    Setting& operator=( const Setting& ) = delete;

    // ====================================================
    // Move constructor / move assignment
    // Both will invalidate the original Setting object.

    Setting( Setting&& other) :
        _L(other._L),
        _thread_id(other._thread_id)
    {
        other._L = nullptr;
    }

    Setting& operator=( Setting&& other){
        _L = other._L;
        _thread_id = other._thread_id;
        other._L = nullptr;
        return *this;
    }

    // ====================================================
    // Lookup and return Lua variable
    
    template<class T>
    T get( const char* key){
        return read<T,Scope>(_L,key);
    }

    template<class T>
    T get( const std::string& key){
        return get<T>(key.c_str());
    }

    template<class T>
    T get( int key){
        return read<T,Scope>(_L,key);
    }

    // ====================================================
    // Set a new Lua variable 

    template<class T>
    void set( const char* key, T value){
        write<Scope>( _L, key, value);
    }

    template<class T>
    void set( const std::string& key, T value){
        set( key.c_str(), value);
    }

    template<class T>
    void set( int key, T value){
        write<Scope>( _L, key, value);
    }

    // ====================================================
    // Get length of table
    // Reminder: Lua indexing goes from 1 to len, not 0 to len-1!
    
    int len(){
        lua_len(_L,-1);
        return stack_to_cpp<int>(_L);
    }

};

} // end namespace
#endif
