
#pragma once

#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "block_meta.h"

// Michael Saelee I.I.T.
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define MMAP_THRESHOLD (128 * 1024)
#define PREALLOCATE_SIZE (128 * 1024)
#define META_SIZE sizeof(struct block_meta)

struct block_meta *list_head;

void preallocate_memory(void);
struct block_meta *expand_memory(size_t size);
