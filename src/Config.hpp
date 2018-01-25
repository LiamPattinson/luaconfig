// Config.hpp
//
// A Config encapsulates a lua_State* and represents the global scope of a Lua configuration file.
// It handles file reading, manages the lifetime of a Lua State, and offers get/set methods that
// act on the global scope.

#ifndef __LUACONFIG_CONFIG_HPP
#define __LUACONFIG_CONFIG_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>
#include <type_traits>
#include <utility>
#include <tuple>

#include "core.hpp"
#include "utils.hpp"
#include "Setting.hpp"

namespace luaconfig {

class Config
{
    private:

    lua_State* _L;
    std::string _filename;

    using Scope = Global;

    public:

    // ====================================================
    // Constructor and Destructor

    Config( const char* filename ) :
        _L(luaL_newstate()),
        _filename(filename)
    {
        luaL_openlibs(_L);   
        if ( luaL_loadfile(_L,filename) || lua_pcall(_L, 0, 0, 0) ) throw FileException(lua_tostring(_L,-1));
    }

    Config( const std::string& filename) : Config(filename.c_str()) {}

    ~Config(){
        if ( _L != nullptr ) lua_close(_L);
    }

    // ====================================================
    // Copy constructor, assignment operator
    // Both are deleted, as a Config object has unique control over the lifetime of a lua_State*.

    Config( const Config& ) = delete;
    Config& operator=( const Config& ) = delete;

    // ====================================================
    // Move constructor / move assignment
    // Both will invalidate the original Config object

    Config( Config&& other) :
        _L(other._L),
        _filename(std::move(other._filename))
    {
        other._L = nullptr;
    }

    Config& operator=( Config&& other){
        _L = other._L;
        _filename = std::move(other._filename);
        other._L = nullptr;
        return *this;
    }

    // ====================================================
    // Lookup and return Lua variable

    // throwing version
    template< class T>
    T get( const char* key){
        return read<T,Scope>(_L,key);
    }

    template<class T>
    T get( const std::string& key){
        return get<T>(key.c_str());
    }

    // non-throwing version with default
    template< class T>
    T get( const char* key, T def){
        return read<T,Scope>(_L,key,def);
    }

    template< class T>
    T get( const std::string& key, T def){
        return get<T>(key.c_str(),def);
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

    // ====================================================
    // Test existance of Lua variable

    bool exists( const char* key){
        return luaconfig::exists<Scope>(_L,key);
    }

    bool exists( const std::string& key){
        return exists(key.c_str());
    }

    // ====================================================
    // Get size of Lua variable

    std::size_t len( const char* key){
        return luaconfig::len<Scope>(_L,key);
    }

    std::size_t len( const std::string& key){
        return len(key.c_str());
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

    // ====================================================
    // Lookup table and use to reconfigure an existing Setting
    // This allows the reuse of a sub-Setting without having
    // to rebuild a Lua State each time.
    //
    void refocus( Setting& other, const char* key){
        luaconfig::refocus<Setting,Scope>( _L, other._L, key);
    }

    void refocus( Setting& other, const std::string& key){
        luaconfig::refocus<Setting,Scope>( _L, other._L, key.c_str());
    }

};


} // namespace end
#endif
