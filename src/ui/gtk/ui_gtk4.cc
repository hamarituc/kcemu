/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk4.cc,v 1.15 2002/06/09 14:24:34 torsten_paul Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//#define DISPLAY_DIRTY
//#define DISPLAY_DIRTY_RENDER
//#define DISPLAY_DIRTY_DRAW

#include <iostream.h>
#include <iomanip.h>

#include <unistd.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/rc.h"
#include "kc/z80.h"
#include "kc/ports.h"
#include "kc/memory.h"

#include "ui/hsv2rgb.h"
#include "ui/gtk/ui_gtk4.h"

#include "libdbg/dbg.h"

static int __frame_skip = 0;

static word_t __addr2scr[0x2800];
static word_t __scr2addr[0x2800];
static word_t __pdirty[0x200];
static word_t __bdirty[0x200];
static GdkImage *__image;
static gulong __fg, __bg;
static gulong __pixel[4];

static inline void set_bit(int nr, void *addr)
{
  ((unsigned short *)addr)[nr >> 4] |= (1UL << (nr & 15));
}

static inline void clear_bit(int nr, void *addr)
{
  ((unsigned short *)addr)[nr >> 4] &= ~(1UL << (nr & 15));
}

static inline int test_bit(int nr, const void * addr)
{
  return ((1UL << (nr & 15)) & (((const unsigned short *)addr)[nr >> 4])) != 0;
}

UI_Gtk4::UI_Gtk4(void)
{
  reset();
  // z80->register_ic(this); already done in base class!
}

UI_Gtk4::~UI_Gtk4(void)
{
  // z80->unregister_ic(this); already done in base class!
}

int
UI_Gtk4::get_width(void)
{
  return kcemu_ui_scale * 320;
}

int
UI_Gtk4::get_height(void)
{
  return kcemu_ui_scale * 256;
}

int
UI_Gtk4::get_callback_offset(void)
{
  return CB_OFFSET;
}

void
UI_Gtk4::callback(void * /* data */)
{
  ui_callback();
  if (!_auto_skip)
    handle_flash();
}

const char *
UI_Gtk4::get_title(void)
{
  return "KC 85/4 Emulator";
}

static inline void
pix_loop(register int x2, register int y2, byte_t val)
{
  register int a;

  switch (kcemu_ui_scale)
    {
    case 1:
      for (a = 0;a < 8;a++)
	{
	  if (val & 0x80)
	    gdk_image_put_pixel(__image, x2, y2, __fg);
	  else
	    gdk_image_put_pixel(__image, x2, y2, __bg);
	  x2++;
	  val <<= 1;
	}
      break;
    case 2:
      x2 *= 2;
      y2 *= 2;
      for (a = 0;a < 8;a++)
	{
	  if (val & 0x80)
	    {
	      gdk_image_put_pixel(__image, x2,     y2 + 1, __fg);
	      gdk_image_put_pixel(__image, x2,     y2,     __fg);
	      gdk_image_put_pixel(__image, x2 + 1, y2 + 1, __fg);
	      gdk_image_put_pixel(__image, x2 + 1, y2,     __fg);
	    }
	  else
	    {
	      gdk_image_put_pixel(__image, x2,     y2 + 1, __bg);
	      gdk_image_put_pixel(__image, x2,     y2,     __bg);
	      gdk_image_put_pixel(__image, x2 + 1, y2 + 1, __bg);
	      gdk_image_put_pixel(__image, x2 + 1, y2,     __bg);
	    }
	  x2 += 2;
	  val <<= 1;
	}
      break;
    }
}

/*
 *  some optimizations to get better inner loop for the pixel
 *  drawing; using dword_t for val and col will let the compiler
 *  happily use 32 bit registers
 *  (of cause this only applies to 32 bit intel)
 */
static inline void
pix_loop_hires(register int x2, register int y2, dword_t val, dword_t col)
{
  int a, b;

  val *= 2;

  switch (kcemu_ui_scale)
    {
    case 1:
      for (a = 0;a < 8;a++)
	{
	  b  = ((val & 256) | (col & 128)) >> 7;
	  gdk_image_put_pixel(__image, x2, y2, __pixel[b]);
	  x2++;
	  val <<= 1;
	  col <<= 1;
	}
      break;
    case 2:
      x2 *= 2;
      y2 *= 2;
      for (a = 0;a < 8;a++)
	{
	  b  = ((val & 256) | (col & 128)) >> 7;
	  gdk_image_put_pixel(__image, x2    , y2 + 1, __pixel[b]);
	  gdk_image_put_pixel(__image, x2    , y2    , __pixel[b]);
	  gdk_image_put_pixel(__image, x2 + 1, y2 + 1, __pixel[b]);
	  gdk_image_put_pixel(__image, x2 + 1, y2    , __pixel[b]);
	  x2 += 2;
	  val <<= 1;
	  col <<= 1;
	}
      break;
    }
}

