/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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

#ifndef __kc_mod_segm_h
#define __kc_mod_segm_h

#include "kc/module.h"
#include "kc/memory.h"

class ModuleSegmentedRAM : public ModuleInterface
{
 private:
  byte_t _val;
  byte_t *_ram;
  int _segments;
  word_t _segment_size;
  MemAreaGroup *_group;

 protected:
  virtual int get_segment_count(void);
  virtual int get_segment_size(void);

  virtual int      get_segment_index(word_t addr, byte_t val) = 0;
  virtual word_t   get_base_address(word_t addr, byte_t val) = 0;

 public:
  ModuleSegmentedRAM(ModuleSegmentedRAM &tmpl);
  ModuleSegmentedRAM(const char *name, byte_t id, int segments, int segment_size);
  virtual ~ModuleSegmentedRAM(void);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void) = 0;
  virtual void reset(bool power_on = false);
};

#endif /* __kc_mod_segm_h */
