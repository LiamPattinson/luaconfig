// Exception.hpp
//
// Defines collection of exceptions used by luaconfig

#ifndef __LUACONFIG_EXCEPTION_HPP
#define __LUACONFIG_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace luaconfig {

// File exception
// Thrown due to missing file, parse error, etc.
// Lua itself will return quite useful error messages, so this simply forwards them to what().
class FileException : public std::runtime_error
{
    public:
    FileException( const char* msg) : std::runtime_error(msg) {}
};

// Type exception
// If a user anticipates type X, but instead finds type Y, this error is thrown and tries to return
// as much information as possible about what went wrong.
// TODO also return Setting path, as "GLOBAL.table.some_other_table.[5].my_setting"
class TypeMismatchException : public std::runtime_error
{
    public:

    TypeMismatchException( const char* key, const char* requested, const char* actual) : std::runtime_error(
        std::string{"Lookup for variable name \""} + std::string{key} + std::string{"\""}
        +std::string{" expected type \""} + std::string{requested} + std::string{"\""}
        +std::string{" but found type \""} + std::string{actual} + std::string{"\""}
    ) {}

    TypeMismatchException( int index, const char* requested, const char* actual) : std::runtime_error(
        std::string{"Lookup for index \""} + std::to_string(index) + std::string{"\""}
        +std::string{" expected type \""} + std::string{requested} + std::string{"\""}
        +std::string{" but found type \""} + std::string{actual} + std::string{"\""}
    ) {}
};

} // namespace end
#endif
