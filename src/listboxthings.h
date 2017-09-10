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

Row * get_clean_list_box_row (Yaup *yaup,
                              int   index,
                              Row  *row);

Row * add_list_item_from_row (Row  *row,
                              Yaup *yaup,
                              int   index);

Row * add_list_item_from_config (gboolean  enabled,
                                 char     *name,
                                 gdouble   iport,
                                 char      *ip,
                                 gdouble   oport,
																 gdouble   oport2,
                                 char     *protocol,
                                 int       index,
                                 Yaup     *yaup);

Row * add_row_to_list_box(Yaup *yaup,
                          int   index,
                          Row  *row);

Row * add_row_to_gslist(Yaup *yaup,
                        Row  *row);

Row * set_list_box_row (Yaup     *yaup,
                        Row      *row,
                        gboolean  enabled,
                        char     *name,
                        gdouble   iport,
                        char     *ip,
                        gdouble   oport,
                        gdouble   oport2,
                        char     *protocol,
                        int       index);

Row *
set_list_widget_values(Yaup *yaup,
											 Row *row);

// These don't work, I don't know why. TODO: Figure it out.
/* gint find_same_row(gconstpointer, gconstpointer); */
/* gint find_opposite_row(gconstpointer, gconstpointer); */
/* gint find_row_with_both(gconstpointer, gconstpointer); */

void activate_row (Row  *row,
                   Yaup *yaup);

void disable_row (Row  *row,
                  Yaup *yaup);

Row * get_port_in_use (Row  *row,
                       Yaup *yaup);

void remove_list_item (Row  *row,
                       Yaup *yaup);

void delete_row (GtkWidget *widget,
                 gpointer   data);

void refresh_listbox (Yaup *yaup);

void clean_up_list_box(Yaup *yaup);
