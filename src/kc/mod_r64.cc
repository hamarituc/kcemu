/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2003 Torsten Paul
 *
 *  $Id$
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

#include <string.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory1.h"
#include "kc/memory7.h"
#include "kc/mod_r64.h"

using namespace std;

/*
 *  64k RAM module for use with the KC87-CP/M system (CPM-Z9)
 *
 *  two pages of 16k at 0x4000, one page of 16k at 0x8000 and 0xc000
 *
 */
ModuleRAM64::ModuleRAM64(ModuleRAM64 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  int a;

  _ram[0] = new byte_t[0x4000];
  _ram[1] = new byte_t[0x4000];
  _ram[2] = new byte_t[0x4000];
  _ram[3] = new byte_t[0x4000];

  srand(time(NULL));
  for (a = 0;a < 0x4000;a++)
    {
      //_ram[0][a] = (byte_t)(256.0 * rand() / (RAND_MAX + 1.0));
      //_ram[1][a] = (byte_t)(256.0 * rand() / (RAND_MAX + 1.0));
      //_ram[2][a] = (byte_t)(256.0 * rand() / (RAND_MAX + 1.0));
      //_ram[3][a] = (byte_t)(256.0 * rand() / (RAND_MAX + 1.0));
      _ram[0][a] = 0;
      _ram[1][a] = 0;
      _ram[2][a] = 0;
      _ram[3][a] = 0;
    }

  _group[0] = memory->register_memory(get_name(), 0x4000, 0x4000, _ram[0], 10, 0);
  _group[1] = memory->register_memory(get_name(), 0x4000, 0x4000, _ram[1], 20, 0);
  _group[2] = memory->register_memory(get_name(), 0x8000, 0x4000, _ram[2], 10, 0);
  _group[3] = memory->register_memory(get_name(), 0xc000, 0x4000, _ram[3], 10, 0);

  _portg = ports->register_ports(get_name(), 4, 4, this, 0);

  set_valid(true);
}

ModuleRAM64::ModuleRAM64(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _portg = NULL;
  _ram[0] = _ram[1] = _ram[2] = _ram[3] = NULL;
  _group[0] = _group[1] = _group[2] = _group[3] = NULL;
}

ModuleRAM64::~ModuleRAM64(void)
{
  if (_portg)
    ports->unregister_ports(_portg);

  if (_group[0])
    memory->unregister_memory(_group[0]);
  if (_group[1])
    memory->unregister_memory(_group[1]);
  if (_group[2])
    memory->unregister_memory(_group[2]);
  if (_group[3])
    memory->unregister_memory(_group[3]);
  
  delete[] _ram[0];
  delete[] _ram[1];
  delete[] _ram[2];
  delete[] _ram[3];
}

void
ModuleRAM64::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleRAM64::clone(void)
{
  return new ModuleRAM64(*this);
}

byte_t
ModuleRAM64::in(word_t addr)
{
  in_out(addr);
  return 0xff;
}

void
ModuleRAM64::out(word_t addr, byte_t val)
{
  in_out(addr);
}

void
ModuleRAM64::in_out(word_t addr)
{
  switch (addr & 7)
    {
    case 0x04:
      break;
    case 0x05:
      break;
    case 0x06: // RAM between c000h and e7ffh in write only mode (read from rom)
    case 0x07: // RAM between c000h and e7ffh in read/write mode (romdi active)
      /*
       * FIXME: make common base class for Memory1 and Memory7
       */
      switch (get_kc_type())
	{
	case KC_TYPE_85_1:
	  ((Memory1 *)memory)->set_romdi(addr & 1);
	  break;
	case KC_TYPE_87:
	  ((Memory7 *)memory)->set_romdi(addr & 1);
	  break;
	default: break;
	}
      break;
    }
}