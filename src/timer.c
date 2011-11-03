#include "timer.h"
#include "isr.h"
#include "monitor.h"

u32int tick = 0;

static void timer_callback(registers_t regs)
{
  tick++;
  monitor_write("Tick: ");
  monitor_write_dec(tick);
  monitor_write("\n");
}

void init_timer(u32int freq)
{
  // register the timer callback
  register_interrupt_handler(IRQ0, &timer_callback);
  
  // We send this value to the PIT. It's the input clock divided by frequency.
  u32int divisor = 1193180 / freq;

  // Send the command byte
  outb(0x43, 0x36);

  u8int l = (u8int)(divisor & 0xFF);
  u8int h = (u8int)((divisor >> 8) && 0xFF);

  outb(0x40, l);
  outb(0x40, h);
}
