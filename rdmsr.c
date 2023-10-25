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
/// @file   rdmsr.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>  // For printf()
#include <sys/capability.h>  // For CAP_SYS_ADMIN, cap_get_proc() cap_set_flag() cap_set_proc() cap_free()

#include "rdmsr.h"  // For obvious reasons


/// On Linux, return true if we are running as root (with CAP_SYS_ADMIN).  In
/// all other situations, return false.
bool checkCapabilities() {
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
         printf( "Not running with root privlidges\n" ) ;
         return false;
      }
      
      rVal = cap_free( myCapabilities ) ;
      if( rVal != 0 ) {
         printf( "cap_free failed\n" ) ;
         return false;
      }
      
      return true;
   
   #elif
      return false;  // In all other operating systems, return false
   #endif
}
