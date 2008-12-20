/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: reset.cc,v 1.5 2002/10/31 16:40:02 torsten_paul Exp $
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

#include "kc/system.h"

#include "kc/z80.h"
#include "ui/status.h"
#include "cmd/cmd.h"

class CMD_reset : public CMD
{
public:
  
  CMD_reset(void) : CMD("Reset")
    {
      register_cmd("emu-reset", 0);
      register_cmd("emu-power-on", 1);
      register_cmd("emu-nmi", 2);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
        {
        case 0:
          Status::instance()->setMessage("*** RESET ***");
          z80->reset();
          break;
        case 1:
          Status::instance()->setMessage("*** POWER ON ***");
          z80->power_on();
	  break;
	case 2:
          Status::instance()->setMessage("*** NMI ***");
	  z80->handleIrq(0x0066);
	  break;
        }
    }
};

__force_link(CMD_reset);
