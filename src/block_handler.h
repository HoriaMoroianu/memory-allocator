
#pragma once

#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#include "block_meta.h"
#include "osmem.h"

// Michael Saelee I.I.T.
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define MMAP_THRESHOLD (128 * 1024)
#define PREALLOCATE_SIZE (128 * 1024)
#define META_SIZE ALIGN(sizeof(struct block_meta))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

struct block_meta *list_head;
struct block_meta *mapped_head;

void preallocate_memory(void);
void remove_mapped(struct block_meta *mem_block);
void split_block(struct block_meta *parent, size_t size);
void coalesce_block(struct block_meta *block);
void *realloc_memory(struct block_meta *mem_block, size_t size);

struct block_meta *find_best_free(size_t size);
struct block_meta *get_last_block(void);
struct block_meta *expand_mapped_memory(size_t size);
struct block_meta *expand_heap_memory(size_t size);

int check_address(struct block_meta *mem_block);
