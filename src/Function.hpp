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

template< class RType, class... Args>
class Function : FunctionBase
{
    private:

    lua_State* _L;
    int _thread_id;
    
    static constexpr int n_args = sizeof...(Args); // Number of function arguments.

    public:

    // ====================================================
    // Constructor and Destructor

    Function( lua_State* p_thread, int thread_id) : _L(p_thread), _thread_id(thread_id) {}

    ~Function(){
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

    Function( const Function& ) = delete;
    Function& operator=( const Function& ) = delete;

    // ====================================================
    // Move constructor / move assignment
    // Both will invalidate the original Function object.

    Function( Function&& other) :
        _L(other._L),
        _thread_id(other._thread_id)
    {
        other._L = nullptr;
    }

    Function& operator=( Function&& other){
        _L = other._L;
        _thread_id = other._thread_id;
        other._L = nullptr;
        return *this;
    }

    // ====================================================
    // Call function

    RType operator() ( Args... args){
        // Annoyingly, lua_call pops a function from the stack. Must work with a copy of function.
        lua_pushnil(_L);
        lua_copy(_L,-2,-1);
        // One by one, push args to stack
        auto push = {(cpp_to_stack(_L,args),0)...}; (void)push;
        // Execute Lua function
        lua_call(_L,n_args,1);
        // Extract and return result
        return stack_to_cpp<RType>(_L);
    }
};

/* extension case:

template< class...RTypes, class... Args>
class Function< std::tuple<RTypes...>, Args> {

};

*/

} // end namespace
#endif
