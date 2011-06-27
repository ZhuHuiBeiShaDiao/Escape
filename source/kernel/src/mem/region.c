/**
 * $Id$
 * Copyright (C) 2008 - 2009 Nils Asmussen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <sys/common.h>
#include <sys/mem/pmem.h>
#include <sys/mem/cache.h>
#include <sys/mem/region.h>
#include <sys/mem/swapmap.h>
#include <sys/video.h>
#include <string.h>
#include <assert.h>

/**
 * The region-module implements the abstraction 'region' which is simply a group of pages that
 * have common properties. So for example 'text' is a group, 'read-only-data', 'data', 'stack'
 * and so on. A region has no address and can be shared (if RF_SHAREABLE is set) by multiple
 * processes (and of course the virtual address to which it is mapped may be different).
 * A region has some flags that specify what operations are allowed and it has an array of
 * page-flags (flags for each page). So that each page can have different flags like copy-on-write,
 * swapped, demand-load or demand-zero. Additionally the page-flags store the swap-block if a
 * page is swapped out.
 */

static void reg_sprintfFlags(sStringBuffer *buf,const sRegion *reg);

sRegion *reg_create(const sBinDesc *bin,off_t binOffset,size_t bCount,size_t lCount,
		ulong pgFlags,ulong flags) {
	size_t i,pageCount;
	sRegion *reg;
	assert(pgFlags == PF_DEMANDLOAD || pgFlags == 0);

	reg = (sRegion*)cache_alloc(sizeof(sRegion));
	if(reg == NULL)
		return NULL;
	reg->procs = sll_create();
	if(reg->procs == NULL)
		goto errReg;
	if(bin != NULL && bin->ino) {
		reg->binary.ino = bin->ino;
		reg->binary.dev = bin->dev;
		reg->binary.modifytime = bin->modifytime;
		reg->binOffset = binOffset;
	}
	else {
		reg->binary.ino = 0;
		reg->binary.dev = 0;
		reg->binary.modifytime = 0;
		reg->binOffset = 0;
	}
	reg->flags = flags;
	reg->byteCount = bCount;
	reg->loadCount = lCount;
	reg->timestamp = 0;
	pageCount = BYTES_2_PAGES(bCount);
	/* if we have no pages, create the page-array with 1; using 0 will fail and this may actually
	 * happen for data-regions of zero-size. We want to be able to increase their size, so they
	 * must exist. */
	reg->pfSize = MAX(1,pageCount);
	reg->pageFlags = (ulong*)cache_alloc(reg->pfSize * sizeof(ulong));
	if(reg->pageFlags == NULL)
		goto errPDirs;
	for(i = 0; i < pageCount; i++)
		reg->pageFlags[i] = pgFlags;
	return reg;

errPDirs:
	sll_destroy(reg->procs,false);
errReg:
	cache_free(reg);
	return NULL;
}

void reg_destroy(sRegion *reg) {
	size_t i,pcount = BYTES_2_PAGES(reg->byteCount);
	assert(reg != NULL);
	/* first free the swapped out blocks */
	for(i = 0; i < pcount; i++) {
		if(reg->pageFlags[i] & PF_SWAPPED)
			swmap_free(reg_getSwapBlock(reg,i));
	}
	cache_free(reg->pageFlags);
	sll_destroy(reg->procs,false);
	cache_free(reg);
}

size_t reg_presentPageCount(const sRegion *reg) {
	size_t i,c = 0,pcount = BYTES_2_PAGES(reg->byteCount);
	assert(reg != NULL);
	for(i = 0; i < pcount; i++) {
		if((reg->pageFlags[i] & (PF_DEMANDLOAD | PF_LOADINPROGRESS)) == 0)
			c++;
	}
	return c;
}

ulong reg_getSwapBlock(const sRegion *reg,size_t pageIndex) {
	assert(reg->pageFlags[pageIndex] & PF_SWAPPED);
	return reg->pageFlags[pageIndex] >> PF_BITCOUNT;
}

void reg_setSwapBlock(sRegion *reg,size_t pageIndex,ulong swapBlock) {
	reg->pageFlags[pageIndex] &= (1 << PF_BITCOUNT) - 1;
	reg->pageFlags[pageIndex] |= swapBlock << PF_BITCOUNT;
}

size_t reg_refCount(const sRegion *reg) {
	assert(reg != NULL);
	return sll_length(reg->procs);
}

bool reg_addTo(sRegion *reg,const void *p) {
	assert(reg != NULL);
	assert(sll_length(reg->procs) == 0 || (reg->flags & RF_SHAREABLE));
	return sll_append(reg->procs,(void*)p);
}

bool reg_remFrom(sRegion *reg,const void *p) {
	assert(reg != NULL);
	return sll_removeFirst(reg->procs,(void*)p);
}

