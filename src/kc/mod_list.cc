/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_list.cc,v 1.15 2002/06/09 14:24:33 torsten_paul Exp $
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

/*
 *  Quelle: http://www.kc85emu.de/scans/fa0192/Bild5.jpg
 *  Original: Zeitschrift Funkamateur
 * 
 *  ---: D001 Grundger�t KC85/2
 *  ---: D001 Grundger�t KC85/3
 *  ---: D001 Grundger�t KC85/4
 *  ---: D002 Busdriver
 *  ---: D004 Floppy Disk Drive
 *  a7h: D004 Floppy Disk Basis (ROM) -- always at slot fch!
 *  efh: M001 Digital IN/OUT
 *  eeh: M003 V24 (2 Kan�le)
 *  ---: M005 User (Leermodul)
 *  ---: M006 BASIC (f�r KC 85/2)
 *  ---: M007 Adapter (Herausf�hrung der Anschl�sse an die Frontseite)
 *  ---: M008 Joystick
 *  e7h: M010 ADU1
 *  f6h: M011 64 KByte RAM
 *  fbh: M012 Software: Texor
 *  ---: M021 Joy + Centronics
 *  f4h: M022 Expander RAM (16 KByte)
 *  f7h: M025 User PROM (8 KByte)
 *  fbh: M026 Software: Forth
 *  fbh: M027 Software: Development
 *  e3h: M029 DAU1
 *  ---: M030 EPROMmer
 *  ---: M032 256 KByte RAM
 *  ---: M033 Software: TYPESTAR
 *  ---: M036 128 KByte RAM
 *  ---: M040 USER PROM ? KByte
 *  ---: M053 RS 232
 *  ---: M125 USER ROM ? KByte
 */

#include <string.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/rc.h"
#include "kc/mod_ram.h"
#include "kc/mod_ram1.h"
#include "kc/mod_ram8.h"
#include "kc/mod_64k.h"
#include "kc/mod_rom.h"
#include "kc/mod_disk.h"
#include "kc/mod_list.h"

#ifdef TARGET_OS_LINUX
#include "kc/mod_v24.h"
#endif /* TARGET_OS_LINUX */

#include "ui/ui.h"

#include "libdbg/dbg.h"

