/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: window.cc,v 1.4 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "ui/gtk/window.h"

void
UI_Gtk_Window::show(void)
{
  if (_visible) return;
  gtk_widget_show(_window);
  _visible = true;
}

void
UI_Gtk_Window::hide(void)
{
  if (!_visible) return;
  gtk_widget_hide(_window);
  _visible = false;
}

void
UI_Gtk_Window::toggle(void)
{
  if (_visible)
    hide();
  else
    show();
}

bool
UI_Gtk_Window::is_visible(void)
{
  return _visible;
}

GtkWidget *
UI_Gtk_Window::create_pixmap_widget(GtkWidget *parent, char **data)
{
  GtkStyle  *style;
  GdkPixmap *pixmap;
  GdkBitmap *pixmap_mask;
  GtkWidget *pixmap_widget;

  style = gtk_widget_get_style(parent);
  pixmap = gdk_pixmap_create_from_xpm_d(parent->window,
					&pixmap_mask,
					&style->bg[GTK_STATE_NORMAL],
					data);
  pixmap_widget = gtk_pixmap_new(pixmap, pixmap_mask);
  return pixmap_widget;
}

GtkWidget *
UI_Gtk_Window::create_button_with_pixmap(GtkWidget *parent, char **data)
{
  GtkWidget *button;
  GtkWidget *pixmap;

  button = gtk_button_new();
  gtk_container_set_border_width(GTK_CONTAINER(button), 0);
  pixmap = create_pixmap_widget(parent, data);
  gtk_widget_show(pixmap);
  gtk_container_add(GTK_CONTAINER(button), pixmap);

  return button;
}

