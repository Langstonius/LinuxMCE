/*
 * Copyright 1998-2005 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2005 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define __NO_VERSION__
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <linux/pci.h>

#include "via_ds.h"
extern unsigned int VIA_DEBUG;

set_t *setInit(void)
{
    int i;
    set_t *set;
    set = (set_t *)MALLOC(sizeof(set_t));
    for (i = 0; i < SET_SIZE; i++) {
	set->list[i].free_next = i+1;    
	set->list[i].alloc_next = -1;
    }    
    set->list[SET_SIZE-1].free_next = -1;
    set->free = 0;
    set->alloc = -1;
    set->trace = -1;
    return set;
}

int setAdd(set_t *set, ITEM_TYPE item)
{
    int free = set->free;
    if (free != -1) {
	set->list[free].val = item;
	set->free = set->list[free].free_next;
    }
    else {
	return 0;
    }
    set->list[free].alloc_next = set->alloc;
    set->alloc = free;  
    set->list[free].free_next = -1;  
    return 1;
}

int setDel(set_t *set, ITEM_TYPE item)
{
    int alloc = set->alloc;
    int prev = -1;  
  
    while (alloc != -1) {
	if (set->list[alloc].val == item) {
    	    if (prev != -1)      
    		set->list[prev].alloc_next = set->list[alloc].alloc_next; 
	    else
    		set->alloc = set->list[alloc].alloc_next;
    	    break;
	}
	prev = alloc;
	alloc = set->list[alloc].alloc_next;      
    }

    if (alloc == -1)
	return 0;
  
    set->list[alloc].free_next = set->free;
    set->free = alloc;
    set->list[alloc].alloc_next = -1;   

    return 1;
}

/* setFirst -> setAdd -> setNext is wrong */

int setFirst(set_t *set, ITEM_TYPE *item)
{
    if (set->alloc == -1)
	return 0;

    *item = set->list[set->alloc].val;
    set->trace = set->list[set->alloc].alloc_next; 

	
    return 1;
}

int setNext(set_t *set, ITEM_TYPE *item)
{
    if (set->trace == -1)
	return 0;
  
    *item = set->list[set->trace].val;
    set->trace = set->list[set->trace].alloc_next;      

    return 1;
}

int setDestroy(set_t *set)
{
    FREE(set);

    return 1;
}

#define ISFREE(bptr) ((bptr)->free)

#define PRINTF(fmt, arg...) do{}while(0)
#define fprintf(fmt, arg...) do{}while(0)

static void *calloc(size_t nmemb, size_t size)
{
    void *addr;
    addr = kmalloc(nmemb*size, GFP_KERNEL);
    memset(addr, 0, nmemb*size);
    return addr;
}
#define free(n) kfree(n)
           
void mmDumpMemInfo( memHeap_t *heap )
{
    TMemBlock *p;

    PRINTF ("Memory heap %p:\n", heap);
    
    if (heap == 0)
	PRINTF ("  heap == 0\n");
    else {
	p = (TMemBlock *)heap;
	
	while (p) {
    	    PRINTF ("  Offset:%08x, Size:%08x, %c%c\n",p->ofs,p->size,
		p->free ? '.':'U',
	        p->reserved ? 'R':'.');
    	    p = p->next;
	}
    }
    
    PRINTF ("End of memory blocks\n");
}

memHeap_t *mmInit(int ofs,
		  int size)
{
    PMemBlock blocks;
  
    if (size <= 0)
	return 0;

      
    blocks = (TMemBlock *)calloc(1,sizeof(TMemBlock));
   
    if (blocks) {
         blocks->ofs = ofs;
        blocks->size = size;
        blocks->free = 1;
        return (memHeap_t *)blocks;
    } else
	 return 0;
}

memHeap_t *mmAddRange(memHeap_t *heap,
		       int ofs,
		       int size)
{
    PMemBlock blocks;
    blocks = (TMemBlock *) calloc(2,sizeof(TMemBlock));
    
    if (blocks) {
        blocks[0].size = size;
        blocks[0].free = 1;
        blocks[0].ofs = ofs;
        blocks[0].next = &blocks[1];

        /* Discontinuity - stops JoinBlock from trying to join non-adjacent
        * ranges.
        */
        blocks[1].size = 0;
        blocks[1].free = 0;
        blocks[1].ofs = ofs+size;
        blocks[1].next = (PMemBlock) heap;      
        return (memHeap_t *)blocks;
    } 
else
      return heap;
}

