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
/// Sample Output (from our development system):
///     Start test-sgx (version 2.0.0) at Mon Nov 20 18:20:43 2023
///     CPUID is available
///     The CPU is Genuine Intel
///     CPUID is capable of examining SGX capabilities
///     CPU: Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz
///       Stepping 13        Model 14           Family 6 
///       Processor type 0   Extended model 9   Extended family 0 
///     Safer Mode Extensions (SMX): 1
///     Extended feature bits (EAX=7, ECX=0): eax: 00000000  ebx: 029c6fbf  ecx: 40000000  edx: bc000e00
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
///     EPC[0]: Protection: ci  Base phys addr: 0000000070200000  size: 0000000005d80000
///     vDSO base address: 0x7ffddbf41000
///     Printing Symbol Table:
///     vDSO symbol: __vdso_time
///     vDSO symbol: getcpu
///     vDSO symbol: __vdso_clock_getres
///     vDSO symbol: __vdso_getcpu
///     vDSO symbol: clock_getres
///     vDSO symbol: __vdso_gettimeofday
///     vDSO symbol: LINUX_2.6
///     vDSO symbol: gettimeofday
///     vDSO symbol: __vdso_clock_gettime
///     vDSO symbol: time
///     vDSO symbol: __vdso_sgx_enter_enclave
///     vDSO symbol: clock_gettime
///     Raw IA32_FEATURE_CONTROL: 0000000000060005
///         IA32_FEATURE_CONTROL.LOCK_BIT[bit 0]: 1
///         IA32_FEATURE_CONTROL.SGX_LAUNCH_CONTROL[bit 17] (Is the SGX LE PubKey writable?): 1
///         IA32_FEATURE_CONTROL.SGX_GLOBAL_ENABLE[bit 18]: 1
///     The SGX Launch Enclave Public Key Hash can be changed
///     IA32_SGXLEPUBKEYHASH: 72d712fed48f9f2f 5282ca7175d8f132 e40577fe4e7127ca c4e8225afdcf2262
///     Raw IA32_SGX_SVN_STATUS: 0000000e00020001
///     MSR_SGXOWNEREPOCH not readable
///     XSAVE features and state-components
///       Maximum size (in bytes) of current XCR0 XSAVE area: 1088
///       Maximum size (in bytes) of all-set XCR0 XSAVE area: 1088
///       Size (in bytes) of current XCR0+IA32_XSS XSAVE area: 960
///       Supported XCR0:     000000000000001f
///       Actual    XCR0:     000000000000001f
///       Supported IA32_XSS: 0000000000000100
///       Actual    IA32_XSS: 0000000000000000
///         Register Name    Supported Value Description
///         ======== ======= ========= ===== ===========
///         XCR0     x87:       yes      set x87 Floating Point Unit & MMX
///         XCR0     SSE:       yes      set MXCSR and XMM registers
///         XCR0     AVX:       yes      set YMM registers
///         XCR0     BNDREG:    yes      set MPX for BND registers
///         XCR0     BNDCSR:    yes      set MPX for BNDCFGU and BNDSTATUS registers
///         XCR0     opmask:     no    clear AVX-512 for AVX opmask and AKA k-mask
///         XCR0     ZMM_hi256:  no    clear AVX-512 for the upper-halves of lower ZMM registers
///         XCR0     Hi16_ZMM:   no    clear AVX-512 for the upper ZMM registers
///         IA32_XSS PT:        yes    clear Processor Trace
///         XCR0     PKRU:       no    clear User Protection Keys
///         IA32_XSS PASID:      no    clear Process Address Space ID
///         IA32_XSS CET_U:      no    clear Control-flow Enforcement Technology: user-mode functionality MSRs
///         IA32_XSS CET_S:      no    clear CET: shadow stack pointers for rings 0,1,2
///         IA32_XSS HDC:        no    clear Hardware Duty Cycling
///         IA32_XSS UINTR:      no    clear User-Mode Interrupts
///         IA32_XSS LBR:        no    clear Last Branch Record
///         IA32_XSS HWP:        no    clear Hardware P-state control
///         XCR0     TILECFG:    no    clear AMX - Advanced Matrix Extensions
///         XCR0     TILEDATA:   no    clear AMX - Advanced Matrix Extensions
///         XCR0     APX:        no    clear Extended General Purpose Registers R16-R31
///       Supported XSAVE feature flags: 0000000f
///         xsaveopt - save state-components that have been modified since last XRSTOR: 1
///         xsavec - save/restore state with compaction: 1
///         xgetbv_ecx1 - XGETBV with ECX=1 support: 1
///         xss - save/restore state with compaction, including supervisor state: 1
///         xfd - Extended Feature Disable supported: 0
///     End test-sgx
///
/// @file   test-sgx.c
/// @author Lars Luhr   	<mail@ayeks.de>
/// @author Mark Nelson 	<marknels@hawaii.edu>
/// @author Brooke Maeda <bmhm@hawaii.edu>
/// @author Eliya Nakamura	<eliyanak@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>     // For printf()
#include <inttypes.h>  // For PRIx64 uint64_t PRIx32 uint32_t
#include <time.h>      // For fetching timestamps

#include "test-sgx.h"  // For obvious reasons
#include "cpuid.h"     // For native_cpuid32()
#include "rdmsr.h"     // For checkCapabilities()
#include "vdso.h"      // For dump_vDSO()
#include "xsave.h"     // For print_XSAVE_enumeration()

// Prove the compiler regognizes SGX instructions
void sgxInstruction( void ) {
#if !defined(_MSC_VER)

   __asm( "encls" );
   __asm( "enclu" );

#endif
}


int main( void ) {
   // Get current timestamp
   time_t timestamp;
   time(&timestamp);

   // Print program info
   printf( "Start " PROGRAM_NAME " (version %d.%d.%d) at %s\n", PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH, ctime(&timestamp) );

   doesCPUIDwork();
   isIntelCPU();
   printCPUBrandString();
   supportsSGXInstructions();
   enumerateEPCsections();
   dump_vDSO();

   if( checkCapabilities() ) {
      read_SGX_MSRs();
   }

   print_XSAVE_enumeration();

   printf( "End " PROGRAM_NAME "\n" );
   return EXIT_SUCCESS;
}

