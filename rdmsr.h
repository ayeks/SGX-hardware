///////////////////////////////////////////////////////////////////////////////
//  rdmsr.h - 2023
//
/// This module contains Linux-specific, privlidged code (wants to run as root)
/// that utilizes the RDMSR instruction by reading from /dev/cpu/0/msr to 
/// discover & report SGX additional capabilities.
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