static inline void
hsv_to_gdk_color(double h, double s, double v, GdkColor *col)
{
  int r, g, b;

  hsv2rgb(h, s, v, &r, &g, &b);
  col->red   = r << 8;
  col->green = g << 8;
  col->blue  = b << 8;
}

void
UI_Gtk4::allocate_colors(double saturation_fg,
			 double saturation_bg,
			 double brightness_fg,
			 double brightness_bg,
			 double black_level,
			 double white_level)
{
  int a;
  
  hsv_to_gdk_color(  0,             0,   black_level, &_col[ 0]); /* black */
  hsv_to_gdk_color(240, saturation_fg, brightness_fg, &_col[ 1]); /* blue */
  hsv_to_gdk_color(  0, saturation_fg, brightness_fg, &_col[ 2]); /* red */
  hsv_to_gdk_color(300, saturation_fg, brightness_fg, &_col[ 3]); /* magenta */
  hsv_to_gdk_color(120, saturation_fg, brightness_fg, &_col[ 4]); /* green */
  hsv_to_gdk_color(180, saturation_fg, brightness_fg, &_col[ 5]); /* cyan */
  hsv_to_gdk_color( 60, saturation_fg, brightness_fg, &_col[ 6]); /* yellow */
  hsv_to_gdk_color(  0,             0,   white_level, &_col[ 7]); /* white */
  
  hsv_to_gdk_color(  0,             0,   black_level, &_col[ 8]); /* black */
  hsv_to_gdk_color(270, saturation_fg, brightness_fg, &_col[ 9]); /* blue + 30� */
  hsv_to_gdk_color( 30, saturation_fg, brightness_fg, &_col[10]); /* red + 30� */
  hsv_to_gdk_color(330, saturation_fg, brightness_fg, &_col[11]); /* magenta + 30� */
  hsv_to_gdk_color(150, saturation_fg, brightness_fg, &_col[12]); /* green + 30� */
  hsv_to_gdk_color(210, saturation_fg, brightness_fg, &_col[13]); /* cyan + 30� */
  hsv_to_gdk_color( 90, saturation_fg, brightness_fg, &_col[14]); /* yellow + 30� */
  hsv_to_gdk_color(  0,             0,   white_level, &_col[15]); /* white */
  
  hsv_to_gdk_color(  0,             0,   black_level, &_col[16]); /* black */
  hsv_to_gdk_color(240, saturation_bg, brightness_bg, &_col[17]); /* blue */
  hsv_to_gdk_color(  0, saturation_bg, brightness_bg, &_col[18]); /* red */
  hsv_to_gdk_color(300, saturation_bg, brightness_bg, &_col[19]); /* magenta */
  hsv_to_gdk_color(120, saturation_bg, brightness_bg, &_col[20]); /* green */
  hsv_to_gdk_color(180, saturation_bg, brightness_bg, &_col[21]); /* cyan */
  hsv_to_gdk_color( 60, saturation_bg, brightness_bg, &_col[22]); /* yellow */
  hsv_to_gdk_color(  0,             0, brightness_bg, &_col[23]); /* white */
  
  _colormap = gdk_colormap_get_system();
  for (a = 0;a < 24;a++)
    gdk_color_alloc(_colormap, &_col[a]);

  __pixel[0] = _col[0].pixel; /* hires color 0 -> black */
  __pixel[1] = _col[2].pixel; /* hires color 1 -> red */
  __pixel[2] = _col[5].pixel; /* hires color 2 -> cyan */
  __pixel[3] = _col[7].pixel; /* hires color 3 -> white */
}

void
UI_Gtk4::render_tile_hires(byte_t *irm, int x, int y, bool no_cache)
{
  int has_flash;
  int a, b, xx, yy, x1, y1;

  __image = _image;
  
  xx = x * 2;
  yy = y * 16;
  has_flash = 0;

  __pdirty[(x << 4) | y] = 0;
  __bdirty[(x << 4) | y] = has_flash;

  for (y1 = 0;y1 < 16;y1++)
    {
      for (x1 = 0;x1 < 2;x1++)
	{
	  int changed = 0;
	  int idx = ((xx + x1) << 8) | (yy + y1);

	  byte_t val = irm[idx];
	  byte_t col = irm[idx + 0x4000];
	  
	  pix_loop_hires(8 * (xx + x1), yy + y1, val, col);
	}
    }
}

