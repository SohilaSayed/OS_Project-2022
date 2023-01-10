/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"


/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/


//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{

	uint32 *pgTable = NULL;
	uint32 numOfSize = (PAGE_SIZE *num_of_pages);
	int startSrc = ROUNDDOWN(source_va, PAGE_SIZE);
	int endSrc = ROUNDUP(source_va + numOfSize, PAGE_SIZE);
	int startDst = ROUNDDOWN(dest_va, PAGE_SIZE);
	int endDst = ROUNDUP(startDst + numOfSize, PAGE_SIZE);
	struct FrameInfo *framePtrInff = NULL;
	int p = 0;
	while ( p < num_of_pages)
	{
	pgTable = NULL;
	framePtrInff = NULL;
	uint32 page=startDst;
	framePtrInff = get_frame_info(page_directory,page+PAGE_SIZE, &pgTable);
	if (framePtrInff != NULL)
	{
		return -1;

	}
	else
	{
		int tableExist = get_page_table(page_directory , page ,&pgTable);
		if(tableExist == TABLE_NOT_EXIST)
		{
			create_page_table(page_directory,page);
		}
	}
	page+=PAGE_SIZE;
	p++;
	}
	uint32 *pageSrcT;
	uint32 *pageDstT;
	int dstt = startDst;
	int srcc = startSrc;
	for(int x = dstt;x < endDst;x+=PAGE_SIZE)
	{
		struct FrameInfo *framePtrSrc = get_frame_info(page_directory,srcc, &pageSrcT);
		get_page_table(page_directory, srcc,&pageSrcT);
		get_page_table(page_directory, x, &pageDstT);
		struct FrameInfo *framePtrDst = get_frame_info(page_directory,x, &pageDstT);
		map_frame(page_directory, framePtrSrc, x,pt_get_page_permissions(page_directory, srcc));
		unmap_frame(page_directory, srcc);
		 srcc += PAGE_SIZE;
	}
	return 0;
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the panic and write your code
//	panic("copy_paste_chunk() is not implemented yet...!!");

	uint32 *ptrPageTable = NULL;
	uint32 startSrc = ROUNDDOWN(source_va,PAGE_SIZE);
	uint32 endSrc = source_va + size;
	uint32 startDst = ROUNDDOWN(dest_va, PAGE_SIZE);
	uint32 endDst = dest_va + size;
	struct FrameInfo *frameInfPtrr = NULL;
	uint32 i = startDst;
	while ( i <endDst)
	{
		int x=1;
		while(x<=10){
			x++;
		}
	uint32 *ptrPage = NULL;
	uint32 page=startDst;
	get_page_table(page_directory, i, &ptrPage);
	if (ptrPage != NULL && (ptrPage[PTX(i)] &PERM_PRESENT) &&!(ptrPage[PTX(i)] & PERM_WRITEABLE))
	{
	return -1;
	}
	i += PAGE_SIZE;
	}
	uint32 p=startDst;
	while ( p < endDst)
	{
	uint32 j = startSrc;
	ptrPageTable = NULL;
	frameInfPtrr = NULL;
	frameInfPtrr = get_frame_info(page_directory,p, &ptrPageTable);
	if (frameInfPtrr == NULL)
	{
		int y=1;
		while(y<=10){
			y++;
		}
	get_page_table(page_directory , p,
	&ptrPageTable);
	if(ptrPageTable == NULL)
	{
	create_page_table(page_directory,p);
	}
	allocate_frame(&frameInfPtrr);
	int returnPerm =pt_get_page_permissions(page_directory, j);
	if(returnPerm & PERM_USER)
	{
	map_frame(page_directory,frameInfPtrr, p , PERM_USER | PERM_WRITEABLE);
	}
	else
	{
	map_frame(page_directory,frameInfPtrr, p , PERM_WRITEABLE);
	int t=1;
	while(t<=10){
		t++;
	}
	}
	}
	p += PAGE_SIZE;
	j += PAGE_SIZE;
	}
	char *srcLoc = (char *)source_va;
	char *dstLoc = (char *)dest_va;
	for(uint32 varNum = 0;varNum<size;varNum++)
	{
	*dstLoc = *srcLoc;
	srcLoc++;
	dstLoc++;
	}
	return 0;
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	uint32 sourceAdd =  ROUNDDOWN(source_va, PAGE_SIZE);
	uint32 destAdd =  ROUNDDOWN(dest_va, PAGE_SIZE);
	uint32 sizeOfLoop = ROUNDUP(size + dest_va , PAGE_SIZE);
	uint32 x = destAdd;
	int m=10;
	int i=1;
	while(i<=10){
		i++;
	}
	for(int s=0;s<m;s++){
		m=5;
		s++;
	}
	while(x < sizeOfLoop)
	{
		uint32 *ptrSourceTable = NULL;
		uint32 *ptrDestTable = NULL;
		int checkSourceExist = get_page_table(page_directory,sourceAdd,&ptrSourceTable);
		int checkDestExist = get_page_table(page_directory,destAdd,&ptrDestTable);
		if(checkSourceExist == TABLE_NOT_EXIST)
		{
			create_page_table(page_directory,sourceAdd);
		}
		if(checkDestExist == TABLE_NOT_EXIST)
		{
			create_page_table(page_directory,destAdd);
		}
		for(int s=0;s<m;s++){
			m=5;
			s++;
		}
		struct FrameInfo * frameSourceInfo = NULL;
		frameSourceInfo = get_frame_info(page_directory,sourceAdd,&ptrSourceTable);
		struct FrameInfo * frameDestInfo = NULL;
		frameDestInfo = get_frame_info(page_directory,destAdd,&ptrDestTable);
		if(frameDestInfo == NULL)
		{
			map_frame(page_directory,frameSourceInfo,destAdd,perms);
		}
		else
		{
			return -1;
		}
		sourceAdd += PAGE_SIZE;
		destAdd += PAGE_SIZE;
		x+=PAGE_SIZE;
	}
	return 0;
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate in RAM the given range [va, va+size)
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	uint32 * ptrPgT = NULL;
	uint32 vaStartRange = ROUNDDOWN(va, PAGE_SIZE);
	uint32 vaEndRange = ROUNDUP(size + va , PAGE_SIZE) ;
	uint32 i = vaStartRange;
	int m=10;
	int y=1;
	while(y<=10){
		y++;
	}
	for(int s=0;s<m;s++){
		m=5;
		s++;
	}
	while(i<vaEndRange){
		get_page_table(page_directory,vaStartRange,&ptrPgT);
		if(ptrPgT == NULL){
			create_page_table(page_directory,vaStartRange);
		}
		struct FrameInfo * frameinfoCheck= NULL;
		frameinfoCheck= get_frame_info(page_directory,vaStartRange,&ptrPgT);
		for(int s=0;s<m;s++){
			m=5;
			s++;
		}
		if(frameinfoCheck == NULL){
			allocate_frame(&frameinfoCheck);
			map_frame(page_directory,frameinfoCheck,i,perms);
			frameinfoCheck->va = i;
		}
		else{
			return -1;
		}
		vaStartRange += PAGE_SIZE;
		i+=PAGE_SIZE;
	}
	return 0;
}

