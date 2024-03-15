///////////////////////////////////////////////////////////////////////////////
//  test-sgx.h - 2023
//
/// Things that every module in test-sgx needs to know.
///
/// @file   test-sgx.h
/// @author Lars Luhr   <mail@ayeks.de>
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdlib.h>  // For exit() EXIT_FAILURE

#define PROGRAM_NAME "test-sgx"
#define PROGRAM_VERSION_MAJOR 2
#define PROGRAM_VERSION_MINOR 0
#define PROGRAM_VERSION_PATCH 0

#define EXIT_ON_FAILURE  exit( EXIT_FAILURE );
