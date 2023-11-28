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

/// Compute the hash of an ELF symbol
/// 
/// @param name The name of the symbol to hash
/// @return the hash of `name`
// static unsigned long hash_elf_symbol( const char* name ) {
// 	unsigned long h = 0, high;

// 	while( *name ) {
// 		h = (h << 4) + *name++;
// 		high = h & 0xf0000000;

// 		if( high )
// 			h ^= high >> 24;

// 		h &= ~high;
// 	}

// 	return h;
// }


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


/// Get symbol `name` from the symbol table pointed to by `symtab`
///
/// @param symtab Pointer to a vDSO symbol table
/// @param name   The name of the symbol to retrieve
/// @return void
// void get_symbol_from_table( struct vdso_symtab* symtab, const char* name ) {
// // void get_symbol_from_table( struct vdso_symtab* symtab ) {
// 	Elf64_Word  bucketnum = symtab->elf_hashtab[0];
// 	Elf64_Word* buckettab = &symtab->elf_hashtab[2];
// 	Elf64_Word* chaintab  = &symtab->elf_hashtab[2 + bucketnum];
// 	Elf64_Sym*  sym;

// 	for( Elf64_Word i = buckettab[ hash_elf_symbol(name) % bucketnum ] ; i != STN_UNDEF ; i = chaintab[i] ) {
// 		sym = &symtab->elf_symtab[i];
// 		printf( "vDSO symbol: %s\n", &symtab->elf_symstrtab[ sym->st_name ] );
// 		//if( !strcmp( name, &symtab->elf_symstrtab[ sym->st_name ] ) )
// 			//return sym;
// 	}
	
// }

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


/// Get the address of a vDSO symbol
///
/// @param symbol The name of the symbol to retrieve
/// @return The vDSO address of the symbol or 0 if it can't be found
// Elf64_Addr get_vDSO_symbol( const char* symbol ) {
void get_vDSO_symbol( void ) {
//    Elf64_Sym*         vdso_symbol_ptr = NULL;
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

	//vdso_symbol_ptr = get_symbol_from_table( &symtab, symbol );
	// printf("Doing get_symbol_from_table\n");
    // get_symbol_from_table( &symtab,  symbol );
	// printf("Doing print_whole_symbol_table\n");
	print_whole_symbol_table( &symtab );
    return;
    // return 0;
// 	if( !vdso_symbol_ptr )
// 		goto err;
	
// 	return (Elf64_Addr) (vdso_base_addr + vdso_symbol_ptr->st_value);
	
// err:
// 	// fprintf( stderr, "Failed to find vDSO symbol %s\n", symbol );
// 	fprintf( stderr, "Failed to find vDSO symbol\n" );

// 	return 0;	
}

// print out the symbol table
void dump_vDSO ( void ) {
    printf("Printing Symbol Table:\n");
	get_vDSO_symbol( );

}
