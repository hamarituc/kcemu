/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: window.h,v 1.4 2002/10/31 01:38:07 torsten_paul Exp $
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

#ifndef __ui_gtk_window_h
#define __ui_gtk_window_h

#include <gtk/gtk.h>

#include "ui/window.h"

class UI_Gtk_Window : public UI_Window
{
 private:
  bool _visible;
 protected:
  GtkWidget *_window;

  virtual void init(void) = 0;

 public:
  UI_Gtk_Window(void) { _window = 0; _visible = false; }
  virtual ~UI_Gtk_Window(void) {}

  void show(void);
  void hide(void);
  void toggle(void);

  bool is_visible(void);

  GtkWidget * create_pixmap_widget(GtkWidget *parent, char **data);
  GtkWidget * create_button_with_pixmap(GtkWidget *parent, char **data);
};

#endif /* __ui_gtk_window_h */
