// Update hardware cursor

#include "monitor.h"

// VGA framebuffer begins at 0xB8000;
u16int *video_memory = (u16int *)0xB8000;

// Cursor positions
u8int cursor_x = 0;
u8int cursor_y = 0;

static void move_cursor()
{
  u16int cursorLocation = cursor_y * 80 + cursor_x;
  outb(0x3D4, 14);
  outb(0x3D5, cursorLocation >> 8); // Send the high byte
  outb(0x3D4, 15);
  outb(0x3D5, cursorLocation);      // Send the low byte
}

// Scrolling screens are cool
static void scroll()
{
  u8int attributeByte = (0 << 4 ) | (15 & 0x05);
  u16int blank = 0x20 | (attributeByte << 8);

  if (cursor_y >= 25)
  {
    // Move the current text 
    // back in the buffer by 1 line
    int i;
    for (i = 0 * 80; i < 24 * 80; i++)
    {
      video_memory[i] = video_memory[i+80];
    }   
    
    for (i = 24 * 80; i < 25 * 80; i++)
    {
      video_memory[i] = blank;
    }

    cursor_y = 24;
  }
}

void monitor_put(char c)
{
  u8int backColor = 0;
  u8int foreColor = 15;

  // Made up of 2 nibbles setting the background and foreground colors
  u8int attributeByte = (backColor << 4) | (foreColor & 0x07);

  // We send the attributeByte to the top 8 bits of the VGA board
  u16int attribute = attributeByte << 8;
  u16int *location;

  // Handle backspace
  if (c == 0x08 && cursor_x)
  {
    cursor_x--;
  }
  // Handle tab
  else if (c == 0x09)
  {
    cursor_x = (cursor_x+8) & ~(8-1);
  }
  // Handle CR
  else if (c == '\r')
  {
    cursor_x = 0;
  }
  // Handle newline
  else if (c == '\n')
  {
    cursor_x = 0;
    cursor_y++;
  }
  // Handle any printable character
  else if (c >= ' ')
  {
    location = video_memory + (cursor_y*80 + cursor_x);
    *location = c | attribute;
    cursor_x++;
  }
  // If we're above or at 80 chars we need a new line holmes..
  if (cursor_x >= 80)
  {
    cursor_x = 0;
    cursor_y ++;
  }

  scroll();
  move_cursor();
}

void monitor_clear()
{
  u8int attributeByte = (0 << 4) | (15 & 0x05);
  u16int blank = 0x20 | (attributeByte << 8);

  int i;
  for (i = 0; i < 80 * 25; i++)
  {
    video_memory[i] = blank;
  }

  cursor_x = 0;
  cursor_y = 0;
  move_cursor();
}

void monitor_write(char *c)
{
  int i = 0;
  while (c[i])
  {
    monitor_put(c[i++]);
  }
}

void monitor_write_hex(u32int n)
{
  int i = 8;
  monitor_write("0x");
  while (i-- > 0)
  {
    u8int c = "0123456789abcdef"[(n>>(i*4))&0x0F];
    monitor_put(c);
  }
}

void monitor_write_dec(u32int n)
{
  static u8int r;
  if (n > 0)
  {
    monitor_write_dec(n / 10);
    r = n % 10;
    monitor_put(r+'0');
  }
  else
    return;
}