void
UI_Gtk4::render_tile(byte_t *irm, int x, int y, bool no_cache)
{
  int has_flash;
  int xx, yy, x1, y1, x2, y2;
  int a, b, c;
  byte_t val, col;
  struct {
    int x, y;
    long long counter;
    byte_t data_val[32];
    byte_t data_col[32];
  } tile;

  __image = _image;

  has_flash = 0;
  xx = x * 2;
  yy = y * 16;

#ifdef WRITE_TILES
  int fd = open("/tmp/tiles.out", O_APPEND | O_CREAT | O_RDWR, 0644);
  memset(&tile, 0, sizeof(tile));
  tile.x = x;
  tile.y = y;
  tile.counter = z80->getCounter();
#endif /* WRITE_TILES */

  for (y1 = 0;y1 < 16;y1++)
    {
      for (x1 = 0;x1 < 2;x1++)
        {
          int changed = 0;
          int idx = ((xx + x1) << 8) | (yy + y1);
          
	  val = irm[idx];
	  if (val != _pix_mem[idx])
	    {
	      changed++;
	      _pix_mem[idx] = val;
	    }
	  col = irm[idx + 0x4000];
	  if (col != _col_mem[idx])
	    {
	      changed++;
	      _col_mem[idx] = col;
	    }

	  changed += no_cache;
          
          /*
           *  fetch color
           */
          __bg = _col[(col & 7) | 0x10].pixel;
          __fg = _col[(col >> 3) & 15].pixel;
          if (col & 128)
            {
              changed++;
              if (__fg != __bg)
                {
                  has_flash = 1;
                  if (test_bit(yy + y1, _flash_v1))
		    {
#ifdef WRITE_TILES
		      col &= 7;
		      col |= (col << 3);
		      col |= 0x40;
#endif /* WRITE_TILES */
		      __fg = __bg;
		    }
                }
            }
          
#ifdef WRITE_TILES
	  tile.data_val[2 * y1 + x1] = val;
	  tile.data_col[2 * y1 + x1] = col;
#endif /* WRITE_TILES */

          /*
           *  skip if no change
           */
          if (!changed) continue;
	  
	  pix_loop(8 * (xx + x1), yy + y1, val);
        }
    }
  __pdirty[(x << 4) | y] = 0;
  __bdirty[(x << 4) | y] = has_flash;

#ifdef WRITE_TILES
  write(fd, &tile, sizeof(tile));
  close(fd);
#endif /* WRITE_TILES */
}

void
UI_Gtk4::update(bool full_update, bool clear_cache)
{
  int hires;
  byte_t *irm;
  int a, b, c, x, y, z, scale;
  static byte_t *irm_old = NULL;
  static int hires_old = -1;
  static int fs_val = RC::instance()->get_int("Frame Skip", 0);

  scale = kcemu_ui_scale * 16;

  irm = memory->getIRM();
  if (irm != irm_old)
    {
      /*
       *  on screen switch we need to ignore the
       *  display cache
       */
      irm_old = irm;
      clear_cache = true;
    }  

  hires = ((ports->in(0x84) & 8) == 0);
  if (hires != hires_old)
    {
      /*
       *  when switching lores/hires mode ignore the
       *  display cache too
       */
      hires_old = hires;
      clear_cache = true;
    }

  if (clear_cache)
    {
      full_update = false;
      for (a = 0;a < 0x200;a++)
	{
	  __pdirty[a] = 0xffff;
	  __bdirty[a] = 0xffff;
	}
    }
  if (full_update)
    {
      gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
                     0, 0, 0, 0, get_width(), get_height());
      return;
    }
  
  if (__frame_skip > 0)
    {
      __frame_skip--;
      return;
    }
  __frame_skip = fs_val;


  for (y = 0;y < 16;y++)
    {
#ifdef DISPLAY_DIRTY
      for (x = 0;x < 20;x++)
        cout << (__pdirty[(x << 4) | y] ? '#' : '.');
      cout << " - ";
      for (x = 0;x < 20;x++)
        cout << (__bdirty[(x << 4) | y] ? '#' : '.');
      cout << " - ";
#endif

      a = 0;
      b = 0;
      c = (_flash_v1[y] != _flash_v2[y]);
      for (x = 0;x < 20;x++)
        {
          z = (x << 4) | y;
          if (__pdirty[z] || (c && __bdirty[z]))
            {
#ifdef DISPLAY_DIRTY_RENDER
              cout << '#';
#endif
	      if (hires)
		render_tile_hires(irm, x, y, clear_cache);
	      else
		render_tile(irm, x, y, clear_cache);
              b += 16;
            }
          else
            {
#ifdef DISPLAY_DIRTY_RENDER
              cout << '.';
#endif
              if (b)
		{
		  gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
				 scale * a, scale * y, scale * a, scale * y, kcemu_ui_scale*b, scale);
		}
#ifdef DISPLAY_DIRTY_DRAW
              if (b)
                for (int i = 0;i < (b / 16);i++) 
                  cout << (char )('a' + a);
              cout << '.';
#endif
              b = 0;
              a = x + 1;
            }
        }
      if (b)
	{
	  gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
			 scale * a, scale * y, scale * a, scale * y, kcemu_ui_scale*b, scale);
	}
