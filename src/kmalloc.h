#include <stdint.h>
#include "paging.h"
#pragma once

void init_heap(void);
void kfree(intptr_t addr);
intptr_t krealloc(intptr_t old_ptr, size_t bytes);
intptr_t kmalloc(size_t bytes);