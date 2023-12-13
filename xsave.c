///////////////////////////////////////////////////////////////////////////////
//  xsave.c - 2023
//
/// This module documents XSAVE related enumeration.
///
/// SGX interacts with XSAVE (for those processors that support it) and so
/// it's important to understand what the current CPU's XSAVE capabilities are
/// so you can correctly configure them in SGX.  For example, SECS.ATTRIBUTES.XFRM
/// represents the XSAFE Feature Request Mask.
///
/// ## What do SGX programmers need to know about XSAVE?
///
/// First, XSAVE uses the XCR0 register.  XCR0 has nothing to do with CR0, so
/// don't get confused.  They have similar names, but they are not related.
///
/// @file   xsave.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>     // For printf()
#include <inttypes.h>  // For PRIx32
#include <stdbool.h>   // For bool
#include <string.h>    // For strlen()


#include "xsave.h"  // For obvious reasons
#include "cpuid.h"  // For native_cpuid32()
#include "rdmsr.h"  // For checkCapabilities()


bool is_XGETBV_supported = 0;


/// Call `native_XGETBV`, passing `ecx`.
///
/// @param xcr Extended control register (XCR) specified in the ECX register
uint64_t native_XGETBV( uint32_t xcr ) {

   uint32_t edx;
   uint32_t eax;

#if !defined( _MSC_VER )
   __asm volatile (
       "mov ecx, %[xcr];"
       "xgetbv;"
       "mov %[edx], edx;"
       "mov %[eax], eax;"
      :[edx] "=rm" (edx)    // Output
      ,[eax] "=rm" (eax)
      :[xcr] ""    (xcr)    // Input
      : );  // Clobbers

   // printf( "edx=%d  eax=%d  xcr=%d\n", edx, eax, xcr);
   return (uint64_t) edx<<32 | eax;

#else
   // Visual Studio (still!) doesn't support inline Assembly Language, so we
   // have to depend on the `_xgetbv` intrinsic.
   return _xgetbv( xcr );  // This is almost certianly broken right now
#endif
}


void print_detailed_state_component( char* type, char* name, int bit, uint64_t flag_supported, uint64_t flag_acutal, char* desc ) {
   printf( "    %-8s %-10s", type, name );

   if( flag_supported >> bit & 1 ) {
      printf( " yes    " );
   } else {
      printf( "  no    " );
   }

   if( flag_acutal >> bit & 1 ) {
      printf( "  set" );
   } else {
      printf( "clear" );
   }

   if( strlen( desc ) > 0 ) {
      printf( " %s", desc );
   }

   printf( "\n" );
}


void print_XSAVE_state_components( uint64_t xcr0, uint64_t xss, uint64_t xcr0_actual, uint64_t xss_actual  ) {

   printf( "    Register Name    Supported Value Description\n" );
   printf( "    ======== ======= ========= ===== ===========\n" );

/// @see https://en.wikipedia.org/wiki/Control_register

   print_detailed_state_component( "XCR0",     "x87:",       0, xcr0, xcr0_actual, "x87 Floating Point Unit & MMX" );
   print_detailed_state_component( "XCR0",     "SSE:",       1, xcr0, xcr0_actual, "MXCSR and XMM registers" );
   print_detailed_state_component( "XCR0",     "AVX:",       2, xcr0, xcr0_actual, "YMM registers" );
   print_detailed_state_component( "XCR0",     "BNDREG:",    3, xcr0, xcr0_actual, "MPX for BND registers" );
   print_detailed_state_component( "XCR0",     "BNDCSR:",    4, xcr0, xcr0_actual, "MPX for BNDCFGU and BNDSTATUS registers" );
   print_detailed_state_component( "XCR0",     "opmask:",    5, xcr0, xcr0_actual, "AVX-512 for AVX opmask and AKA k-mask" );
   print_detailed_state_component( "XCR0",     "ZMM_hi256:", 6, xcr0, xcr0_actual, "AVX-512 for the upper-halves of lower ZMM registers" );
   print_detailed_state_component( "XCR0",     "Hi16_ZMM:",  7, xcr0, xcr0_actual, "AVX-512 for the upper ZMM registers" );
   print_detailed_state_component( "IA32_XSS", "PT:",        8, xss,  xss_actual,  "Processor Trace" );
   print_detailed_state_component( "XCR0",     "PKRU:",      9, xcr0, xcr0_actual, "User Protection Keys" );
   print_detailed_state_component( "IA32_XSS", "PASID:",    10, xss,  xss_actual,  "Process Address Space ID" );
   print_detailed_state_component( "IA32_XSS", "CET_U:",    11, xss,  xss_actual,  "Control-flow Enforcement Technology: user-mode functionality MSRs" );
   print_detailed_state_component( "IA32_XSS", "CET_S:",    10, xss,  xss_actual,  "CET: shadow stack pointers for rings 0,1,2" );
   print_detailed_state_component( "IA32_XSS", "HDC:",      13, xss,  xss_actual,  "Hardware Duty Cycling" );
   print_detailed_state_component( "IA32_XSS", "UINTR:",    14, xss,  xss_actual,  "User-Mode Interrupts" );
   print_detailed_state_component( "IA32_XSS", "LBR:",      15, xss,  xss_actual,  "Last Branch Record" );
   print_detailed_state_component( "IA32_XSS", "HWP:",      16, xss,  xss_actual,  "Hardware P-state control" );
   print_detailed_state_component( "XCR0",     "TILECFG:",  17, xcr0, xcr0_actual, "AMX - Advanced Matrix Extensions" );
   print_detailed_state_component( "XCR0",     "TILEDATA:", 18, xcr0, xcr0_actual, "AMX - Advanced Matrix Extensions" );
   print_detailed_state_component( "XCR0",     "APX:",      19, xcr0, xcr0_actual, "Extended General Purpose Registers R16-R31" );
}


