///////////////////////////////////////////////////////////////////////////////
//  vdso.c - 2023
//
/// This module contains code to dump the vDSO symbol table
///
/// @file   vdso.c
/// @author Mark Nelson <marknels@hawaii.edu>
/// @author Brooke Maeda <bmhm@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <elf.h>       // For ELF64_
#include <stdbool.h>   // For bool true false

/// vDSO symbol table information
struct vdso_symtab {
	Elf64_Sym*  elf_symtab;     ///< Symbol table
	const char* elf_symstrtab;  ///< String table
	Elf64_Word* elf_hashtab;    ///< Hash table
};


/// Get symbol `name` from the symbol table pointed to by `symtab`
void get_symbol_from_table( struct vdso_symtab* symtab, const char* name );


/// Get the address of a vDSO symbol
Elf64_Addr get_vDSO_symbol( const char* symbol );

// print out the symbol table
void dump_vDSO ( void );