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
#include <inttypes.h>   // For PRIx64 uint64_t


#define IA32_FEATURE_CONTROL  0x03A
#define IA32_SGXLEPUBKEYHASH0 0x08C
#define IA32_SGX_SVN_STATUS   0x500
#define MSR_SGXOWNEREPOCH0    0x300
#define IA32_XSS              0xda0


/// On Linux, return true if we are running as root (with CAP_SYS_ADMIN).  In
/// all other situations, return false.
bool checkCapabilities( void );

/// Read an MSR on a CPU
bool rdmsr( uint32_t reg, int cpu, uint64_t* pData );

/// Read and print SGX-specific MSRs on a CPU
void read_SGX_MSRs( void );