void print_XSAVE_feature_flags( uint32_t eax ) {

   printf( "    xsaveopt - save state-components that have been modified since last XRSTOR: %" PRId32 "\n", (eax >> 0) & 1 );
   printf( "    xsavec - save/restore state with compaction: %" PRId32 "\n", (eax >> 1) & 1 );
   printf( "    xgetbv_ecx1 - XGETBV with ECX=1 support: %" PRId32 "\n", (eax >> 2) & 1 );
   printf( "    xss - save/restore state with compaction, including supervisor state: %" PRId32 "\n", (eax >> 3) & 1 );
   printf( "    xfd - Extended Feature Disable supported: %" PRId32 "\n", (eax >> 4) & 1 );
}


void print_XSAVE_enumeration() {
   printf( "XSAVE features and state-components\n" );

   uint32_t eax_0 = 0x0D;  // Check XSAVE features and state-components
   uint32_t ebx_0 = 0;
   uint32_t ecx_0 = 0;     // Get basic XSAVE information
   uint32_t edx_0 = 0;

   uint32_t eax_1 = 0x0D;  // Check XSAVE features and state-components
   uint32_t ebx_1 = 0;
   uint32_t ecx_1 = 1;     // Get XSAVE extended features
   uint32_t edx_1 = 0;

   uint64_t xcr0 = 0;      // The actual value
   uint64_t ia32_xss = 0;  // The actual value

   native_cpuid32( &eax_0, &ebx_0, &ecx_0, &edx_0 );
   // print_registers32( eax_0, ebx_0, ecx_0, edx_0 );

   native_cpuid32( &eax_1, &ebx_1, &ecx_1, &edx_1 );
   // print_registers32( eax_1, ebx_1, ecx_1, edx_1 );

   is_XGETBV_supported = (eax_1 >> 2) & 1;

   if( is_XGETBV_supported ) {
      xcr0 = native_XGETBV( 0 );  // Get xcr0
   }

   if( checkCapabilities() ) {
      if( rdmsr( IA32_XSS, 0, &ia32_xss ) ) {
         // printf( "  Raw IA32_XSS: %016" PRIx64 "\n", ia32_xss );
      } else {
         printf( "  IA32_XSS not readable\n" );
      }
   }

   printf("  Maximum size (in bytes) of current XCR0 XSAVE area: %" PRId32 "\n", ebx_0 );
   printf("  Maximum size (in bytes) of all-set XCR0 XSAVE area: %" PRId32 "\n", ecx_0 );
   printf("  Size (in bytes) of current XCR0+IA32_XSS XSAVE area: %" PRId32 "\n", ebx_1 );

   printf("  Supported XCR0:     %08" PRIx32 "%08" PRIx32 "\n", edx_0, eax_0 );
   printf("  Actual    XCR0:     %016" PRIx64 "\n", xcr0 );

   printf("  Supported IA32_XSS: %08" PRIx32 "%08" PRIx32 "\n", edx_1, ecx_1 );
   printf("  Actual    IA32_XSS: %016" PRIx64 "\n", ia32_xss );

   print_XSAVE_state_components( (uint64_t)edx_0 << 32 | eax_0, (uint64_t)edx_1 << 32 | ecx_1, xcr0, ia32_xss );
   /// @todo Need to get into IA32_XSS flags and print the system state components

   printf("  Supported XSAVE feature flags: %08" PRIx32 "\n", eax_1 );
   print_XSAVE_feature_flags( eax_1 );

}
