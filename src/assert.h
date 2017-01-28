#pragma once

#ifdef NDEBUG
#define assert(EX) 
#else
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
int _os_assert(const char*, const char*, const char*);
#define assert(EX) (void)( (EX) || _os_assert(#EX, __FILE__, TOSTRING(__LINE__)) )
#endif