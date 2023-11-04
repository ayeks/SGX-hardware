///////////////////////////////////////////////////////////////////////////////
//  cpuid.h - 2023
//
/// This module contains multi-platform, non-privlidged code that utilizes the
/// CPUID instruction to discover & report SGX capabilities.
///
/// @file   cpuid.h
/// @author Lars Luhr   <mail@ayeks.de>
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <inttypes.h>  // For PRIx64 uint64_t PRIx32 uint32_t


// Print the register set:
//     eax: 80000008  ebx: 00000000  ecx: 00000000  edx: 00000000
void print_registers32( uint32_t eax
                       ,uint32_t ebx
                       ,uint32_t ecx
                       ,uint32_t edx );


/// Does this CPU support the CPUID instruction?
///
/// gcc supports intrisics to detect certain CPU features:
///   https://gcc.gnu.org/onlinedocs/gcc-13.2.0/gcc/x86-Built-in-Functions.html
///   ... however, as of GCC 13.2, the detection does not support SGX, so
///   we'll do it old school.
///
extern void doesCPUIDwork( void );


// If this is a genuine Intel CPU, then print that fact.
// If not, tell the user what it is and exit.
//
// If it is a genuine Intel CPU, make sure it's capapble of examining SGX
// features.
void isIntelCPU( void );


// Print the CPU Brand String.  This will look like this:
//     CPU: Intel(R) Core(TM) i9-9980HK CPU @ 2.40GHz
void printCPUBrandString( void );


void supportsSGXInstructions( void );


void enumerateEPCsections( void );
