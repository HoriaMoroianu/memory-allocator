
#include "block_handler.h"
#include "block_meta.h"

void preallocate_memory(void)
{
	void *new_block = sbrk(PREALLOCATE_SIZE);
	DIE(new_block == (void *) -1, "Preallocation failed!");

	((struct block_meta *)new_block)->size = PREALLOCATE_SIZE - META_SIZE;
	((struct block_meta *)new_block)->status = STATUS_FREE;
	((struct block_meta *)new_block)->prev = NULL;
	((struct block_meta *)new_block)->next = NULL;

	list_head = (struct block_meta *)new_block;
}

struct block_meta *expand_memory(size_t size)
{
	void *new_mem;

	if (ALIGN(size) + ALIGN(META_SIZE) >= MMAP_THRESHOLD) {
		new_mem = mmap(NULL, ALIGN(size) + ALIGN(META_SIZE), PROT_READ | PROT_WRITE,
						MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		DIE(new_mem == MAP_FAILED, "Failed to expand program memory!");

		// ((block_meta *list_head)new_mem)->status = STATUS_MAPPED;
		((struct block_meta *)new_mem)->status = STATUS_MAPPED;

	} else {
		new_mem = sbrk(ALIGN(size) + ALIGN(META_SIZE));
		DIE(new_mem == (void *) -1, "Failed to expand program memory!");

		((struct block_meta *)new_mem)->status = STATUS_ALLOC;
	}

	((struct block_meta *)new_mem)->size = size;
	((struct block_meta *)new_mem)->next = NULL;
	((struct block_meta *)new_mem)->prev = NULL;

	return new_mem;
}
