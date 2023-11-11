// SPDX-License-Identifier: BSD-3-Clause

#include "block_handler.h"
#include "osmem.h"

void *os_malloc(size_t size)
{
	if (!size)
		return NULL;

	if (ALIGN(size) + META_SIZE >= MMAP_THRESHOLD)
		return (void *)expand_mapped_memory(size) + META_SIZE;

	preallocate_memory();
	struct block_meta *mem_block = find_best_free(size);

	if (mem_block)
		split_block(mem_block, size);
	else
		mem_block = expand_heap_memory(size);

	return (void *)mem_block + META_SIZE;
}

void os_free(void *ptr)
{
	if (!ptr)
		return;

	// TODO: search if previosly allocated
	struct block_meta *mem_block = ptr - META_SIZE;

	if (mem_block->status == STATUS_MAPPED)
		munmap(mem_block, META_SIZE + mem_block->size);
	else
		coalesce_block(mem_block);
}

void *os_calloc(size_t nmemb, size_t size)
{
	if (!(nmemb * size) || (nmemb * size / nmemb != size))
		return NULL;

	size *= nmemb;
	if (ALIGN(size) + META_SIZE >= getpagesize())
		return (void *)expand_mapped_memory(size) + META_SIZE;

	preallocate_memory();
	struct block_meta *mem_block = find_best_free(size);

	if (mem_block)
		split_block(mem_block, size);
	else
		mem_block = expand_heap_memory(size);

	memset((void *)mem_block + META_SIZE, 0, mem_block->size);
	return (void *)mem_block + META_SIZE;
}

void *os_realloc(void *ptr, size_t size)
{
	/* TODO: Implement os_realloc */
	return NULL;
}
