/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_ram1.cc,v 1.4 2002/10/31 01:46:35 torsten_paul Exp $
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
#include "kc/mod_ram1.h"

ModuleRAM1::ModuleRAM1(ModuleRAM1 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  unsigned int a;

  _group = NULL;
  _size = tmpl._size;
  _ram = new byte_t[_size];
  _addr = tmpl._addr;
  if (_ram)
    {
      srand(time(NULL));
      for (a = 0;a < _size;a++)
	_ram[a] = (byte_t)(256.0 * rand() / (RAND_MAX + 1.0));
      set_valid(true);
      _group = memory->register_memory(get_name(), _addr, _size, _ram, 0, 0);
    }
}

ModuleRAM1::ModuleRAM1(const char *name, word_t addr, dword_t size) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _ram = new byte_t[size];
  _addr = addr;
  _size = size;

  if (_ram)
    {
      _group = 0;
      memset(_ram, 0, _size);
      set_valid(true);
    }
}

ModuleRAM1::~ModuleRAM1(void)
{
  if (_group)
    memory->unregister_memory(_group);

  delete[] _ram;
}

void
ModuleRAM1::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleRAM1::clone(void)
{
  return new ModuleRAM1(*this);
}

void
ModuleRAM1::reset(bool power_on)
{
  if (power_on)
    Memory::scratch_mem(_ram, _size);
}
