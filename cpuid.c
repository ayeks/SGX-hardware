///////////////////////////////////////////////////////////////////////////////
//  cpuid.c - 2023
//
/// This module contains multi-platform, non-privlidged code that utilizes the
/// CPUID instruction to discover & report SGX capabilities.
///
/// @file   cpuid.c
/// @author Lars Luhr   <mail@ayeks.de>
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>     // For printf()
#include <stdlib.h>    // For exit() EXIT_FAILURE
#include <string.h>    // For memset() strcmp()
#include <inttypes.h>  // For PRIx64 uint64_t PRIx32 uint32_t
#include <ctype.h>     // For isprint()
#include <stdbool.h>   // For true & false

#include "cpuid.h"     // Obviously...


/// Call `CPUID`, passing `eax`, `ebx`, `ecx` and `eax` in & out.
void native_cpuid32( uint32_t* eax
                    ,uint32_t* ebx
                    ,uint32_t* ecx
                    ,uint32_t* edx ) {
#if !defined(_MSC_VER)
   __asm volatile (
       "mov eax, %0;"
       "mov ebx, %1;"
       "mov ecx, %2;"
       "mov edx, %3;"
       "cpuid;"
       "mov %0, eax;"
       "mov %1, ebx;"
       "mov %2, ecx;"
       "mov %3, edx;"

      :"=rm" (*eax)    // Output
      ,"=rm" (*ebx)
      ,"=rm" (*ecx)
      ,"=rm" (*edx)
      :"0"   (*eax)    // Input
      ,"1"   (*ebx)
      ,"2"   (*ecx)
      ,"3"   (*edx)
      : "rax", "rbx", "rcx", "rdx" );  // Clobbers

#else
   // Visual Studio (still!) doesn't support inline Assembly Language, so we
   // have to depend on the `__cpuidex` intrinsic.
   int registers[4] = {0,0,0,0};

   __cpuidex( registers, *eax, *ecx );
   *eax = registers[0];
   *ebx = registers[1];
   *ecx = registers[2];
   *edx = registers[3];

#endif
}
