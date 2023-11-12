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
	if (!nmemb || !size || (nmemb * size / nmemb != size))
		return NULL;

	size *= nmemb;
	if (ALIGN(size) + META_SIZE >= (size_t)getpagesize())
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
	if (!ptr)
		return os_malloc(size);

	if (!size) {
		os_free(ptr);
		return NULL;
	}

	struct block_meta *mem_block = ptr - META_SIZE;

	if (mem_block->status == STATUS_FREE)
		return NULL;

	// Heap / mapped -> mapped
	if (ALIGN(size) + META_SIZE >= MMAP_THRESHOLD)
		return realloc_memory(mem_block, size);

	// Truncate
	if (ALIGN(size) <= mem_block->size) {
		// Mapped -> Heap
		if (mem_block->status == STATUS_MAPPED)
			return realloc_memory(mem_block, size);

		// Heap -> Heap + new free block
		split_block(mem_block, size);
		return (void *)mem_block + META_SIZE;
	}

	// Expand last
	if (mem_block == get_last_block()) {
		void *new_block = sbrk(ALIGN(size) - mem_block->size);
		DIE(new_block == (void *) -1, "Failed to expand program memory!");

		mem_block->size = ALIGN(size);
		return (void *)mem_block + META_SIZE;
	}

	// Expand middle
	if (mem_block->next->status == STATUS_FREE &&
		mem_block->next->size + META_SIZE >= (ALIGN(size) - mem_block->size)) {

		// Merge two blocks
		mem_block->size += (META_SIZE + mem_block->next->size);
		if (mem_block->next->next)
			mem_block->next->next->prev = mem_block;
		mem_block->next = mem_block->next->next;

		// Split excess
		split_block(mem_block, ALIGN(size));

		return (void *)mem_block + META_SIZE;
	}

	// Move memory or create new block
	return realloc_memory(mem_block, size);
}
