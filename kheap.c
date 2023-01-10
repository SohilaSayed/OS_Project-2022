#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//

void initialize_dyn_block_system() {

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	LIST_INIT(&AllocMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);

	uint32 H_size;
	uint32 size;
#if STATIC_MEMBLOCK_ALLOC
	//DO NOTHING
#else
	/*[2] Dynamically allocate the array of MemBlockNodes
	 * 	remember to:
	 * 		1. set MAX_MEM_BLOCK_CNT with the chosen size of the array
	 * 		2. allocation should be aligned on PAGE boundary
	 * 	HINT: can use alloc_chunk(...) function
	 */
	MAX_MEM_BLOCK_CNT = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE;

	MemBlockNodes = (void *) KERNEL_HEAP_START;
	H_size = KERNEL_HEAP_MAX - KERNEL_HEAP_START;

	size = sizeof(struct MemBlock);
	allocate_chunk(ptr_page_directory, KERNEL_HEAP_START,ROUNDUP(size*MAX_MEM_BLOCK_CNT,PAGE_SIZE), PERM_WRITEABLE);

	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);

	struct MemBlock *element;
	element = LIST_FIRST(&AvailableMemBlocksList);

	if (element == NULL) {
		cprintf("element in fun is null \n");
	}
	element->size = H_size - ROUNDUP((size * MAX_MEM_BLOCK_CNT), PAGE_SIZE);

	element->sva = ((uint32) (KERNEL_HEAP_START+ ROUNDUP((size * MAX_MEM_BLOCK_CNT), PAGE_SIZE)));

	LIST_REMOVE(&AvailableMemBlocksList, element);

	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList
	LIST_INSERT_HEAD(&FreeMemBlocksList, element);

#endif
}




void* kmalloc(unsigned int size) {
	uint32 s = ROUNDUP(size, PAGE_SIZE);
	struct MemBlock *block;
	int ret;
	if (isKHeapPlacementStrategyFIRSTFIT()) {
	   block = alloc_block_FF(s);
   }
	if (isKHeapPlacementStrategyBESTFIT()) {
		block = alloc_block_BF(s);
   }
	if (isKHeapPlacementStrategyNEXTFIT()) {
			block = alloc_block_NF(s);

  }
	if (block != NULL) {
		ret = allocate_chunk(ptr_page_directory, block->sva, s, PERM_WRITEABLE);
		uint32 add = block->sva;
		if(ret==0)
		{
			insert_sorted_allocList(block);
			return (void*)block->sva;
		}
	}
	return NULL;
}


void kfree(void* virtual_address)
{
	struct MemBlock *temp = find_block(&AllocMemBlocksList,(uint32)virtual_address);
	if(temp != NULL){
		LIST_REMOVE(&AllocMemBlocksList,temp);
		int m=10;
		int y=1;
		while(y<=10){
			y++;
		}
		for(int s=0;s<m;s++){
			m=5;
			s++;
		}
		for(uint32 i = 0 ; i< temp->size; i+=PAGE_SIZE)
		{
			uint32 sizeForUnmap = (uint32)virtual_address+i;
			unmap_frame(ptr_page_directory,sizeForUnmap);
		}
		insert_sorted_with_merge_freeList(temp);
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address) {
	struct FrameInfo *frame = NULL;
	if(frame ==  NULL){
		frame = to_frame_info(physical_address);
		return frame->va ;
	}
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address) {
	uint32* ptr_page = NULL;
	int ret = get_page_table(ptr_page_directory, virtual_address, &ptr_page);

	if (ret == TABLE_NOT_EXIST || ptr_page == NULL) {
		return -1;
	}

	return ptr_page[PTX(virtual_address)] & 0xFFFFF000;
}

void kfreeall() {
	panic("Not implemented!");

}

void kshrink(uint32 newSize) {
	panic("Not implemented!");
}

void kexpand(uint32 newSize) {
	panic("Not implemented!");
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	panic("krealloc() is not implemented yet...!!");
}
