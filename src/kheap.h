#ifndef KHEAP_H
#define KHEAP_H

#include "common.h"

u32int kmalloc_int(u32int sz, int align, u32int *phys);

// Allocate memory, must be page aligned
u32int kmalloc_a(u32int sz);

// Allocate memory, returns address in phys.
u32int kmalloc_p(u32int sz, u32int *phys);

u32int kmalloc_ap(u32int sz, u32int *phys);

u32int kmalloc(u32int sz);

#endif
