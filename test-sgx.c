///////////////////////////////////////////////////////////////////////////////
//  test-sgx.c - 2023
//
/// A command line based SGX feature enumeration tool based on the Intel
/// Architecture Instruction Set Extensions and Future Features manual dated
/// September 2023
///
/// @see https://www.intel.com/content/www/us/en/content-details/790021/intel-architecture-instruction-set-extensions-programming-reference.html
///
/// As the purpose of this app is to enumerate SGX features, it will assume
/// that it's on an somewhat modern platform platform.
///
/// This has been tested with:
///   - Linux / gcc 13.1:  gcc -Wl,--no-as-needed -Wall -Wextra -Wpedantic -masm=intel -o test-sgx -lcap cpuid.c rdmsr.c test-sgx.c
///   - Windows 11 / Visual Studio 2022 (x64 Native Tools):  cl test-sgx.c cpuid.c rdmsr.c
///   - MacOS / Clang 15:  clang -Wall -Wextra -Wpedantic -masm=intel -std=c2x -Wno-gnu-binary-literal -o test-sgx cpuid.c rdmsr.c test-sgx.c
///
/// If you receive this message during compiling:  fatal error: sys/capability.h: No such file or directory
/// ... then run this:  sudo apt-get install libcap-dev
///
/// The output of this program needs to be treated with some skepticism... here
/// are some scenarios that it may mislead you:
///   - You are running in a VM.  The host's CPU (and BIOS) may actually support
///     SGX, but the hypervisor may not.  In this case, test-sgx will report that
///     the CPU does not support SGX when it actually does.
///   - You are running on a Mac.  The CPU may actually support SGX, but the
///     BIOS does not enable it.
///
/// Test CPU is really intended to inform you of the capabilities of the system
/// you are on right now... not an entire class of CPUs or motherboards.
///
/// Sample Output (from an Azure VM):
///     Start test-sgx
///     CPUID is available
///     The CPU is Genuine Intel
///     CPUID is capable of examining SGX capabilities
///     CPU: Intel(R) Xeon(R) E-2288G CPU @ 3.70GHz
///       Stepping 13        Model 14           Family 6 
///       Processor type 0   Extended model 9   Extended family 0 
///     Safer Mode Extensions (SMX): 0
///     Extended feature bits (EAX=7, ECX=0): eax: 00000000  ebx: 009c6fbd  ecx: 40000000  edx: 20000400
///     Supports SGX
///     SGX Launch Configuration (SGX_LC): 1
///     SGX Attestation Services (SGX_KEYS): 0
///     SGX1 leaf instructions (SGX1): 1
///     SGX2 leaf instructions (SGX2): 0
///     EINCVIRTCHILD, EDECVIRTCHILD, and ESETCONTEXT (OVERSUB-VMX): 0
///     ETRACKC, ERDINFO, ELDBC, and ELDUC (OVERSUB-Supervisor): 0
///     EVERIFYREPORT2: 0
///     Allow attestation w/ updated microcode (EUPDATESVN): 0
///     Allow enclave thread to decrement TCS.CSSA (EDECCSSA): 1
///     Supported Extended features for MISC region of SSA (MISCSELECT) 0x00000000
///     The maximum supported enclave size in non-64-bit mode is 2^31
///     The maximum supported enclave size in     64-bit mode is 2^36
///     Raw ECREATE SECS.ATTRIBUTES[63:0]: 00000000 00000436
///         ECREATE SECS.ATTRIBUTES[DEBUG] (Debugger can read/write enclave data w/ EDBGRD/EDBGWR): 1
///         ECREATE SECS.ATTRIBUTES[MODE64BIT] (Enclave can run as 64-bit): 1
///         ECREATE SECS.ATTRIBUTES[PROVISIONKEY] (Provisioning key available from EGETKEY): 1
///         ECREATE SECS.ATTRIBUTES[EINITTOKEN_KEY] (EINIT token key available from EGETKEY): 1
///         ECREATE SECS.ATTRIBUTES[CET] (Enable Control-flow Enforcement Technology in enclave): 0
///         ECREATE SECS.ATTRIBUTES[KSS] (Key Separation and Sharing Enabled): 0
///         ECREATE SECS.ATTRIBUTES[AEXNOTIFY] (Threads may receive AEX notifications): 1
///     Raw ECREATE SECS.ATTRIBUTES[127:64] (XFRM: Copy of XCR0): 00000000 0000001f
///     EPC[0]: Protection: ci  Base phys addr: 00000001c0000000  size: 0000000001c00000
///     Raw IA32_FEATURE_CONTROL: 0000000000060001
///         IA32_FEATURE_CONTROL.LOCK_BIT[bit 0]: 1
///         IA32_FEATURE_CONTROL.SGX_LAUNCH_CONTROL[bit 17] (Is the SGX LE PubKey writable?): 1
///         IA32_FEATURE_CONTROL.SGX_GLOBAL_ENABLE[bit 18]: 1
///     The SGX Launch Enclave Public Key Hash can be changed
///     IA32_SGXLEPUBKEYHASH: a6053e051270b7ac 6cfbe8ba8b3b413d c4916d99f2b3735d d4f8c05909f9bb3b
///     Raw IA32_SGX_SVN_STATUS: 0000000000000000
///     MSR_SGXOWNEREPOCH not readable
///     End test-sgx
///
/// @file   test-sgx.c
/// @author Lars Luhr   <mail@ayeks.de>
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>     // For printf()
#include <inttypes.h>  // For PRIx64 uint64_t PRIx32 uint32_t

#include "test-sgx.h"  // For obvious reasons
#include "cpuid.h"     // For native_cpuid32()
#include "rdmsr.h"     // For checkCapabilities()


// Prove the compiler regognizes SGX instructions
void sgxInstruction( void ) {
#if !defined(_MSC_VER)

   __asm( "encls" );
   __asm( "enclu" );

#endif
}


int main( void ) {
   printf( "Start " PROGRAM_NAME "\n" );

   doesCPUIDwork();
   isIntelCPU();
   printCPUBrandString();
   supportsSGXInstructions();
   enumerateEPCsections();
   
   if( checkCapabilities() ) {
      read_SGX_MSRs();
   }


   printf( "End " PROGRAM_NAME "\n" );
   return EXIT_SUCCESS;
}
