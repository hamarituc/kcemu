/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ic.h,v 1.8 2002/01/20 13:39:29 torsten_paul Exp $
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

#ifndef __kc_ic_h
#define __kc_ic_h

#include <malloc.h>
#include <string.h>
#include <iostream.h>

#include "kc/config.h"
#include "kc/system.h"

#define IRQ_NOT_ACK (0x100)

class InterfaceCircuit
{
private:

  /**
   *  input value of the daisy chain.
   */
  int _iei;

  /**
   *  will be set between reti_ED() and reti_4D() by elements
   *  of the daisy chain that have an interrupt pending but
   *  not acknowledged
   */
  int _ieo_reti;

  /**
   *  interrupt is requested but not yet aknowledged by the cpu
   */
  int _irqreq;

  /**
   *  interrupt is aknowledged and the interrupt service routine
   *  is still running
   */
  int _irqactive;

  char *_name;
  InterfaceCircuit *_next;
  InterfaceCircuit *_prev;

public:
  InterfaceCircuit(const char *name);
  virtual ~InterfaceCircuit(void);

  virtual void debug(void);
  virtual const char * const get_ic_name();

  virtual void reti(void) = 0;
  virtual void irqreq(void) = 0;
  virtual word_t irqack(void) = 0;

  virtual void irq(void);
  virtual word_t ack(void);
  virtual void iei(byte_t val);
  virtual byte_t ieo(void);

  virtual void prev(InterfaceCircuit *ic);
  virtual void next(InterfaceCircuit *ic);
  virtual InterfaceCircuit * get_first();
  virtual InterfaceCircuit * get_last();

  virtual void reti_ED(void);
  virtual void reti_4D(void);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_ic_h */
