#ifndef __ERRORS_HPP__
#define __ERRORS_HPP__

#include <ostd/Types.hpp>
#include <vector>

#ifdef __DEBUG__
   #define GLCall( x ) \
     ogfx::ErrorHandler::__gl_clearErrors(); \
     x; \
     if ( ogfx::ErrorHandler::__gl_printError( #x, __FILE__, __LINE__) ) __builtin_trap();
   #define GLCallRV( x ) [&]() { \
     ogfx::ErrorHandler::__gl_clearErrors(); \
     auto retVal = x; \
     if ( ogfx::ErrorHandler::__gl_printError( #x, __FILE__, __LINE__) ) __builtin_trap(); \
     return retVal; \
   }()
#else
   #define GLCall( x ) x
   #define GLCallRV( x ) x
#endif

namespace ogfx
{
    using namespace ostd; //TODO: Remove
    typedef String(*fpCustomMapErrorCallback)(int32_t);

    class ErrorHandler
    {
        public:
            static String mapError(int32_t err_code);
            static void pushError(int32_t err_code);
            static String getLastErrorString(void);
            inline static void setCustomMapErrorCallback(fpCustomMapErrorCallback callback) { s_customMapErrorCallback = callback; }

            static void __gl_clearErrors(void);
            static bool __gl_printError(const char* func_call, const char* file_path, int32_t line_number);

        private:
            inline static fpCustomMapErrorCallback s_customMapErrorCallback { nullptr };
            inline static std::vector<int32_t> s_errorStack;
    };
}

#endif
