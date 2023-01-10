#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
		initialize_dyn_block_system();
		cprintf("DYNAMIC BLOCK SYSTEM IS INITIALIZED\n");
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//=================================
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//panic("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
		LIST_INIT(&AllocMemBlocksList);
		LIST_INIT(&FreeMemBlocksList);

	//[2] Dynamically allocate the array of MemBlockNodes at VA USER_DYN_BLKS_ARRAY
	//	  (remember to set MAX_MEM_BLOCK_CNT with the chosen size of the array)
		uint32 userHeapsize;
		uint32 size;
		MAX_MEM_BLOCK_CNT = (USER_HEAP_MAX - USER_HEAP_START)/PAGE_SIZE;
		MemBlockNodes = (void *) USER_DYN_BLKS_ARRAY;
		userHeapsize = USER_HEAP_MAX - USER_HEAP_START;
		size = sizeof(struct MemBlock);
		sys_allocate_chunk(USER_DYN_BLKS_ARRAY,ROUNDUP(size*MAX_MEM_BLOCK_CNT,PAGE_SIZE),PERM_WRITEABLE|PERM_USER|PERM_PRESENT);

	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
		initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);
		//cprintf("here 2 \n");

		struct MemBlock *element;
		element = LIST_FIRST(&AvailableMemBlocksList);
		if (element == NULL) {
			cprintf("element in fun is null \n");
		}
		element->size = userHeapsize ;
		element->sva = ((uint32) (USER_HEAP_START));
		LIST_REMOVE(&AvailableMemBlocksList, element);
	//[4] Insert a new MemBlock with the heap size into the FreeMemBlocksList

		LIST_INSERT_HEAD(&FreeMemBlocksList,element);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//==============================================================
	struct MemBlock *block;
	uint32 sZ = ROUNDUP(size, PAGE_SIZE);
	if (sys_isUHeapPlacementStrategyFIRSTFIT()) {
	   block = alloc_block_FF(sZ);
   }
	if(block != NULL){

		insert_sorted_allocList(block);
		return (void*)block->sva;
	}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	FROM main memory AND free pages from page file then switch back to the user again.
//
//	We can use sys_free_user_mem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls free_user_mem() in
//		"kern/mem/chunk_operations.c", then switch back to the user mode here
//	the free_user_mem function is empty, make sure to implement it.
void free(void* virtual_address)
{
	struct MemBlock *vaBlock = find_block(&AllocMemBlocksList,ROUNDDOWN((uint32)virtual_address, PAGE_SIZE));
	if(vaBlock != NULL){
		sys_free_user_mem((uint32)virtual_address,vaBlock->size);
		LIST_REMOVE(&AllocMemBlocksList,vaBlock);
		insert_sorted_with_merge_freeList(vaBlock);
	}

}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================

	struct MemBlock *block;
	uint32 s =ROUNDUP(size,PAGE_SIZE);
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
	   block = alloc_block_FF(s);
	}
	if(block!=NULL)
	{
		//LIST_INSERT_HEAD(&AllocMemBlocksList,block);
		int sh= sys_createSharedObject(sharedVarName,s,isWritable,(void*)block->sva);
		if(sh==E_NO_SHARE )
		{
		return NULL;
		}
		else if( sh==E_SHARED_MEM_EXISTS)
		{
			return NULL;
		}
		else if( sh==-1)
				{
					return NULL;
				}
		else
		{
			return (void*)block->sva;
		}
	}
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	struct MemBlock *block;
	uint32 sharedSize = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(sharedSize == E_SHARED_MEM_NOT_EXISTS){
		return NULL;
	}
	else{
		struct MemBlock *block;
		if(sys_isUHeapPlacementStrategyFIRSTFIT())
		{
			uint32 sizeNew = ROUNDUP(sharedSize,PAGE_SIZE);
		    block = alloc_block_FF(sizeNew);
		}
		if(block != NULL){

			if(sys_getSharedObject(ownerEnvID,sharedVarName,(void*)block->sva)!= E_SHARED_MEM_NOT_EXISTS)
				return (void*)block->sva;
		}
	}
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// [USER HEAP - USER SIDE] realloc
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
//	struct MemBlock *vaBlock = find_block(&AllocMemBlocksList,(uint32)virtual_address);
//	if(vaBlock !=NULL)
//	{
//
//		sys_freeSharedObject((int32) id,virtual_address);
//		LIST_REMOVE(&AllocMemBlocksList,vaBlock);
//		insert_sorted_with_merge_freeList(vaBlock);
//
//	}
		panic("YYY");
}




//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");
}
