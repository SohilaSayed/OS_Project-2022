/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// PRINT MEM BLOCK LISTS:
//===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk ;
	struct MemBlock* lastBlk = NULL ;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1 ;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n") ;

	lastBlk = NULL ;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1 ;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n") ;
	cprintf("\n=========================================\n");

}

//********************************************************************************//
//********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//===============================
// [1] INITIALIZE AVAILABLE LIST:
//===============================
void initialize_MemBlocksList(uint32 numOfBlocks)
{
	LIST_INIT(&AvailableMemBlocksList);

	for(int block = 0 ; block < numOfBlocks ; block++){
	LIST_INSERT_HEAD(&AvailableMemBlocksList,&MemBlockNodes[block]);}

}
//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock *find_block(struct MemBlock_List *blockList, uint32 va)
{
	struct MemBlock *arrow = NULL;
	LIST_FOREACH(arrow,blockList){
		if(va == arrow->sva){
			return arrow;
		}
	}
	return NULL;

}

//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock *blockToInsert)
{

	if(LIST_SIZE(&(AllocMemBlocksList)) <= 0 || blockToInsert->sva < LIST_FIRST(&AllocMemBlocksList)->sva){
		LIST_INSERT_HEAD(&AllocMemBlocksList, blockToInsert);
	}
	else if(blockToInsert->sva > LIST_LAST(&AllocMemBlocksList)->sva){
		LIST_INSERT_TAIL(&AllocMemBlocksList, blockToInsert);
	}
	else{
		struct MemBlock *block_insert;
		uint32 Ptrnext = 0;
		LIST_FOREACH(block_insert, &AllocMemBlocksList){
			if(block_insert->sva < blockToInsert->sva){
				if(LIST_NEXT(block_insert) != NULL){
					Ptrnext = LIST_NEXT(block_insert)->sva;
					if(LIST_NEXT(block_insert)->sva > blockToInsert->sva){
						LIST_INSERT_AFTER(&(AllocMemBlocksList), block_insert, blockToInsert);
						break;
					}
				}
			}
		}
	}
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
struct MemBlock *alloc_block_FF(uint32 size)
{
	struct MemBlock* freeBlock = FreeMemBlocksList.lh_first;
		while(freeBlock != NULL){

			if(freeBlock->size == size){
				LIST_REMOVE(&FreeMemBlocksList, freeBlock);
				return freeBlock;
			}else if(freeBlock->size > size){
			struct MemBlock* temp = LIST_FIRST(&AvailableMemBlocksList);

				temp->size = size;
				temp->sva = freeBlock->sva;

				freeBlock->size = freeBlock->size - size;
				freeBlock->sva =  freeBlock->sva + size;
				LIST_REMOVE(&AvailableMemBlocksList, temp);
				return temp;
			}

			freeBlock = freeBlock->prev_next_info.le_next;
		}
		return NULL;

}

//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock *alloc_block_BF(uint32 size)
{
	struct MemBlock* freeBlock, *bestBS = FreeMemBlocksList.lh_last;
		bool found = 0, firstTime = 0;
		LIST_FOREACH(freeBlock,&FreeMemBlocksList)
		{
			if (freeBlock->size == size)
			{
				struct  MemBlock* temp  = freeBlock;
				LIST_REMOVE(&FreeMemBlocksList , temp);
				return freeBlock;
			}
			if (freeBlock->size > size)
			{
				if(!firstTime)
				{
					bestBS = freeBlock;
					firstTime = 1;
				}

				if(freeBlock->size < bestBS -> size)
				{
					bestBS = freeBlock;
					found = 1;
				}
			}
		}


		if(found || (!found && firstTime))
		{
			struct  MemBlock* newBlock = AvailableMemBlocksList.lh_first ;
			newBlock->size = size;
			newBlock->sva = bestBS->sva;
			LIST_REMOVE(&AvailableMemBlocksList ,newBlock);
			bestBS->size -= size;
			bestBS->sva += size;
			return newBlock;
		}


		return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
struct MemBlock* current = NULL;
struct MemBlock *alloc_block_NF(uint32 size)
{
	if(current == NULL){
			current = FreeMemBlocksList.lh_first;
		}

		struct MemBlock* freeBlock = current;
		int cnt = 0;
		while(LIST_NEXT(freeBlock) != current){
			if(freeBlock->size == size){
				current = LIST_NEXT(freeBlock);
				LIST_REMOVE(&FreeMemBlocksList, freeBlock);
				cprintf("current: %x, size: %d\n", freeBlock->sva , size);
				return freeBlock;
			}else if(freeBlock->size > size){
				struct MemBlock* temp = LIST_FIRST(&AvailableMemBlocksList);

				temp->size = size;
				temp->sva = freeBlock->sva;

				freeBlock->size = freeBlock->size - size;
				freeBlock->sva =  freeBlock->sva + size;
				current = freeBlock;
				cprintf("current: %x, size: %d\n", temp->sva , size);
				LIST_REMOVE(&AvailableMemBlocksList, temp);
				return temp;
			}


			freeBlock = freeBlock->prev_next_info.le_next;

			if(freeBlock == NULL && cnt == 0){
				cnt++;
				freeBlock = FreeMemBlocksList.lh_first;
			}
			else if(freeBlock == NULL && cnt == 1){
				break;
			}
		}
		return NULL;

}

//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================
void insert_sorted_with_merge_freeList(struct MemBlock *blockToInsert)
{
	if(LIST_SIZE(&FreeMemBlocksList) == 0){
		LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
		return;
	}
	struct MemBlock *findBlock = NULL;
	uint32 endBI = blockToInsert -> sva + blockToInsert -> size;
	LIST_FOREACH(findBlock, &FreeMemBlocksList){
		uint32 endFB = findBlock -> sva + findBlock -> size;
		if(endBI == findBlock -> sva){
			findBlock -> size = findBlock -> size + blockToInsert -> size;
			findBlock -> sva = blockToInsert -> sva;
			blockToInsert -> sva = blockToInsert -> size = 0;
			LIST_INSERT_TAIL(&AvailableMemBlocksList, blockToInsert);
			return;
		}

		if(endFB == blockToInsert -> sva){
			if(findBlock == LIST_LAST(&FreeMemBlocksList)){
				findBlock -> size = findBlock -> size + blockToInsert -> size;
				blockToInsert -> sva = blockToInsert -> size = 0;
				LIST_INSERT_TAIL(&AvailableMemBlocksList, blockToInsert);
				return;
			}
			struct MemBlock *nextBlock = LIST_NEXT(findBlock);
			if(nextBlock -> sva == endBI){
				findBlock -> size = findBlock -> size + blockToInsert -> size + nextBlock -> size;
				blockToInsert -> sva = blockToInsert -> size = 0;
				nextBlock -> sva = nextBlock -> size = 0;
				LIST_INSERT_TAIL(&AvailableMemBlocksList, blockToInsert);
				LIST_REMOVE(&FreeMemBlocksList, nextBlock);
				LIST_INSERT_TAIL(&AvailableMemBlocksList, nextBlock);
				return;
			}
			findBlock -> size = findBlock -> size + blockToInsert -> size;
			blockToInsert -> sva = blockToInsert -> size = 0;
			LIST_INSERT_TAIL(&AvailableMemBlocksList, blockToInsert);
			return;
		}

		if(findBlock -> sva > blockToInsert -> sva){
			LIST_INSERT_BEFORE(&FreeMemBlocksList, findBlock, blockToInsert);
			return;
		}
	}
	LIST_INSERT_TAIL(&FreeMemBlocksList, blockToInsert);
}














