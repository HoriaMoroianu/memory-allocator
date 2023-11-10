
#include "block_handler.h"
#include "block_meta.h"

void preallocate_memory(void)
{
	if (list_head)
		return;

	void *new_block = sbrk(PREALLOCATE_SIZE);

	DIE(new_block == (void *) -1, "Preallocation failed!");
	((struct block_meta *)new_block)->size = PREALLOCATE_SIZE - META_SIZE;
	((struct block_meta *)new_block)->status = STATUS_FREE;
	((struct block_meta *)new_block)->prev = NULL;
	((struct block_meta *)new_block)->next = NULL;

	list_head = (struct block_meta *)new_block;
}

/**
 * @param size Raw data size
 */
struct block_meta *find_best_free(size_t size)
{
	struct block_meta *best_block = NULL;

	for (struct block_meta *curr = list_head; curr; curr = curr->next) {
		if (curr->status == STATUS_FREE && curr->size >= ALIGN(size) &&
			(!best_block || best_block->size > curr->size))
				best_block = curr;
	}
	return best_block;
}

/**
 * @param size Raw data size
 */
struct block_meta *expand_mapped_memory(size_t size)
{
	void *new_block = mmap(NULL, ALIGN(size) + META_SIZE, PROT_READ |
							PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	
	DIE(new_block == MAP_FAILED, "Failed to expand program memory!");

	// TODO: add block in list
	((struct block_meta *)new_block)->size = ALIGN(size);
	((struct block_meta *)new_block)->status = STATUS_MAPPED;
	((struct block_meta *)new_block)->prev = NULL;
	((struct block_meta *)new_block)->next = NULL;

	return (struct block_meta *)new_block;
}

/**
 * @param size Raw data size
 */
struct block_meta *expand_heap_memory(size_t size)
{
	void *new_block = sbrk(ALIGN(size) + META_SIZE);

	DIE(new_block == (void *) -1, "Failed to expand program memory!");

	((struct block_meta *)new_block)->size = ALIGN(size);
	((struct block_meta *)new_block)->status = STATUS_ALLOC;
	((struct block_meta *)new_block)->next = NULL;

	struct block_meta *last_block = get_last_block();

	if (!last_block) {
		list_head = new_block;
		((struct block_meta *)new_block)->prev = NULL;

	} else {
		last_block->next = new_block;
		((struct block_meta *)new_block)->prev = last_block;
	}

	return (struct block_meta *)new_block;
}

struct block_meta *get_last_block(void)
{
	struct block_meta *curr = list_head;

	for (; curr; curr = curr->next)
		continue;

	return curr;
}
