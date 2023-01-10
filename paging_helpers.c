/*
 * paging_helpers.c
 *
 *  Created on: Sep 30, 2022
 *      Author: HP
 */
#include "memory_manager.h"

/*[2.1] PAGE TABLE ENTRIES MANIPULATION */
inline void pt_set_page_permissions(uint32* page_directory, uint32 virtual_address, uint32 permissions_to_set, uint32 permissions_to_clear)
{
	uint32 * ptrPagePointer = NULL ;
	int eXiSt = get_page_table(page_directory,virtual_address,&ptrPagePointer);
	if(eXiSt == TABLE_IN_MEMORY){
		ptrPagePointer[PTX(virtual_address)] = ptrPagePointer[PTX(virtual_address)] &(~permissions_to_clear) ;
		ptrPagePointer[PTX(virtual_address)] = ptrPagePointer[PTX(virtual_address)] | (permissions_to_set);
		tlb_invalidate((void *)NULL,(void *)virtual_address);
	}
	else{
		panic("Invalid Virtual Address");
	}
}

inline int pt_get_page_permissions(uint32* page_directory, uint32 virtual_address )
{
	uint32 *ptr_page_table = NULL;
		get_page_table(page_directory, virtual_address, &ptr_page_table);
		if (ptr_page_table != NULL)
		{
		int val =
		ptr_page_table[PTX(virtual_address)] & (0x00000fff) ;
		return val;
		}
		else{
			return -1;
		}
}

inline void pt_clear_page_table_entry(uint32* page_directory, uint32 virtual_address)
{
	uint32 * p_table =NULL;
	get_page_table(page_directory,virtual_address,&p_table);
	if (p_table==NULL)
		panic("Invalid Virtual Address");

	 p_table[PTX(virtual_address)]=0;
	 tlb_invalidate((void *) NULL,(void *)virtual_address);
}

/***********************************************************************************************/

/*[2.2] ADDRESS CONVERTION*/
inline int virtual_to_physical(uint32* page_directory, uint32 virtual_address)
{
	uint32 directory_entry =page_directory[PDX(virtual_address)]&(0xffc00000);
	uint32*ptr_page_table =NULL;
	get_page_table(page_directory,virtual_address,&ptr_page_table);
	if(ptr_page_table!=NULL)
	{
		uint32 table_entry = ptr_page_table[PTX(virtual_address)]&(0x003ff000);
		int val=ptr_page_table[PTX(virtual_address)]&(0x00000fff);
		int res =ptr_page_table[PTX(virtual_address)]&(0xfffff000);
		return res;
	}
	else
	{
		return -1;
	}

}

/***********************************************************************************************/

/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/

///============================================================================================
/// Dealing with page directory entry flags

inline uint32 pd_is_table_used(uint32* page_directory, uint32 virtual_address)
{
	return ( (page_directory[PDX(virtual_address)] & PERM_USED) == PERM_USED ? 1 : 0);
}

inline void pd_set_table_unused(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] &= (~PERM_USED);
	tlb_invalidate((void *)NULL, (void *)virtual_address);
}

inline void pd_clear_page_dir_entry(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] = 0 ;
	tlbflush();
}