static TMemBlock* SliceBlock(TMemBlock *p, 
			     int startofs, int size, 
			     int reserved, int alignment)
{
    TMemBlock *newblock;

    /* break left */
    if (startofs > p->ofs) {
	newblock = (TMemBlock*)calloc(1,sizeof(TMemBlock));
	newblock->ofs = startofs;
	newblock->size = p->size - (startofs - p->ofs);
	newblock->free = 1;
	newblock->next = p->next;
	p->size -= newblock->size;
	p->next = newblock;
	p = newblock;
    }

    /* break right */
    if (size < p->size) {
	newblock = (TMemBlock*) calloc(1,sizeof(TMemBlock));
	newblock->ofs = startofs + size;
	newblock->size = p->size - size;
	newblock->free = 1;
	newblock->next = p->next;
	p->size = size;
	p->next = newblock;
     }

    /* p = middle block */
    p->align = alignment;
    p->free = 0;
    p->reserved = reserved;
    return p;
}

PMemBlock mmAllocMem(memHeap_t *heap, int size, int align2, int startSearch)
{
    int mask,startofs,endofs;
    TMemBlock *p;

    if (!heap || align2 < 0 || size <= 0)
	return NULL;
    
    mask = (1 << align2)-1;
    startofs = 0;
    p = (TMemBlock *)heap;
    
    while (p) {
	if (ISFREE(p)) {
        startofs = (p->ofs + mask) & ~mask;
    	
	if ( startofs < startSearch )
      	    startofs = startSearch;
        
	endofs = startofs+size;
        
	if (endofs <= (p->ofs+p->size))
	    break;
	}
	
	p = p->next;
    }
    
    if (!p)
	return NULL;
    
    p = SliceBlock(p,startofs,size,0,mask+1);
    p->heap = heap;
    
    return p;
}

static __inline__ int Join2Blocks(TMemBlock *p)
{
    if (p->free && p->next && p->next->free) {
	TMemBlock *q = p->next;
	p->size += q->size;
	p->next = q->next;
	free(q);
	
	return 1;
    }
    
    return 0;
}

int mmFreeMem(PMemBlock b)
{
    TMemBlock *p,*prev;

    if (!b)
	return 0;
    
    if (!b->heap) {
        fprintf(stderr, "no heap\n");
        
	return -1;
    }
    
    p = b->heap;
    prev = NULL;
    
    while (p && p != b) {
	prev = p;
	p = p->next;
    }
    
    if (!p || p->free || p->reserved) {
        if (!p)
	    fprintf(stderr, "block not found in heap\n");
        else if (p->free)
	    fprintf(stderr, "block already free\n");
        else
	    fprintf(stderr, "block is reserved\n");
	
	
	return -1;
    }
    
    p->free = 1;
    Join2Blocks(p);
    
    if (prev)
	Join2Blocks(prev);
  
    return 0;
}

int mmReserveMem(memHeap_t *heap, int offset,int size)
{
    int endofs;
    TMemBlock *p;

    if (!heap || size <= 0)
	return -1;
    endofs = offset+size;
    p = (TMemBlock *)heap;
    
    while (p && p->ofs <= offset) {
	if (ISFREE(p) && endofs <= (p->ofs+p->size)) {
    	    SliceBlock(p,offset,size,1,1);
    	    return 0;
	}
	p = p->next;
    }
    return -1;
}

int mmFreeReserved(memHeap_t *heap, int offset)
{
    TMemBlock *p,*prev;

    if (!heap)
	return -1;
  
    p = (TMemBlock *)heap;
    prev = NULL;
    
    while (p && p->ofs != offset) {
	prev = p;
	p = p->next;
    }
    
    if (!p || !p->reserved)
	return -1;
    p->free = 1;
    p->reserved = 0;
    Join2Blocks(p);
    
    if (prev)
	Join2Blocks(prev);
    
    return 0;
}

void mmDestroy(memHeap_t *heap)
{
    TMemBlock *p,*q;

    if (!heap)
	return;
    p = (TMemBlock *)heap;
    
    while (p) {
	q = p->next;
	free(p);
	p = q;
    }
}
