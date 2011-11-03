// main.c -- LainOS.
// This code initializes all of our cool stuff.

#include "monitor.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "paging.h"

int main(struct multiboot *mboot_ptr)
{
  init_descriptor_tables();

  monitor_clear();
  initialise_paging();
  monitor_write("Hello, Lain!\n");
  monitor_write("Testing hex output: ");
  monitor_write_hex(34);
  monitor_write("\nTesting decimal output: ");
  monitor_write_dec(250);
  monitor_put('\n');

  u32int *ptr = (u32int*)0xA0000000;
  u32int do_page_fault = *ptr;

  return 0;
}
