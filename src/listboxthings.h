/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * listboxthings.h
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

#include "yaup.h"

Row * get_list_box_row (Yaup          *,
                        GtkListBoxRow *);

Row * add_list_item_from_row (Row  *row,
                              Yaup *yaup,
                              int   index);

Row * add_list_item_from_config (gboolean  enabled,
                                 char     *name,
                                 gdouble   iport,
                                 char      *ip,
                                 gdouble   oport,
                                 char     *protocol,
                                 int       index,
                                 Yaup     *yaup);

void set_list_box_row (Yaup     *yaup,
                       Row      *row,
                       gboolean  enabled,
                       char     *name,
                       gdouble   iport,
                       char     *ip,
                       gdouble   oport,
                       char     *protocol);

Row * add_list_item (Yaup *yaup,
                     int   index);

Row * get_exact_row (Row  *row,
                     Yaup *yaup);

void activate_row (Row  *row,
                   Yaup *yaup);

void disable_row (Row  *row,
                  Yaup *yaup);

Row * get_port_in_use (Row  *row,
                       Yaup *yaup);

Row * get_counterpart (Row  *row,
                       Yaup *yaup);

Row * get_udp_tcp (Row  *row,
                   Yaup *yaup);

void remove_list_item (Row  *row,
                       Yaup *yaup);

void delete_row (GtkWidget *widget,
                 gpointer   data);

void refresh_listbox (Yaup *yaup);
