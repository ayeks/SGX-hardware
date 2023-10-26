///////////////////////////////////////////////////////////////////////////////
//  rdmsr.h - 2023
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
/// @file   rdmsr.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdbool.h>  // For bool

/// On Linux, return true if we are running as root (with CAP_SYS_ADMIN).  In
/// all other situations, return false.
bool checkCapabilities( void );

/// Read and print SGX-specific MSRs on a CPU
void read_SGX_MSRs( void );