bool reg_grow(sRegion *reg,ssize_t amount) {
	size_t count = BYTES_2_PAGES(reg->byteCount);
	assert(reg != NULL && (reg->flags & RF_GROWABLE));
	if(amount > 0) {
		ssize_t i;
		ulong *pf = (ulong*)cache_realloc(reg->pageFlags,(reg->pfSize + amount) * sizeof(ulong));
		if(pf == NULL)
			return false;
		reg->pfSize += amount;
		if(reg->flags & RF_GROWS_DOWN) {
			memmove(pf + amount,pf,count * sizeof(ulong));
			for(i = 0; i < amount; i++)
				pf[i] = 0;
		}
		else {
			for(i = 0; i < amount; i++)
				pf[i + count] = 0;
		}
		reg->pageFlags = pf;
		reg->byteCount += amount * PAGE_SIZE;
	}
	else {
		size_t i;
		if(reg->byteCount < (size_t)-amount * PAGE_SIZE)
			return false;
		/* free swapped pages */
		for(i = count + amount; i < count; i++) {
			if(reg->pageFlags[i] & PF_SWAPPED)
				swmap_free(reg_getSwapBlock(reg,i));
		}
		if(reg->flags & RF_GROWS_DOWN)
			memmove(reg->pageFlags,reg->pageFlags + -amount,(count + amount) * sizeof(ulong));
		reg->byteCount -= -amount * PAGE_SIZE;
	}
	return true;
}

sRegion *reg_clone(const void *p,const sRegion *reg) {
	sRegion *clone;
	assert(reg != NULL && !(reg->flags & RF_SHAREABLE));
	clone = reg_create(&reg->binary,reg->binOffset,reg->byteCount,reg->loadCount,0,reg->flags);
	if(clone) {
		memcpy(clone->pageFlags,reg->pageFlags,reg->pfSize * sizeof(ulong));
		reg_addTo(clone,p);
	}
	return clone;
}

void reg_sprintf(sStringBuffer *buf,const sRegion *reg,uintptr_t virt) {
	size_t i,x;
	sSLNode *n;
	prf_sprintf(buf,"\tSize: %zu bytes\n",reg->byteCount);
	prf_sprintf(buf,"\tLoad: %zu bytes\n",reg->loadCount);
	prf_sprintf(buf,"\tflags: ");
	reg_sprintfFlags(buf,reg);
	prf_sprintf(buf,"\n");
	if(reg->binary.ino) {
		prf_sprintf(buf,"\tbinary: ino=%d dev=%d modified=%u offset=%#Ox\n",
				reg->binary.ino,reg->binary.dev,reg->binary.modifytime,reg->binOffset);
	}
	prf_sprintf(buf,"\tTimestamp: %d\n",reg->timestamp);
	prf_sprintf(buf,"\tProcesses: ");
	for(n = sll_begin(reg->procs); n != NULL; n = n->next)
		prf_sprintf(buf,"%d ",((sProc*)n->data)->pid);
	prf_sprintf(buf,"\n");
	prf_sprintf(buf,"\tPages (%d):\n",BYTES_2_PAGES(reg->byteCount));
	for(i = 0, x = BYTES_2_PAGES(reg->byteCount); i < x; i++) {
		prf_sprintf(buf,"\t\t%d: (%p) (swblk %d) %c%c%c\n",i,virt + i * PAGE_SIZE,
				(reg->pageFlags[i] & PF_SWAPPED) ? reg_getSwapBlock(reg,i) : 0,
				reg->pageFlags[i] & PF_COPYONWRITE ? 'c' : '-',
				reg->pageFlags[i] & PF_DEMANDLOAD ? 'l' : '-',
				reg->pageFlags[i] & PF_SWAPPED ? 's' : '-');
	}
}

void reg_printFlags(const sRegion *reg) {
	sStringBuffer buf;
	buf.dynamic = true;
	buf.len = 0;
	buf.size = 0;
	buf.str = NULL;
	reg_sprintfFlags(&buf,reg);
	if(buf.str) {
		vid_printf("%s",buf.str);
		cache_free(buf.str);
	}
}

void reg_print(const sRegion *reg,uintptr_t virt) {
	sStringBuffer buf;
	buf.dynamic = true;
	buf.len = 0;
	buf.size = 0;
	buf.str = NULL;
	reg_sprintf(&buf,reg,virt);
	if(buf.str != NULL)
		vid_printf("%s",buf.str);
	else
		vid_printf("- no regions -\n");
	cache_free(buf.str);
}

static void reg_sprintfFlags(sStringBuffer *buf,const sRegion *reg) {
	struct {
		const char *name;
		ulong no;
	} flagNames[] = {
		{"Gr",RF_GROWABLE},
		{"Sh",RF_SHAREABLE},
		{"Wr",RF_WRITABLE},
		{"Ex",RF_EXECUTABLE},
		{"St",RF_STACK},
		{"NoFree",RF_NOFREE},
		{"TLS",RF_TLS},
		{"GrDwn",RF_GROWS_DOWN}
	};
	size_t i;
	for(i = 0; i < ARRAY_SIZE(flagNames); i++) {
		if(reg->flags & flagNames[i].no)
			prf_sprintf(buf,"%s ",flagNames[i].name);
	}
}