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
#include "utils.hpp"

namespace luaconfig {

class Setting
{
    friend Config;

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

    Setting( const Setting& other){
        std::tie(_L,_thread_id) = copy_thread(other._L);
    }

    Setting& operator=( const Setting& other){
        // Delete current thread
        if( _L != nullptr ){
            // Remove table from stack
            lua_pop(_L,1);
            // Delete thread
            kill_thread(_L,_thread_id);
        }
        // Copy
        std::tie(_L,_thread_id) = copy_thread(other._L);
    }

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
    
    // throwing version
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

    // non-throwing version with default
    template<class T>
    T get( const char* key, T def){
        return read<T,Scope>(_L,key,def);
    }

    template<class T>
    T get( const std::string& key, T def){
        return get<T>(key.c_str(),def);
    }

    template<class T>
    T get( int key, T def){
        return read<T,Scope>(_L,key,def);
    }

    // ====================================================
    // Write to iterable

    // iterable version
    template< class itype>
    void get( const char* key, itype it, itype end){
        read<itype,Scope>(_L,key,it,end);
    }

    template< class itype>
    void get( const std::string& key, itype it, itype end){
        get(key.c_str(),it,end);
    }

    template< class itype>
    void get( int key, itype it, itype end){
        read<itype,Scope>(_L,key,it,end);
    }

    // ====================================================
    // Test existance of Lua variable

    bool exists( const char* key){
        return luaconfig::exists<Scope>(_L,key);
    }

    bool exists( const std::string& key){
        return exists(key.c_str());
    }

    bool exists( int index){
        return luaconfig::exists<Scope>(_L,index);
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
    
    std::size_t len(){
        lua_len(_L,-1);
        return stack_to_cpp<std::size_t>(_L);
    }

    // ====================================================
    // Get length of object within table
    // Reminder: Lua indexing goes from 1 to len, not 0 to len-1!

    std::size_t len( const char* key){
        return luaconfig::len<Scope>(_L,key);
    }

    std::size_t len( const std::string& key){
        return len(key.c_str());
    }

    std::size_t len( int key){
        return luaconfig::len<Scope>(_L,key);
    }

    // ====================================================
    // Lookup table and use to reconfigure an existing Setting
    // This allows the reuse of a sub-Setting without having
    // to rebuild a Lua State each time.

    void refocus( Setting& other, const char* key){
        luaconfig::refocus<Setting,Scope>( _L, other._L, key);
    }

    void refocus( Setting& other, const std::string& key){
        luaconfig::refocus<Setting,Scope>( _L, other._L, key.c_str());
    }

    void refocus( Setting& other, int index){
        luaconfig::refocus<Setting,Scope>( _L, other._L, index);
    }

};

} // end namespace
#endif