ModuleList::ModuleList(void)
{
  int a;
  char *ptr;
  const char *mod;
  ModuleInterface *m;
  ModuleListEntry *entry;
  
  /*
   *  placeholder for a not present module ;-)
   */
  m = 0;
  _mod_list.push_back(new ModuleListEntry(_("<no module>"), m, KC_TYPE_ALL));

  /*
   *  V24 module
   */
#ifdef TARGET_OS_LINUX
  if (RC::instance()->get_int("Enable V24-Module"))
    {
      m = new ModuleV24("M003", 0xee);
      entry = new ModuleListEntry(_("M003: V24 (not working!)"), m, KC_TYPE_85_2_CLASS);
      _mod_list.push_back(entry);
    }
#endif /* TARGET_OS_LINUX */

  /*
   *  RAM module 64k
   */
  m = new Module64k("M011", 0xf6);
  _mod_list.push_back(new ModuleListEntry(_("M011: 64k RAM"), m, KC_TYPE_85_2_CLASS));
  
  /*
   *  RAM modules 1k at 2400h-27ffh,
   *                    2800h-2fffh,
   *                    3000h-3fffh,
   *                    4000h-7fffh,
   *                    8000h-ffffh
   *  (lc80)
   */
  m = new ModuleRAM8("RAM2400", 0x2400, 0x0400);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (1k/2400h)"), m, KC_TYPE_LC80));
  m = new ModuleRAM8("RAM2800", 0x2800, 0x0800);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (2k/2800h)"), m, KC_TYPE_LC80));
  m = new ModuleRAM8("RAM3000", 0x3000, 0x1000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (4k/3000h)"), m, KC_TYPE_LC80));
  m = new ModuleRAM8("RAM4000", 0x4000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/4000h)"), m, KC_TYPE_LC80));
  m = new ModuleRAM8("RAM8000", 0x8000, 0x8000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (32k/8000h)"), m, KC_TYPE_LC80));

  /*
   *  RAM modules 16k at 4000h and 8000h (kc85/1)
   */
  m = new ModuleRAM1("RAM4", 0x4000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/4000h)"), m, KC_TYPE_85_1_CLASS));
  m = new ModuleRAM1("RAM8", 0x8000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/8000h)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  IRM Expansion for color display (kc85/1)
   */
  m = new ModuleRAM1("IRMX", 0xe800, 0x0800);
  entry = new ModuleListEntry(_("IRM Color Expansion"), m, KC_TYPE_NONE);
  _mod_list.push_back(entry);
  _init_color_expansion = 0;
  if (get_kc_type() == KC_TYPE_87)
    _init_color_expansion = entry;

  /*
   *  RAM module 16k (kc85/3)
   */
  m = new ModuleRAM("M022", 0xf4);
  _mod_list.push_back(new ModuleListEntry(_("M022: Expander RAM (16k)"), m, KC_TYPE_85_2_CLASS));

  /*
   *  basic
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m006.rom");
  m = new ModuleROM(ptr, "Basic", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M006: Basic"), m, KC_TYPE_85_2_CLASS));
  delete ptr;

  /*
   *  texor
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m012.rom");
  m = new ModuleROM(ptr, "M012", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M012: Texor"), m, KC_TYPE_85_2_CLASS));
  delete ptr;

  /*
   *  forth
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m026.rom");
  m = new ModuleROM(ptr, "M026", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M026: Forth"), m, KC_TYPE_85_2_CLASS));
  delete ptr;

  /*
   *  development
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m027.rom");
  m = new ModuleROM(ptr, "M027", 0x2000, 0xfb);
  entry = new ModuleListEntry(_("M027: Development"), m, KC_TYPE_85_2_CLASS);
  _mod_list.push_back(entry);
  delete ptr;

  /*
   *  wordpro ROM version for kc85/3
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m900.rom");
  m = new ModuleROM(ptr, "M900", 0x2000, 0xfb);
  entry = new ModuleListEntry(_("M900: WordPro '86 (KC85/3)"), m, KC_TYPE_85_3);
  _mod_list.push_back(entry);
  delete ptr;

  /*
   *  wordpro ROM version for kc85/4
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m901.rom");
  m = new ModuleROM(ptr, "M901", 0x2000, 0xfb);
  entry = new ModuleListEntry(_("M901: WordPro '86 (KC85/4)"), m, KC_TYPE_85_4);
  _mod_list.push_back(entry);
  delete ptr;

  /*
   *  Floppy Disk Basis ROM
   */
  ptr = new char[strlen(kcemu_datadir) + 12];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/floppy.rom");
  m = new ModuleDisk(ptr, "Floppy Disk Basis", 0x2000, 0xa7);
  entry = new ModuleListEntry(_("Floppy Disk Basis"), m, KC_TYPE_NONE);
  _mod_list.push_back(entry);
  _init_floppy_basis = 0;
  if (RC::instance()->get_int("Floppy Disk Basis"))
    {
      _init_floppy_basis = entry;
    }
  delete ptr;

  _nr_of_bd = RC::instance()->get_int("Busdrivers");
  if (_nr_of_bd < 0)
    _nr_of_bd = 0;
  if (_nr_of_bd > MAX_BD)
    _nr_of_bd = MAX_BD;
  if (get_kc_type() & KC_TYPE_85_1_CLASS)
    _nr_of_bd = 0;
  if (get_kc_type() & KC_TYPE_LC80)
    _nr_of_bd = 1;

  for (a = 0;a < 4 * _nr_of_bd + 2;a++)
    {
      mod = RC::instance()->get_string_i(a, "Module");
      _init_mod[a] = mod;
    }
}

ModuleList::~ModuleList(void)
{
  ModuleListEntry *entry;
  ModuleList::iterator it;

  for (it = module_list->begin();it != module_list->end();it++)
    {
      entry = *it;
      delete entry->get_mod();
      delete entry;
    }
}

void
ModuleList::init(void)
{
  int a;
  ModuleInterface *m;
  ModuleList::iterator it;

  for (a = 0;a < 4 * _nr_of_bd + 2;a++)
    {
      if (!_init_mod[a]) continue;
      for (it = module_list->begin();it != module_list->end();it++)
        {
          m = (*it)->get_mod();
          if (!m) continue;
          if (strcmp(m->get_name(), _init_mod[a]) == 0)
            insert(a, *it);
        }
    }
  if (_init_floppy_basis)
    {
      /*
       *  floppy disk basis ROM is always in slot fch!
       */
      insert(61, _init_floppy_basis);
    }

  if (_init_color_expansion)
    {
      /*
       *  this is set if the emulator runs in KC 87 mode which means
       *  the color expansion is always present
       *  the KC 87 doesn't support the module slot notation like
       *  the KC 85/2 class we just insert it at slot fch like
       *  the floppy disk interface for KC 85/2 which is never available
       *  in KC 87 mode.
       */
      insert(61, _init_color_expansion);
    }
}

void
ModuleList::insert(int slot, ModuleListEntry *entry)
{
  ModuleInterface *m;
  
  /*
   *  the module object is deleted in module->insert()
   *  or module->remove()!
   */
  m = entry->get_mod();

  if (m)
    module->insert(slot, m->clone());
  else
    module->remove(slot);

  ui->getModuleInterface()->insert(slot, m);
  ui->getModuleInterface()->activate(slot, 0);
}
