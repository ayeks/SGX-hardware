///////////////////////////////////////////////////////////////////////////////
//  vdso.h - 2023
//
/// This module contains code to dump the vDSO symbol table
///
/// @file   vdso.h
/// @author Mark Nelson <marknels@hawaii.edu>
/// @author Brooke Maeda <bmhm@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <elf.h>       // For Elf64_Sym Elf64_Word

/// vDSO symbol table information
struct vdso_symtab {
	Elf64_Sym*  elf_symtab;     ///< Symbol table
	const char* elf_symstrtab;  ///< String table
	Elf64_Word* elf_hashtab;    ///< Hash table
};

// Print the symbol table pointed to by `symtab`
void print_whole_symbol_table(struct vdso_symtab* symtab);


// Print out the symbol table
void dump_vDSO ( void );
