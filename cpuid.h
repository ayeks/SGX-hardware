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

/// Call `CPUID`, passing `eax`, `ebx`, `ecx` and `eax` in & out.
extern void native_cpuid32( uint32_t* eax
                           ,uint32_t* ebx
                           ,uint32_t* ecx
                           ,uint32_t* edx );

