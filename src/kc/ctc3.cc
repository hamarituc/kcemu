/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ctc3.cc,v 1.9 2002/06/09 14:24:33 torsten_paul Exp $
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

#include "kc/rc.h"
#include "kc/tape.h"
#include "kc/ctc3.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

CTC3::CTC3(void)
{
}

CTC3::~CTC3(void)
{
}

byte_t
CTC3::in(word_t addr)
{
  DBG(2, form("KCemu/CTC/3/in",
              "CTC3::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return c_in(0);
    case 1:
      return c_in(1);
    case 2:
      return c_in(2);
    case 3:
      return c_in(3);
    }
}

void
CTC3::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/CTC/3/out",
              "CTC3::out(): addr = %04x, val = %02x\n",
              addr, val));
  switch (addr & 3)
    {
    case 0:
      c_out(0, val);
      break;
    case 1:
      c_out(1, val);
      break;
    case 2:
      c_out(2, val);
      break;
    case 3:
      c_out(3, val);
      break;
    }
}

bool
CTC3::irq_0(void)
{
}

bool
CTC3::irq_1(void)
{
  tape->tape_signal();
}

bool
CTC3::irq_2(void)
{
  static int b = RC::instance()->get_int("Full Flash Support", 1);

  if (b || isCounter(2))
    ui->flash(true);
  else
    ui->flash(false);
}

bool
CTC3::irq_3(void)
{
}

long
CTC3::counter_value_0(void)
{
  return 0;
}

long
CTC3::counter_value_1(void)
{
  return 0;
}

long
CTC3::counter_value_2(void)
{
  /*
   *  CLK for channel 2 is 50 Hz
   */
  return CHANNEL_2_CLK;
}

long
CTC3::counter_value_3(void)
{
  return 0;
}
