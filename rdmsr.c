///////////////////////////////////////////////////////////////////////////////
//  rdmsr.c - 2023
//
/// This module contains Linux-specific, privlidged code (wants to run as root)
/// that utilizes the RDMSR instruction by reading from /dev/cpu/0/msr to
/// discover & report SGX additional capabilities.
///
/// Important design note:  We are always reading from cpu 0.  It's possible
/// that different CPUs may have different SGX values, so beware that what
/// this program reports may not be what is actually executing.  If you discover
/// that this happens, please eMail marknels@hawaii.edu and we'll enhance this
/// module to look at multiple CPU settings more carefully.
///
/// CPU vendors enable limited CPU configuration via model-specific registers
/// or MSRs.  They can be read or written to by the RDMSR and WRMSR instructions.
/// MSRs are special registers that can be read from and written to by
/// privileged code (usually the operating system).  Every MSR has a unique
/// 32-bit address and a data size of 64-bits.
///
/// Generally, MSRs are used for interaction with the CPU, such as enabling and
/// disabling CPU features, debugging, and performance monitoring.
///
/// @see https://www.intel.com/content/www/us/en/developer/articles/technical/software-security-guidance/technical-documentation/cpuid-enumeration-and-architectural-msrs.html
///
/// @file   rdmsr.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

/// Enables declaration of `pread()`
///
/// @see https://man.archlinux.org/man/pread.2
/// @NOLINTNEXTLINE(bugprone-reserved-identifier, cert-dcl37-c, cert-dcl51-cpp): This is a legitimate use of a reserved identifier
#define _XOPEN_SOURCE 700

#include <stdio.h>      // For printf()
#include <inttypes.h>   // For PRIx64 uint64_t

#ifdef __linux__
   #include <fcntl.h>   // For open() O_RDONLY
   #include <unistd.h>  // For pread() close()
   #include <sys/capability.h>  // For CAP_SYS_ADMIN cap_get_proc() cap_set_flag() cap_set_proc() cap_free()
#endif

#include "rdmsr.h"           // For obvious reasons
#include "test-sgx.h"        // For EXIT_ON_FAILURE


#define IA32_FEATURE_CONTROL   0x3A
#define IA32_SGXLEPUBKEYHASH0  0x8C
#define IA32_SGX_SVN_STATUS   0x500
#define MSR_SGXOWNEREPOCH0    0x300


/// On Linux, return true if we are running as root (with CAP_SYS_ADMIN).  In
/// all other situations, return false.
bool checkCapabilities( void ) {
   #ifdef __linux__

      cap_t myCapabilities ;
      const cap_value_t requiredCapabilitiesList[1] = { CAP_SYS_ADMIN } ;

      if( !CAP_IS_SUPPORTED( CAP_SYS_ADMIN )) {
         printf( "Does not support CAP_SYS_ADMIN\n" ) ;
         return false;
      }

      myCapabilities = cap_get_proc() ;
      if( myCapabilities == NULL ) {
         printf( "Unable to get the process' capabilities\n" ) ;
         return false;
      }

      int rVal = cap_set_flag( myCapabilities, CAP_EFFECTIVE, 1, requiredCapabilitiesList, CAP_SET) ;
      if( rVal != 0 ) {
         printf( "cap_set_flag() did not set a capability\n" ) ;
         return false;
      }

      rVal = cap_set_proc( myCapabilities ) ;
      if( rVal != 0 ) {
         printf( "Not running with admin privlidges... On Linux, run as root for more SGX info.\n" ) ;
         return false;
      }

      rVal = cap_free( myCapabilities ) ;
      if( rVal != 0 ) {
         printf( "cap_free failed\n" ) ;
         return false;
      }

      return true;

   #else
      return false;  // In all other operating systems, return false
   #endif
}


