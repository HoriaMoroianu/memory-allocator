// SPDX-License-Identifier: BSD-3-Clause

#include "block_handler.h"
#include "osmem.h"

void *os_malloc(size_t size)
{
	// if (size == 0)
	// 	return NULL;

	// void *ptr = expand_memory(size);
	// return ptr + ALIGN(META_SIZE);

	preallocate_memory();
	return (void *)list_head + ALIGN(META_SIZE);
}

void os_free(void *ptr)
{
	if (!ptr)
		return;

	// struct block_meta *block = ptr - ALIGN(META_SIZE);
	// munmap(block, ALIGN(META_SIZE) + ALIGN(block->size));
}

void *os_calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement os_calloc */
	return NULL;
}

void *os_realloc(void *ptr, size_t size)
{
	/* TODO: Implement os_realloc */
	return NULL;
}
