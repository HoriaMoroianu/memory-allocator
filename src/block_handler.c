
#include "block_handler.h"

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
		if (curr->status == STATUS_FREE && curr->size >= ALIGN(size)) {
			if (!best_block)
				best_block = curr;
			else if (best_block->size > curr->size)
				best_block = curr;
		}
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
	struct block_meta *last_block = get_last_block();

	if (last_block->status == STATUS_FREE) {
		void *new_block = sbrk(ALIGN(size) - last_block->size);
		DIE(new_block == (void *) -1, "Failed to expand program memory!");
	
		last_block->size = ALIGN(size);
		last_block->status = STATUS_ALLOC;
		return last_block;
	}

	void *new_block = sbrk(ALIGN(size) + META_SIZE);

	DIE(new_block == (void *) -1, "Failed to expand program memory!");

	((struct block_meta *)new_block)->size = ALIGN(size);
	((struct block_meta *)new_block)->status = STATUS_ALLOC;
	((struct block_meta *)new_block)->next = NULL;
	((struct block_meta *)new_block)->prev = last_block;
	last_block->next = new_block;

	return (struct block_meta *)new_block;
}

struct block_meta *get_last_block(void)
{
	struct block_meta *curr = list_head;

	for (; curr->next; curr = curr->next)
		continue;
	return curr;
}

/**
 * @param size Raw data size that remains allocated
 */
void split_block(struct block_meta *parent, size_t size)
{
	long remaining_free_size = parent->size - ALIGN(size);

	// Not enought for split
	if (remaining_free_size < (long)(META_SIZE + ALIGN(1))) {
		parent->status = STATUS_ALLOC;
		return;
	}

	struct block_meta *child = (void *)parent + META_SIZE + ALIGN(size);
	child->size = (size_t)remaining_free_size - META_SIZE;
	child->status = STATUS_FREE;
	child->prev = parent;
	child->next = parent->next;

	if (parent->next)
		parent->next->prev = child;
	
	parent->size = ALIGN(size);
	parent->status = STATUS_ALLOC;
	parent->next = child;
}

void coalesce_block(struct block_meta *block)
{
	block->status = STATUS_FREE;

	if (block->next) {
		if (block->next->status == STATUS_FREE) {
			block->size += (META_SIZE + block->next->size);
			if (block->next->next != NULL) {
				block->next->next->prev = block;
				block->next = block->next->next;
			} else {
				block->next = NULL;
			}
		}
	}

	if (block->prev) {
		if (block->prev->status == STATUS_FREE) {
			block->prev->size += (META_SIZE + block->size);
			if (block->next != NULL) {
				block->next->prev = block->prev;
				block->prev->next = block->next;
			} else {
				block->prev->next = NULL;
			}
		}
	}
}

void *realloc_memory(struct block_meta *mem_block, size_t size)
{
	void *new_ptr = os_malloc(size);
	memcpy(new_ptr, (void *)mem_block + META_SIZE, MIN(size, mem_block->size));
	os_free((void *)mem_block + META_SIZE);
	return new_ptr;
}
