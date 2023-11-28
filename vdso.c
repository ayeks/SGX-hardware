///////////////////////////////////////////////////////////////////////////////
//  vdso.c - 2023
//
/// This module contains code to dump the vDSO symbol table
///
/// @file   vdso.c
/// @author Mark Nelson <marknels@hawaii.edu>
/// @author Brooke Maeda <bmhm@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>     // For printf()
#include "vdso.h"     // For obvious reasons
#include <sys/auxv.h>  // For getauxval
#include <asm/sgx.h>
#include <stdbool.h>   // For bool true false

/// Get the vDSO dynamic link table `PT_DYNAMIC`
///
/// Iterate over `phdrtab` until it finds the `PT_DYNAMIC` entry
///
/// @param addr vDSO base address
/// @return A pointer to the table or `NULL` if it's not found
static Elf64_Dyn* vdso_get_dynamic_link_table( void* addr ) {
	Elf64_Ehdr* ehdr = addr;
	// Elf64_Phdr* phdrtab = (Elf64_Phdr*)addr + ehdr->e_phoff;
	Elf64_Phdr* phdrtab = addr + ehdr->e_phoff;

	for( int i = 0 ; i < ehdr->e_phnum ; i++ ) {
		if( phdrtab[i].p_type == PT_DYNAMIC ) {
			// return (Elf64_Dyn*)addr + phdrtab[i].p_offset;
			return addr + phdrtab[i].p_offset;
		}
	}

	return NULL;
}


/// Get a dynamic section from a vDSO file
///
/// @param addr   vDSO base address
/// @param dyntab Pointer to a vDSO dynamic link table
/// @param tag    A dynamic section type. eg. `DT_HASH`, `DT_STRTAB`, et.al.
/// @return A pointer to a dynamic vDSO section or `NULL` if it's not found
static void* vdso_get_dynamic_section( void* addr, Elf64_Dyn* dyntab, Elf64_Sxword tag ) {
	for( int i = 0 ; dyntab[i].d_tag != DT_NULL ; i++ )
		if( dyntab[i].d_tag == tag )
			// return (Elf64_Phdr*)addr + dyntab[i].d_un.d_ptr;
			return addr + dyntab[i].d_un.d_ptr;

	return NULL;
}

/// Get the vDSO symbol table
///
/// @param addr vDSO base address
/// @param symtab A pointer to a symbol table structure which gets populated
///              by this function.
/// @return `true` if successful.  `false` if not.
static bool vdso_get_symbol_table( void* addr, struct vdso_symtab* symtab ) {
	Elf64_Dyn* dyntab = vdso_get_dynamic_link_table( addr );

	symtab->elf_symtab = vdso_get_dynamic_section( addr, dyntab, DT_SYMTAB );
	if( !symtab->elf_symtab )
		return false;

	symtab->elf_symstrtab = vdso_get_dynamic_section( addr, dyntab, DT_STRTAB );
	if( !symtab->elf_symstrtab )
		return false;

	symtab->elf_hashtab = vdso_get_dynamic_section( addr, dyntab, DT_HASH );
	if( !symtab->elf_hashtab )
		return false;

	return true;
}


/// Print the symbol table pointed to by `symtab`
///
/// @param symtab Pointer to a vDSO symbol table
/// @return void
void print_whole_symbol_table(struct vdso_symtab* symtab) {
    Elf64_Word bucketnum = symtab->elf_hashtab[0];
    Elf64_Word* buckettab = &symtab->elf_hashtab[2];
    Elf64_Word* chaintab = &symtab->elf_hashtab[2 + bucketnum];
    Elf64_Sym* sym;

    for (Elf64_Word i = 0; i < bucketnum; ++i) {
        for (Elf64_Word j = buckettab[i]; j != STN_UNDEF; j = chaintab[j]) {
            sym = &symtab->elf_symtab[j];
            printf("vDSO symbol: %s\n", &symtab->elf_symstrtab[sym->st_name]);
        }
    }
}

/// Print out the symbol table
void dump_vDSO ( void ) {
	struct vdso_symtab symtab;

   // Get vDSO base address
	void* vdso_base_addr;
	vdso_base_addr = (void *)getauxval( AT_SYSINFO_EHDR );
	if( !vdso_base_addr ){
		printf("Can't get vDSO base address\n");
		return;
	}
	
	printf( "vDSO base address: %p\n", vdso_base_addr );

	if( !vdso_get_symbol_table( vdso_base_addr, &symtab ) ){
		printf("Can't get symbol table\n");
		return;
	}

    printf("Printing Symbol Table:\n");
	print_whole_symbol_table( &symtab );

}