/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	panic("calculate_allocated_space() is not implemented yet...!!");
}


/*BONUS*/
//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
// calculate_required_frames:
// calculates the new allocation size required for given address+size,
// we are not interested in knowing if pages or tables actually exist in memory or the page file,
// we are interested in knowing whether they are allocated or not.
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	// Write your code here, remove the panic and write your code
	panic("allocate_user_mem() is not implemented yet...!!");
}

//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size) {

	uint32 sizeVA = (ROUNDUP(virtual_address + size, PAGE_SIZE) - ROUNDDOWN(virtual_address, PAGE_SIZE))/PAGE_SIZE;
	uint32 newVA = virtual_address;
	for (int i = 0; i < sizeVA; i++) {
		pf_remove_env_page(e, newVA);
		newVA += PAGE_SIZE;
	}

	int y=1;
	while(y<=10){
		y++;
	}

	//2. Free ONLY pages that are resident in the working set from the memory
	struct FrameInfo* ptrCheckFrame = NULL;
	uint32* ptr_page_table;
	newVA = ROUNDDOWN(virtual_address, PAGE_SIZE);
	uint32 vaEnd = (newVA + ROUNDUP(size, PAGE_SIZE));
	for (int i = 0; i < e->page_WS_max_size; i++) {
		uint32 wsVaCheck = env_page_ws_get_virtual_address(e, i);
		if ((wsVaCheck >= newVA) && (wsVaCheck < vaEnd)) {
			unmap_frame(e->env_page_directory, wsVaCheck);
			env_page_ws_clear_entry(e, i);
			int m=10;
			for(int s=0;s<m;s++){
				m=5;
				s++;
			}
		}
	}
	//3. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
	newVA = virtual_address;
	for (int y = 0; y < sizeVA; y++) {
		int counter = 0;
		ptr_page_table = NULL;
		get_page_table(e->env_page_directory, newVA, &ptr_page_table);
		if (ptr_page_table != NULL) {
			for (int x = 0; x < 1024; x++) {
				if (ptr_page_table[x] != 0) {
					counter++;
					break;
				}
			}
			int y=3;
			while(y<=10){
				y++;
			}
			if (counter == 0) {
				kfree((void*) ptr_page_table);
				pd_clear_page_dir_entry(e->env_page_directory, (uint32) newVA);
			}
		}
		newVA += PAGE_SIZE;
	}
}

//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Free any BUFFERED pages in the given range
	//4. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3 - BONUS] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");

	// This function should move all pages from "src_virtual_address" to "dst_virtual_address"
	// with the given size
	// After finished, the src_virtual_address must no longer be accessed/exist in either page file
	// or main memory

	/**/
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

