/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: pio1.cc,v 1.6 2002/06/09 14:24:33 torsten_paul Exp $
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

#include <iostream.h>
#include <iomanip.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/pio1.h"

#include "libdbg/dbg.h"

#define memory ((Memory1 *)memory)

PIO1_1::PIO1_1(void)
{
}

PIO1_1::~PIO1_1(void)
{
}

byte_t
PIO1_1::in(word_t addr)
{
  DBG(2, form("KCemu/PIO/1a/in",
              "PIO1_1::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return in_A_DATA();
    case 1:
      return in_B_DATA();
    case 2:
      return in_A_CTRL();
    case 3:
      return in_B_CTRL();
    }
}

void
PIO1_1::out(word_t addr, byte_t val)
{
  DBG(3, form("KCemu/PIO/1a/out",
              "PIO1_1::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO1_1::change_A(byte_t changed, byte_t val)
{
}

void
PIO1_1::change_B(byte_t changed, byte_t val)
{
}

void
PIO1_1::tape_callback(byte_t val)
{
  strobe_A();
}

PIO1_2::PIO1_2(void)
{
}

PIO1_2::~PIO1_2(void)
{
}

byte_t
PIO1_2::in(word_t addr)
{
  DBG(2, form("KCemu/PIO/1b/in",
              "PIO1_2::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return in_A_DATA();
    case 1:
      return in_B_DATA();
    case 2:
      return in_A_CTRL();
    case 3:
      return in_B_CTRL();
    }
}

void
PIO1_2::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/1b/out",
              "PIO1_2::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO1_2::change_A(byte_t changed, byte_t val)
{
}

void
PIO1_2::change_B(byte_t changed, byte_t val)
{
}