/// Read an MSR on a CPU
///
/// Courtesy of Intel:  https://github.com/intel/msr-tools/blob/master/rdmsr.c
///
/// @param reg:    The MSR register to read
/// @param cpu:    The CPU number (0, 1, 2, ...) to read
/// @param pData:  Read the data into a uint64_t value pointed to by pData
///
/// @returns true if successful, false if not successful
bool rdmsr( uint32_t reg, int cpu, uint64_t* pData ) {
   #ifdef __linux__

      char     msr_file_name[64];
      int      fd;  // File descriptor to /dev/cpu/%d/msr

      if( reg >= 0x40000000 && reg <= 0x4000FFFF ) {
         fprintf( stderr, "rdmsr: Attempting to read from reserved range\n" );
         return false;
      }

      sprintf( msr_file_name, "/dev/cpu/%d/msr", cpu );
      fd = open( msr_file_name, O_RDONLY );
      if (fd < 0) {
         fprintf( stderr, "rdmsr: CPU %d doesn't support MSRs\n", cpu );
         return false;
      }

      // pread:  Upon successful completion, pread shall return a non-negative
      //         integer indicating  the  number of bytes actually read.
      if( pread( fd, pData, sizeof *pData, reg ) != sizeof *pData ) {
         // fprintf( stderr, "rdmsr: CPU %d did not read MSR 0x%08" PRIx32 "\n", cpu, reg );
         return false;
      }

      close(fd);

   #else
      (void) reg;    // Squelch unused parameter warnings
      (void) cpu;
      (void) pData;
   #endif

   return true;
}


/// Read and print SGX-specific MSRs on a CPU
void read_SGX_MSRs( void ) {
   uint64_t feature_control_msr;

   if( rdmsr( IA32_FEATURE_CONTROL, 0, &feature_control_msr ) ) {
      printf( "Raw IA32_FEATURE_CONTROL: %016" PRIx64 "\n", feature_control_msr );

      printf( "    IA32_FEATURE_CONTROL.LOCK_BIT[bit 0]: %d\n", (int)((feature_control_msr >> 0) & 1 ));
      printf( "    IA32_FEATURE_CONTROL.SGX_LAUNCH_CONTROL[bit 17] (Is the SGX LE PubKey writable?): %d\n", (int)((feature_control_msr >> 17) & 1 ));
      printf( "    IA32_FEATURE_CONTROL.SGX_GLOBAL_ENABLE[bit 18]: %d\n", (int)((feature_control_msr >> 18) & 1 ));

      if( (feature_control_msr & 1 ) && ( (feature_control_msr >> 17) & 1 ) ) {
         printf( "The SGX Launch Enclave Public Key Hash can be changed\n" );
      } else {
         printf( "The SGX Launch Enclave Public Key Hash can NOT be changed\n" );
      }

   } else {
      printf( "IA32_FEATURE_CONTROL not readable\n" );
   }


   uint64_t sgx_lePubKeyHash0_msr;
   uint64_t sgx_lePubKeyHash1_msr;
   uint64_t sgx_lePubKeyHash2_msr;
   uint64_t sgx_lePubKeyHash3_msr;

   if(    rdmsr( IA32_SGXLEPUBKEYHASH0    , 0, &sgx_lePubKeyHash0_msr )
       && rdmsr( IA32_SGXLEPUBKEYHASH0 + 1, 0, &sgx_lePubKeyHash1_msr )
       && rdmsr( IA32_SGXLEPUBKEYHASH0 + 2, 0, &sgx_lePubKeyHash2_msr )
       && rdmsr( IA32_SGXLEPUBKEYHASH0 + 3, 0, &sgx_lePubKeyHash3_msr )
     ) {
      printf( "IA32_SGXLEPUBKEYHASH: %016" PRIx64 " %016" PRIx64 " %016" PRIx64 " %016" PRIx64 "\n"
         ,sgx_lePubKeyHash0_msr
         ,sgx_lePubKeyHash1_msr
         ,sgx_lePubKeyHash2_msr
         ,sgx_lePubKeyHash3_msr );
      } else {
         printf( "IA32_SGXLEPUBKEYHASH[0-3] not readable\n" );
      }

   uint64_t sgx_sgx_svn_status_msr;
   if( rdmsr( IA32_SGX_SVN_STATUS    , 0, &sgx_sgx_svn_status_msr ) ) {
      printf( "Raw IA32_SGX_SVN_STATUS: %016" PRIx64 "\n", sgx_sgx_svn_status_msr );
   } else {
      printf( "IA32_SGX_SVN_STATUS not readable\n" );
   }

   // This may not be available on all CPUs
   uint64_t sgx_OwnerEpoch0_msr;
   uint64_t sgx_OwnerEpoch1_msr;

   if(    rdmsr( MSR_SGXOWNEREPOCH0    , 0, &sgx_OwnerEpoch0_msr )
       && rdmsr( MSR_SGXOWNEREPOCH0 + 1, 0, &sgx_OwnerEpoch1_msr ) ) {
      printf( "Raw MSR_SGXOWNEREPOCH: %016" PRIx64 " %016" PRIx64 "\n", sgx_OwnerEpoch1_msr, sgx_OwnerEpoch0_msr );
   } else {
      printf( "MSR_SGXOWNEREPOCH not readable\n" );
   }
}