#ifdef DISPLAY_DIRTY_DRAW
      if (b)
        for (int i = 0;i < (b / 16);i++) 
          cout << (char )('a' + a);
#endif
#ifdef DISPLAY_DIRTY
      cout.form(" - %04x | %04x | %04x | %d",
                _flash_v1[y] & 0xffff,
                _flash_v2[y] & 0xffff,
                _flash_v3[y] & 0xffff,
                _flash_v1[y] != _flash_v2[y]);
#endif
#ifdef DISPLAY_DIRTY
      cout << endl;
#endif
    }
  
  text_update();
}

void
UI_Gtk4::handle_flash(void)
{
  int a, c;

  _flash_count += 2;
  if (_flash_count == 6) _flash_count = 0;
  if (_flash_count & 1) return;

  if (_flash_idx < 255)
    {
      if (_flash_idx == 0)
        c = _flash;
      else
        c = test_bit(_flash_idx - 1, _flash_v3);

      if (c)
        while (_flash_idx < 255) set_bit(_flash_idx++, _flash_v3);
      else
        while (_flash_idx < 255) clear_bit(_flash_idx++, _flash_v3);
    }

  /*
   *  new frame time will be set when the first call to flash() occurs
   *  this is needed to synchronize the flash frequency with the
   *  screen update
   */
  _frame_time = 0;

  _flash_idx = 0;
  switch (_flash_count)
    {
    case 0:
      _flash_v1 = &_flash_vec[0];
      _flash_v2 = &_flash_vec[16];
      _flash_v3 = &_flash_vec[32];
      break;
    case 2:
      _flash_v1 = &_flash_vec[16];
      _flash_v2 = &_flash_vec[32];
      _flash_v3 = &_flash_vec[0];
      break;
    case 4:
      _flash_v1 = &_flash_vec[32];
      _flash_v2 = &_flash_vec[0];
      _flash_v3 = &_flash_vec[16];
      break;
    default:
      break;
    }
}

/*
 *  Linefrequency: 15.625 kHz => 64�s/line => 112 cycles/line
 *
 *  This function is called by the CTC channel 2.
 */
void
UI_Gtk4::flash(bool enable)
{
  /*
   *  time (clock counter) of the previous call to this function
   */
  static long long t = 0;
  /*
   *  floating time offset to fine tune the scrolling effect
   *  that is caused by the interference between the flash
   *  frequency and the crt refresh
   */
  static long long offset = 0;
  /*
   *  config value that is added to offset each time this function
   *  is called
   */
  static long long o = RC::instance()->get_int("Flash Offset", 50);
  /*
   *  difference between the current and the previous call to
   *  this function in clock ticks
   */
  long long diff;

  if (!enable)
    {
      _flash_enabled = true; // force reset of flash variables
      reset_flash(false);
      return;
    }

  _flash = !_flash;

  _flash_time = z80->getCounter();
  if (_frame_time == 0)
    {
      offset += o;
      diff = _flash_time - t;
      /*
       *  synchronize flash only if flash frequency is high enough
       *  to change the flash value twice per frame
       *  (128 lines * 112 tics/line)
       */
      if (diff < 14336)
        _flash = 0;
      /*
       *  reset flash scrolling offset
       */
      if (offset > 2 * diff)
        offset = 0;
      _frame_time = _flash_time + offset;
    }

  while (242)
    {
      if (_flash_idx >= 255) break;
      if ((112 * _flash_idx) > (_flash_time - _frame_time)) break;
      if (_flash)
        set_bit(_flash_idx++, _flash_v3);
      else
        clear_bit(_flash_idx++, _flash_v3);
    }

  t = _flash_time;
}

void
UI_Gtk4::memWrite(int addr, char val)
{
  int x = (addr >> 5) & 0x01f0;
  int y = (addr >> 4) & 0x0f;

  if ((addr & 0xc000) == 0x8000) __pdirty[x | y] = 1;
}

void
UI_Gtk4::reset_flash(bool enable)
{
  int a;

  if (_flash_enabled == enable)
    return;

  _flash_enabled = enable;

  _flash_idx = 0;
  _flash_time = 0;
  _flash_count = 0;
  _frame_time = 0;
  for (a = 0;a < 48;a++)
    _flash_vec[a] = 0;
  _flash_v1 = &_flash_vec[0];
  _flash_v2 = &_flash_vec[16];
  _flash_v3 = &_flash_vec[32];
}

void
UI_Gtk4::reset(bool power_on)
{
  _flash_enabled = false; // force reset of flash variables
  _cur_auto_skip = 0;
  _max_auto_skip = RC::instance()->get_int("Max Auto Skip", 6);
  reset_flash(true);
  z80->addCallback(CB_OFFSET, this, 0);
}
