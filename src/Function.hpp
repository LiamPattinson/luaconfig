// Function.hpp
//
// Encapsulates a lua_State* with a permanent function on its stack.
//
// Implemented similarly to Function in terms of Lua threading.
//
// Templated over return type and arbitrary argument list.

#ifndef __LUACONFIG_FUNCTION_HPP
#define __LUACONFIG_FUNCTION_HPP

#include "core.hpp"

namespace luaconfig {

// FunctionBase definition
// Defines boring things like constructors etc.

class FunctionBase
{
    protected:

    lua_State* _L;
    int _thread_id;
    
    public:

    // ====================================================
    // Constructor and Destructor

    FunctionBase( lua_State* p_thread, int thread_id) : _L(p_thread), _thread_id(thread_id) {}

    ~FunctionBase(){
        if( _L != nullptr ){
            // Remove function from stack
            lua_pop(_L,1);
            // Delete thread
            kill_thread(_L,_thread_id);
        }
    }

    // ====================================================
    // Copy constructor, assignment operator
    // Both are deleted, as a Function object has unique control over the lifetime of a lua_State*.

    FunctionBase( const FunctionBase& other){
        std::tie(_L,_thread_id) = copy_thread(other._L);
    }

    FunctionBase& operator=( const FunctionBase& other){
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
    // Both will invalidate the original Function object.

    FunctionBase( FunctionBase&& other) :
        _L(other._L),
        _thread_id(other._thread_id)
    {
        other._L = nullptr;
    }

    FunctionBase& operator=( FunctionBase&& other){
        _L = other._L;
        _thread_id = other._thread_id;
        other._L = nullptr;
        return *this;
    }
};

// Function class definition

// General Declaration
template< class Func >
class Function; // undefined

// Single return type

template< class RType, class... Args>
class Function< RType(Args...) > : FunctionBase
{
    public:

    using FunctionBase::FunctionBase;

    // ====================================================
    // Call function

    RType operator() ( Args... args)
    {
        // Annoyingly, lua_call pops a function from the stack. Must work with a copy of function.
        lua_pushnil(_L);
        lua_copy(_L,-2,-1);
        // One by one, push args to stack
        auto push = {(cpp_to_stack(_L,args),0)...}; (void)push;
        // Execute Lua function
        lua_call(_L,sizeof...(Args),1);
        // Extract and return result
        return stack_to_cpp<RType>(_L);
    }
};

// Multiple return type

template< class... Args, class... RTypes>
class Function<std::tuple<RTypes...>(Args...)> : FunctionBase
{
    public:

    using FunctionBase::FunctionBase;

    // ====================================================
    // Call function

    std::tuple<RTypes...> operator() ( Args... args)
    {
        // Make copy of function.
        lua_pushnil(_L);
        lua_copy(_L,-2,-1);
        // One by one, push args to stack
        auto push = {(cpp_to_stack(_L,args),0)...}; (void)push;
        // Execute Lua function
        lua_call(_L,sizeof...(Args),sizeof...(RTypes));
        // Extract results and return
        return tuple_from_stack<RTypes...>(_L);;
    };
};

} // end namespace
#endif
