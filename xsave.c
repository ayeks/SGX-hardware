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

void print_XCR0_state_components( uint64_t xcr0, uint64_t xss ) {

   printf( "    XCR0     x87 state: %" PRId64 "\n", (xcr0 >> 0) & 1 );
   printf( "    XCR0     SSE state: %" PRId64 "\n", (xcr0 >> 1) & 1 );
   printf( "    XCR0     AVX state: %" PRId64 "\n", (xcr0 >> 2) & 1 );
   printf( "    XCR0     MPX state: %" PRId64 "%" PRId64 "\n", (xcr0 >> 4) & 1, (xcr0 >> 3) & 1 );
   printf( "    XCR0     AVX-512 state: %" PRId64 "%" PRId64 "%" PRId64 "\n", (xcr0 >> 7) & 1, (xcr0 >> 6) & 1, (xcr0 >> 5) & 1 );
   printf( "    IA32_XSS processor state trace: %" PRId64 "\n", (xss >> 8) & 1 );
   printf( "    XCR0     PKRU (User Protection Keys) state: %" PRId64 "\n", (xcr0 >> 9) & 1 );

   printf( "    IA32_XSS PASID (Process Address Space ID) state: %" PRId64 "\n", (xss >> 10) & 1 );
   printf( "    IA32_XSS CET_U state (Control-flow Enforcement Technology: user-mode functionality MSRs): %" PRId64 "\n", (xss >> 11) & 1 );
   printf( "    IA32_XSS CET_S state (CET: shadow stack pointers for rings 0,1,2): %" PRId64 "\n", (xss >> 12) & 1 );
   printf( "    IA32_XSS HDC (Hardware Duty Cycling) state: %" PRId64 "\n", (xss >> 13) & 1 );
   printf( "    IA32_XSS UINTR (User-Mode Interrupts) state: %" PRId64 "\n", (xss >> 14) & 1 );
   printf( "    IA32_XSS LBR (Last Branch Record) state: %" PRId64 "\n", (xss >> 15) & 1 );
   printf( "    IA32_XSS HWP (Hardware P-state control) state: %" PRId64 "\n", (xss >> 16) & 1 );

   printf( "    XCR0     AMX state: %" PRId64 "%" PRId64 "\n", (xcr0 >> 18) & 1, (xcr0 >> 17) & 1 );
   printf( "    XCR0     APX extended general purpose registers R16-R31 state: %" PRId64 "\n", (xcr0 >> 19) & 1 );
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
         printf( "  Raw IA32_XSS: %016" PRIx64 "\n", ia32_xss );
      } else {
         printf( "  IA32_XSS not readable\n" );
      }
   }

   printf("  Maximum size (in bytes) of current XCR0 XSAVE area: %" PRId32 "\n", ebx_0 );
   printf("  Maximum size (in bytes) of all-set XCR0 XSAVE area: %" PRId32 "\n", ecx_0 );
   printf("  Size (in bytes) of current XCR0+IA32_XSS XSAVE area: %" PRId32 "\n", ebx_1 );

   printf("  Supported XCR0: %08" PRIx32 "%08" PRIx32 "\n", edx_0, eax_0 );
   printf("  Actual    XCR0: %016" PRIx64 "\n", xcr0 );

   printf("  Supported IA32_XSS: %08" PRIx32 "%08" PRIx32 "\n", edx_1, ecx_1 );
   printf("  Actual    IA32_XSS: %016" PRIx64 "\n", ia32_xss );

   print_XCR0_state_components( (uint64_t)edx_0 << 32 | eax_0, (uint64_t)edx_1 << 32 | ecx_1 );
   /// @todo Need to get into IA32_XSS flags and print the system state components

   printf("  Supported XSAVE feature flags: %08" PRIx32 "\n", eax_1 );
   print_XSAVE_feature_flags( eax_1 );

}
