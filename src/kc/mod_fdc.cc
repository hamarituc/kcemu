/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/fdc0s.h"
#include "kc/mod_fdc.h"

ModuleFDC::ModuleFDC(ModuleFDC &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _portg = NULL;
  _master = &tmpl;

  if (fdc_fdc)
    return;

  if (_master->get_count() > 0)
    return;

  _master->set_count(1);
  _fdc_type = _master->_fdc_type;

  switch (_fdc_type)
    {
    case FDC_INTERFACE_SCHNEIDER:
      fdc_fdc = new FDC0S(); // global in kc.cc
      _portg = ports->register_ports("FDC", 0xf0, 10, fdc_fdc, 0);
      break;
    case FDC_INTERFACE_KRAMER:
      return; // not yet implemented
    }

  set_valid(true);
}

ModuleFDC::ModuleFDC(const char *name, fdc_interface_type_t fdc_type) :
  ModuleInterface(name, 0, KC_MODULE_Z1013)
{
  _count = 0;
  _portg = NULL;
  _fdc_type = fdc_type;
}

ModuleFDC::~ModuleFDC(void)
{
  if (_portg)
    {
      _master->set_count(0);
      ports->unregister_ports(_portg);
      delete fdc_fdc;
      fdc_fdc = NULL;
    }
}

void
ModuleFDC::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleFDC::clone(void)
{
  return new ModuleFDC(*this);
}

int
ModuleFDC::get_count(void)
{
  return _count;
}

void
ModuleFDC::set_count(int count)
{
  _count = count;
}