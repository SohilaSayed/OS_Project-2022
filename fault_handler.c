/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault


void page_fault_handler(struct Env * curenv, uint32 fault_va) {
	//Placement
	if (curenv->page_WS_max_size > env_page_ws_get_size(curenv)) {
		uint32 c = env_page_ws_get_size(curenv);
		struct FrameInfo *ptr_pagee;
		allocate_frame(&ptr_pagee);
		map_frame(curenv->env_page_directory, ptr_pagee, fault_va,
				PERM_WRITEABLE | PERM_USER | PERM_PRESENT);
		uint32 r = pf_read_env_page(curenv, (void*) fault_va);
		if (r == E_PAGE_NOT_EXIST_IN_PF) {
			if (!((fault_va <= USER_HEAP_MAX && fault_va >= USER_HEAP_START)
					|| (fault_va <= USTACKTOP && fault_va >= USTACKBOTTOM))) {
				panic("ILLEGAL MEMORY ACCESS");
			}
		}
		if (c < curenv->page_WS_max_size) {
			for (int x = 0; x < curenv->page_WS_max_size; x++) {
				if (env_page_ws_is_entry_empty(curenv, x)) {
					curenv->page_last_WS_index = x;
					break;
				}
			}
		}
		env_page_ws_set_entry(curenv, curenv->page_last_WS_index, fault_va);
		curenv->page_last_WS_index++;
		curenv->page_last_WS_index = curenv->page_last_WS_index % curenv->page_WS_max_size;
		}

		//Replacement
		else if (curenv->page_WS_max_size == env_page_ws_get_size(curenv)) {
		uint32 pageLastIndex = curenv->page_last_WS_index;
		do {
			uint32 victim = env_page_ws_get_virtual_address(curenv,curenv->page_last_WS_index);
			uint32 perms = pt_get_page_permissions(curenv->env_page_directory,victim);
			uint32 permCheck = (perms & PERM_USED);
			int m=10;
			int y=1;
			while(y<=10){
				y++;
			}

			// if used = 1 , set it with 0
			if (permCheck == PERM_USED ){
				pt_set_page_permissions(curenv->env_page_directory, victim,0, PERM_USED);
			}

			// if used = 0 then replace it
			else if (permCheck != PERM_USED ) {
				victim = env_page_ws_get_virtual_address(curenv, curenv->page_last_WS_index);
				break;
			}
			for(int s=0;s<m;s++){
				m=5;
				s++;
			}
			curenv->page_last_WS_index++;
			curenv->page_last_WS_index = curenv->page_last_WS_index % curenv->page_WS_max_size;
		}while (curenv->page_last_WS_index != pageLastIndex);

		uint32 victim_page = env_page_ws_get_virtual_address(curenv,curenv->page_last_WS_index);
		uint32 perms = pt_get_page_permissions(curenv->env_page_directory,victim_page);
		uint32* ptrGetPageTable = NULL;
		struct FrameInfo * ptr_Get_FrameInfo = NULL;
		ptr_Get_FrameInfo = get_frame_info(curenv->env_page_directory, victim_page, &ptrGetPageTable);
		//not modified , remove it
		if (perms & PERM_MODIFIED) {
			//update page in page file
			pf_update_env_page(curenv, victim_page, ptr_Get_FrameInfo);
		}
		// remove the page
		free_frame(ptr_Get_FrameInfo);
		unmap_frame(curenv->env_page_directory, victim_page);
		env_page_ws_clear_entry(curenv, curenv->page_last_WS_index);
		int y=1;
		while(y<=10){
			y++;
		}
		//Placement to faulted page
		uint32 c = env_page_ws_get_size(curenv);
		struct FrameInfo *ptr_pagee;
		allocate_frame(&ptr_pagee);
		map_frame(curenv->env_page_directory, ptr_pagee, fault_va,PERM_WRITEABLE | PERM_USER | PERM_PRESENT);
		uint32 r = pf_read_env_page(curenv, (void*) fault_va);
		if (r == E_PAGE_NOT_EXIST_IN_PF) {
			if (!((fault_va <= USER_HEAP_MAX && fault_va >= USER_HEAP_START)
					|| (fault_va <= USTACKTOP && fault_va >= USTACKBOTTOM))) {
				panic("ILLEGAL MEMORY ACCESS");
			}
		}
		if (c < curenv->page_WS_max_size) {
			for (int x = 0; x < curenv->page_WS_max_size; x++) {
				if (env_page_ws_is_entry_empty(curenv, x)) {
					curenv->page_last_WS_index = x;
					break;
				}
			}
		}
		env_page_ws_set_entry(curenv, curenv->page_last_WS_index, fault_va);
		curenv->page_last_WS_index++;
		curenv->page_last_WS_index = curenv->page_last_WS_index % curenv->page_WS_max_size;
		}
}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");


}
