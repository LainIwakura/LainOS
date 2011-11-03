#include "paging.h"
#include "kheap.h"

page_directory_t *kernel_directory = 0;

page_directory_t *current_directory = 0;

u32int *frames;
u32int nframes;

extern u32int placement_address;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(u32int frame_addr)
{
  u32int frame = frame_addr / 0x1000;
  u32int idx   = INDEX_FROM_BIT(frame);
  u32int off   = OFFSET_FROM_BIT(frame);
  frames[idx] |= (0x1 << off);
}

static void clear_frame(u32int frame_addr)
{
  u32int frame  = frame_addr / 0x1000;
  u32int idx    = INDEX_FROM_BIT(frame);
  u32int off    = OFFSET_FROM_BIT(frame);
  frames[idx] &= ~(0x1 << off);
}

static u32int test_frame(u32int frame_addr)
{
  u32int frame  = frame_addr / 0x1000;
  u32int idx    = INDEX_FROM_BIT(frame);
  u32int off    = OFFSET_FROM_BIT(frame);
  return (frames[idx] & (0x1 << off));
}

static u32int first_frame()
{
  u32int i, j;
  for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
  {
    if (frames[i] != 0xFFFFFFFF)
    {
      for (j = 0; j < 32; j++)
      {
        u32int toTest = 0x1 << j;
        if (!(frames[i] & toTest))
          return i * 4 * 8 + j;
      }
    }
  }
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
  if (page->frame != 0)
    return;
  else
  {
    u32int idx = first_frame();
    if (idx == (u32int) - 1)
    {
      // PANIC("No free frames!!!!");
    }
    set_frame(idx * 0x1000);
    page->present = 1;
    page->rw = (is_writeable) ? 1 : 0;
    page->user = (is_kernel) ? 1 : 0;
    page->frame = idx;
  }
}

void free_frame(page_t *page)
{
  u32int frame;
  if (!(frame = page->frame))
    return;
  else
  {
    clear_frame(frame);
    page->frame = 0x0;
  }
}

void initialise_paging()
{
  u32int mem_end_page = 0x1000000;

  nframes = mem_end_page / 0x1000;
  frames = (u32int*)kmalloc(INDEX_FROM_BIT(nframes));
  memset(frames, 0, INDEX_FROM_BIT(nframes));

  kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
  current_directory = kernel_directory;

  int i = 0;
  while (i < placement_address)
  {
    alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  register_interrupt_handler(14, page_fault);

  switch_page_directory(kernel_directory);
}

void switch_page_directory(page_directory_t *dir)
{
  current_directory = dir;
  asm volatile("mov %0, %%cr3" :: "r"(&dir->tablesPhysical));
  u32int cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

page_t *get_page(u32int address, int make, page_directory_t *dir)
{
  address /= 0x1000;

  u32int table_idx = address / 1024;
  if (dir->tables[table_idx])
    return &dir->tables[table_idx]->pages[address%1024];
  else if(make)
  {
    u32int tmp;
    dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
    dir->tablesPhysical[table_idx] = tmp | 0x7;
    return &dir->tables[table_idx]->pages[address%1024];
  }
  else
    return 0;
}

void page_fault(registers_t regs)
{
  u32int faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

  int present = !(regs.err_code & 0x1);
  int rw = regs.err_code & 0x2;
  int us = regs.err_code & 0x4;
  int reserved = regs.err_code & 0x8;
  int id = regs.err_code & 0x10;

  monitor_write("Page fault! ( ");
  if (present) { monitor_write("present "); }
  if (rw) { monitor_write("read-only "); }
  if (us) { monitor_write("user-mode "); }
  if (reserved) { monitor_write("reserved "); }
  monitor_write(") at ");
  monitor_write_hex(faulting_address);
  monitor_write("\n");
  PANIC("Page fault");
}
