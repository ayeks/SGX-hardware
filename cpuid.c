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
#include <string.h>    // For memset() strcmp()
#include <inttypes.h>  // For PRIx64 uint64_t PRIx32 uint32_t
#include <ctype.h>     // For isprint()
#include <stdbool.h>   // For true & false

#include "cpuid.h"     // For obvious reasons
#include "test-sgx.h"  // For EXIT_ON_FAILURE
#include "rdmsr.h"     // For TBD


// This number is kinda arbitrary.  Let us know if you ever find a situation
// where we need to increase this.
#define NUMBER_OF_EPCs_TO_ENUMERATE 16


/// Call `CPUID`, passing `eax`, `ebx`, `ecx` and `eax` in & out.
static inline void native_cpuid32( uint32_t* eax
                                  ,uint32_t* ebx
                                  ,uint32_t* ecx
                                  ,uint32_t* edx ) {
#if !defined( _MSC_VER )
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


// Print the register set:
//     eax: 80000008  ebx: 00000000  ecx: 00000000  edx: 00000000
void print_registers32( uint32_t eax
                       ,uint32_t ebx
                       ,uint32_t ecx
                       ,uint32_t edx ) {

   printf("eax: %08" PRIx32 "  ", eax );
   printf("ebx: %08" PRIx32 "  ", ebx );
   printf("ecx: %08" PRIx32 "  ", ecx );
   printf("edx: %08" PRIx32 "\n", edx );

}


/// Does this CPU support the CPUID instruction?
///
/// gcc supports intrisics to detect certain CPU features:
///   https://gcc.gnu.org/onlinedocs/gcc-13.2.0/gcc/x86-Built-in-Functions.html
///   ... however, as of GCC 13.2, the detection does not support SGX, so
///   we'll do it old school.
///
void doesCPUIDwork( void ) {
   uint64_t rax = 0;

   /// @see https://wiki.osdev.org/CPUID#Checking_CPUID_availability

   #if !defined( _MSC_VER )

   __asm volatile (
      "pushfq;"         // Save RFLAGS
      "pushfq;"         // Save RFLAGS  (copy)
      "xor dword ptr [rsp],0x00200000;"  // Invert the ID bit in stored RFLAGS
      "popfq;"          // Load stored RFLAGS (with ID bit inverted)
      "pushfq;"         // Store RFLAGS again (ID bit may or may not be inverted)
      "pop rax;"        // rax = modified RFLAGS (ID bit may or may not be inverted)
      "xor rax, qword ptr[rsp];"  // rax = whichever bits were changed
      "popfq;"          // Restore original RFLAGS
      "and rax,0x00200000;"  // eax = zero if ID bit can't be changed, else non-zero
      "mov %0, rax;"
      :"=rm" (rax)      // Output
      :                 // Input
      : "cc", "rax" );  // Clobbers

   #else
      rax = 1;  // In Visual Studio, we have to assume that CPUID is available
   #endif
   // printf( "rax is: 0x%" PRIx64 "\n", rax );

   if( rax == 0 ) {
      printf( "CPUID is not available\n" );
      EXIT_ON_FAILURE;
   }

   printf( "CPUID is available\n" );
}


// If this is a genuine Intel CPU, then print that fact.
// If not, tell the user what it is and exit.
//
// If it is a genuine Intel CPU, make sure it's capapble of examining SGX
// features.
void isIntelCPU( void ) {
   uint32_t eax = 0;

   union cpuInfo_t {
      char cpuString[13];
      struct registers_t {
         uint32_t ebx;
         uint32_t edx;  // These are meant to be out-of-order (See CPUID.0)
         uint32_t ecx;
      } registers;
   } cpuInfo;

   memset( &cpuInfo, 0, sizeof( cpuInfo ) );

   native_cpuid32( &eax, &cpuInfo.registers.ebx, &cpuInfo.registers.ecx, &cpuInfo.registers.edx );

   if( strcmp( cpuInfo.cpuString, "GenuineIntel" ) != 0 ) {
      printf( "The CPU is not Genuine Intel\n" );
      printf( "The CPU String is: [%s]\n", cpuInfo.cpuString );
      EXIT_ON_FAILURE;
   }
   printf( "The CPU is Genuine Intel\n" );

   uint32_t SGXenumerationLeaf = 0x12;
   if( eax < SGXenumerationLeaf ) {  // CPUID.0:EAX is the maximum input value for basic CPUID.
      printf( "CPUID must be able to enumerate SGX instructions at leaf 0x%" PRIx32 "\n", SGXenumerationLeaf );
      printf( "Maximum enumeration leaf for Basic CPUID is: 0x%" PRIx32 "\n", eax );
      EXIT_ON_FAILURE;
   }
   printf( "CPUID is capable of examining SGX capabilities\n" );
}


// Print the register as if it contained a char[4] string array.  Return
// `true` if every character is printable.  Return `false` on the first
// non-printable character.
bool printRegisterAsASCII( uint32_t exx ) {
   for( int i = 0 ; i < 4 ; i++ ) {
      char byteToPrint = (exx >> (i*8)) & 0xFF;
      if( isprint( byteToPrint ) ) {
         printf( "%c", byteToPrint );
      } else {
         return false;
      }
   }

   return true;
}


// Print the CPU Brand String.  This will look like this:
//     CPU: Intel(R) Core(TM) i9-9980HK CPU @ 2.40GHz
void printCPUBrandString( void ) {
   uint32_t eax = 0x80000000;  // Check Processor Brand
   uint32_t ebx = 0;
   uint32_t edx = 0;
   uint32_t ecx = 0;

   native_cpuid32( &eax, &ebx, &ecx, &edx );
   // print_registers32( eax, ebx, ecx, edx );

   int processorBrandSupported = (eax) & 0x80000000;
   if( !processorBrandSupported ) {
      printf( "Processor Brand: 0\n" );
      return;
   }

   uint8_t processorBrandMaxIndex = eax - 0x80000000;

   printf( "CPU: " );
   for( int i = 2 ; i <= processorBrandMaxIndex ; i++ ) {
      eax = 0x80000000 + i;
      ebx = 0;
      ecx = 0;
      edx = 0;

      native_cpuid32( &eax, &ebx, &ecx, &edx );
      // print_registers32( eax, ebx, ecx, edx );

      if( printRegisterAsASCII( eax )     // Builtin operators like && perform
       && printRegisterAsASCII( ebx )     // short-circuit evaluation (do not
       && printRegisterAsASCII( ecx )     // evaluate the second operand if the
       && printRegisterAsASCII( edx ) ) { // final result is known after
         continue;                        // evaluating the first)
       } else {
         break;
       }
   }
   printf( "\n" );
}


void supportsSGXInstructions( void ) {
   uint32_t eax = 1;  // Basic CPUID Information leaf
   uint32_t ebx = 0;
   uint32_t edx = 0;
   uint32_t ecx = 0;

   native_cpuid32( &eax, &ebx, &ecx, &edx );
   // print_registers32( eax, ebx, ecx, edx );

   printf("  Stepping %-2d      ", eax & 0xF); // Bit 3-0
   printf("  Model %-2d         ", (eax >> 4) & 0xF); // Bit 7-4
   printf("  Family %-2d\n", (eax >> 8) & 0xF); // Bit 11-8
   printf("  Processor type %-2d", (eax >> 12) & 0x3); // Bit 13-12
   printf("  Extended model %-2d", (eax >> 16) & 0xF); // Bit 19-16
   printf("  Extended family %-2d\n", (eax >> 20) & 0xFF); // Bit 27-20

   // if smx set - SGX global enable is supported
   int smxFlag = (ecx >> 6) & 1;  // CPUID.1:ECX.[bit6]
   printf("Safer Mode Extensions (SMX): %d\n", smxFlag );

   eax = 7;  // Structured Extended Features leaf
   ebx = 0;
   ecx = 0;
   edx = 0;

   native_cpuid32( &eax, &ebx, &ecx, &edx );
   printf( "Extended feature bits (EAX=7, ECX=0): " );
   print_registers32( eax, ebx, ecx, edx );

   int sgxFeature = (ebx >> 2) & 1;  // (EAX=7, ECX=0):EBX[2]
   if( !sgxFeature ) {
      printf( "Does not support SGX\n" );
      EXIT_ON_FAILURE;
   }
   printf( "Supports SGX\n" );

   int sgxLaunchConfig = (ecx >> 30) & 1;  // (EAX=7, ECX=0):ECX[30]
   printf( "SGX Launch Configuration (SGX_LC): %d\n", sgxLaunchConfig );

   int sgxAttestationServices = (edx >> 1) & 1; // (EAX=7, ECX=0H):EDX[1]
   printf( "SGX Attestation Services (SGX_KEYS): %d\n", sgxAttestationServices );


   eax = 0x12;  // SGX Capability Enumeration Leaf
   ebx = 0;
   edx = 0;     // Sub-leaf 0
   ecx = 0;

   native_cpuid32( &eax, &ebx, &ecx, &edx );
   // print_registers32( eax, ebx, ecx, edx );

   /* SGX has to be enabled in MSR.IA32_Feature_Control.SGX_Enable
    * check with msr-tools: rdmsr -ax 0x3a
    * SGX_Enable is Bit 18
    * if SGX_Enable = 0, then although the SGX feature is available on the CPU,
    * no SGX leaf instructions will be enabled.  For more information check
    * the Intel Docs Architectures-software-developer-system-programming-manual - 35.1 Architectural MSRS
    */

   int sgx1 = (eax & 1);  // (EAX=12H, ECX=0H):EAX[0]
   printf( "SGX1 leaf instructions (SGX1): %d\n", sgx1 );

   int sgx2 = (eax >> 1) & 1; // (EAX=12H, ECX=0H):EAX[1]
   printf( "SGX2 leaf instructions (SGX2): %d\n", sgx2 );

   int sgxOversubscription1 = (eax >> 5) & 1; // (EAX=12H, ECX=0H):EAX[5]
   printf( "EINCVIRTCHILD, EDECVIRTCHILD, and ESETCONTEXT (OVERSUB-VMX): %d\n", sgxOversubscription1 );

   int sgxOversubscription2 = (eax >> 6) & 1; // (EAX=12H, ECX=0H):EAX[6]
   printf( "ETRACKC, ERDINFO, ELDBC, and ELDUC (OVERSUB-Supervisor): %d\n", sgxOversubscription2 );

   int sgxEVERIFYREPORT2 = (eax >> 7) & 1; // (EAX=12H, ECX=0H):EAX[7]
   printf( "EVERIFYREPORT2: %d\n", sgxEVERIFYREPORT2 );

   int sgxEUPDATESVN = (eax >> 10) & 1; // (EAX=12H, ECX=0H):EAX[10]
   printf( "Allow attestation w/ updated microcode (EUPDATESVN): %d\n", sgxEUPDATESVN );

   int sgxEDECCSSA = (eax >> 11) & 1; // (EAX=12H, ECX=0H):EAX[11]
   printf( "Allow enclave thread to decrement TCS.CSSA (EDECCSSA): %d\n", sgxEDECCSSA );

   printf( "Supported Extended features for MISC region of SSA (MISCSELECT) 0x%08" PRIx32 "\n", ebx );

   uint32_t maxNon64bitEnclaveBase= edx & 0xFF;
   printf( "The maximum supported enclave size in non-64-bit mode is 2^%" PRIu32 "\n", maxNon64bitEnclaveBase );

   uint32_t max64bitEnclaveBase= (edx & 0xFF00) >> 8;
   printf( "The maximum supported enclave size in     64-bit mode is 2^%" PRIu32 "\n", max64bitEnclaveBase );


   eax = 0x12;  // SGX Attributes Enumeration Leaf
   ebx = 0;
   ecx = 1;     // Sub-leaf 1
   edx = 0;

   native_cpuid32( &eax, &ebx, &ecx, &edx );
   // print_registers32( eax, ebx, ecx, edx );

   printf( "Raw ECREATE SECS.ATTRIBUTES[63:0]: %08" PRIx32 " %08" PRIx32 "\n", ebx, eax );

   printf( "    ECREATE SECS.ATTRIBUTES[DEBUG] (Debugger can read/write enclave data w/ EDBGRD/EDBGWR): %d\n", (eax >> 1) & 1 );
   printf( "    ECREATE SECS.ATTRIBUTES[MODE64BIT] (Enclave can run as 64-bit): %d\n", (eax >> 2) & 1 );
   printf( "    ECREATE SECS.ATTRIBUTES[PROVISIONKEY] (Provisioning key available from EGETKEY): %d\n", (eax >> 4) & 1 );
   printf( "    ECREATE SECS.ATTRIBUTES[EINITTOKEN_KEY] (EINIT token key available from EGETKEY): %d\n",(eax >> 5) & 1 );
   printf( "    ECREATE SECS.ATTRIBUTES[CET] (Enable Control-flow Enforcement Technology in enclave): %d\n",(eax >> 6) & 1 );
   printf( "    ECREATE SECS.ATTRIBUTES[KSS] (Key Separation and Sharing Enabled): %d\n",(eax >> 7) & 1 );
   printf( "    ECREATE SECS.ATTRIBUTES[AEXNOTIFY] (Threads may receive AEX notifications): %d\n",(eax >> 10) & 1 );

   printf( "Raw ECREATE SECS.ATTRIBUTES[127:64] (XFRM: Copy of XCR0): %08" PRIx32 " %08" PRIx32 "\n", edx, ecx );

}


void enumerateEPCsections( void ) {
   uint32_t eax = 0;
   uint32_t ebx = 0;
   uint32_t edx = 0;
   uint32_t ecx = 0;

   for( uint32_t i = 2 ; i <= NUMBER_OF_EPCs_TO_ENUMERATE ; i++ ) {
      eax = 0x12;  // SGX EPC Enumeration Leaf Leaf
      ebx = 0;
      ecx = i;     // Sub-leaf n (EPC number-ish)
      edx = 0;

      native_cpuid32( &eax, &ebx, &ecx, &edx );
      // print_registers32( eax, ebx, ecx, edx );

      uint8_t leafType = eax & 0x0F;
      uint64_t epcBasePhysAddr = 0;
      uint64_t epcSectionSize = 0;
      switch( leafType ) {
         case 0:
            if( eax == 0 && ebx == 0 && ecx == 0 && edx == 0 ) {
               // printf( "0" );  // The leaf type is invalid and all the the values are as expected (zeroed out)
            } else {
               // printf( "!" );  // One of the registers has an unexpected value
            }
            break;
         case 1:
            // printf( "@" );  // The leaf is an EPC section
            // printf( "\n" );

            epcBasePhysAddr = (eax & 0xFFFFF000) | ((ebx & (uint64_t) 0x000FFFFF) << 32);
            epcSectionSize  = (ecx & 0xFFFFF000) | ((edx & (uint64_t) 0x000FFFFF) << 32);

            char confidentiality = ' ';
            char integrity = ' ';

            switch( ecx & 0x0F ) {
               case 0x1:
                   confidentiality = 'c';
                   integrity = 'i';
                   break;
               case 0x2:
                   confidentiality = 'c';
                   break;
               default:
                   break;
            }

/*
 * Validated on SGX hardware in /proc/iomem
 *   70200000-75f7ffff : INT0E0C:00
 * Prints the following:
 *   EPC[0]: Protection: ci  Base phys addr: 0000000070200000  size: 0000000005d80000
 */
            printf( "EPC[%u]: Protection: %c%c  Base phys addr: %016" PRIx64 "  size: %016" PRIx64 "\n"
              ,i-2
              ,confidentiality
              ,integrity
              ,epcBasePhysAddr
              ,epcSectionSize );

            break;
         default:
            // printf( "r" );  // The leaf type is reserved
            break;
      }
   }
}
