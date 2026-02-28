#pragma once

#if !defined(__debugbreak)
   #if defined(_MSC_VER)
      #include <intrin.h>
   #else
      #include <csignal>
      #define __debugbreak() raise(SIGTRAP)
   #endif
#endif
