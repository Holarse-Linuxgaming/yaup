/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * yaup.h
 *
 * Copyright (C) 2016 Lucki <lucki@holarse-linuxgaming.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __YAUP__
#define __YAUP__

#include <gtk/gtk.h>
#include <miniupnpc/miniupnpc.h>

#define GET_UI_ELEMENT(TYPE, ELEMENT)   TYPE *ELEMENT = (TYPE *) \
				yaup_get_ui_element(yaup, #ELEMENT);

typedef struct row_
{
  GtkListBoxRow		*row;
  GtkBox					*box;
  GtkSwitch 			*sw;
  GtkEntry				*entry_name;
	GtkEntry				*entry_ip;
	GtkCheckButton	*check_ip;
	GtkCheckButton	*check_iport;
	GtkSpinButton		*ispin;
  GtkSpinButton		*ospin;
  GtkComboBoxText	*combo;
	GtkButton				*bt_remove;
	GtkButton				*bt_up;
	GtkButton				*bt_down;
	GtkButton				*bt_duplicate;
	GtkMenuButton		*bt_menu;
	GtkMenuButton		*bt_iport;
  gboolean				enabled;
	gboolean				iport_enabled;
	gboolean				ip_enabled;
  char						name[80];
  int							iport;
  int							oport;
  char						protocol[10];
  int							index;
	char						ip[64];
} Row;

typedef struct yaup_
{
	GtkApplication	*application;
	GtkBuilder			*definitions;
	GSList					*objects;
	GSList					*rows;
	GSList					*controlURLs;

	const gchar			*config_file;
	const gchar			*config_dir;
	char						local_ip[64];
	char						controlURL[100];
	char						my_controlURL[100];
	char						externalIPAddress[64];

	struct UPNPDev	*devicelist;
	struct UPNPUrls urls;
	struct UPNPDev	*device;
	struct IGDdatas igddata;

	guint						statusbar_error_id;
	guint						statusbar_notice_id;

	gboolean				own_controlURL;

	Row							*row;

} Yaup;

void yaup_init (GApplication *, gpointer);
GtkWidget * create_window (Yaup * );
GObject *yaup_get_ui_element (Yaup *, const gchar * );
void found_devices (GObject      *source_object,
                    GAsyncResult *res,
                    gpointer      user_data);
void activate (GApplication *app,
               gpointer      user_data);
void app_shutdown (GApplication *app,
                   gpointer      user_data);
void
got_redirects (GObject      *source_object,
               GAsyncResult *res,
               gpointer      user_data);

#endif
