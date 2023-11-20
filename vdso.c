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



/// Get the vDSO dynamic link table `PT_DYNAMIC`
///
/// Iterate over `phdrtab` until it finds the `PT_DYNAMIC` entry
///
/// @param addr vDSO base address
/// @return A pointer to the table or `NULL` if it's not found
static Elf64_Dyn* vdso_get_dynamic_link_table( void* addr ) {
	Elf64_Ehdr* ehdr = addr;
	Elf64_Phdr* phdrtab = addr + ehdr->e_phoff;

	for( int i = 0 ; i < ehdr->e_phnum ; i++ ) {
		if( phdrtab[i].p_type == PT_DYNAMIC ) {
			return addr + phdrtab[i].p_offset;
		}
	}

	return NULL;
}

/// Compute the hash of an ELF symbol
/// 
/// @param name The name of the symbol to hash
/// @return the hash of `name`
static unsigned long hash_elf_symbol( const char* name ) {
	unsigned long h = 0, high;

	while( *name ) {
		h = (h << 4) + *name++;
		high = h & 0xf0000000;

		if( high )
			h ^= high >> 24;

		h &= ~high;
	}

	return h;
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
void get_symbol_from_table( struct vdso_symtab* symtab, const char* name ) {
	Elf64_Word  bucketnum = symtab->elf_hashtab[0];
	Elf64_Word* buckettab = &symtab->elf_hashtab[2];
	Elf64_Word* chaintab  = &symtab->elf_hashtab[2 + bucketnum];
	Elf64_Sym*  sym;

	for( Elf64_Word i = buckettab[ hash_elf_symbol(name) % bucketnum ] ; i != STN_UNDEF ; i = chaintab[i] ) {
		sym = &symtab->elf_symtab[i];
		printf( "vDSO symbol: %s\n", &symtab->elf_symstrtab[ sym->st_name ] );
		//if( !strcmp( name, &symtab->elf_symstrtab[ sym->st_name ] ) )
			//return sym;
	}
}


/// Get the address of a vDSO symbol
///
/// @param symbol The name of the symbol to retrieve
/// @return The vDSO address of the symbol or 0 if it can't be found
Elf64_Addr get_vDSO_symbol( const char* symbol ) {
//    Elf64_Sym*         vdso_symbol_ptr = NULL;
	struct vdso_symtab symtab;

   // Get vDSO base address
	void* vdso_base_addr;
	vdso_base_addr = (void *)getauxval( AT_SYSINFO_EHDR );
	if( !vdso_base_addr )
		goto err;
	
	printf( "vDSO base address: %p\n", vdso_base_addr );

	if( !vdso_get_symbol_table( vdso_base_addr, &symtab ) )
		goto err;

	//vdso_symbol_ptr = get_symbol_from_table( &symtab, symbol );
    get_symbol_from_table( &symtab, symbol );
    
    return 0;
// 	if( !vdso_symbol_ptr )
// 		goto err;
	
// 	return (Elf64_Addr) (vdso_base_addr + vdso_symbol_ptr->st_value);
	
err:
	fprintf( stderr, "Failed to find vDSO symbol %s\n", symbol );

	return 0;	
}

// print out the symbol table
void dump_vDSO ( void ) {
//    vdso_sgx_enter_enclave_t vdso_sgx_enter_enclave;
//    vdso_sgx_enter_enclave = (vdso_sgx_enter_enclave_t) get_vDSO_symbol( "__vdso_sgx_enter_enclave" );
    printf("Printing Symbol Table:\n");
    get_vDSO_symbol( "__vdso_sgx_enter_enclave" );

}