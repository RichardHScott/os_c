#pragma once

#ifdef NDEBUG
#define assert(EX) 
#else
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define assert(EX) (void)( (EX) || _os_assert(#EX, __FILE__, TOSTRING(__LINE__)) )
#endif