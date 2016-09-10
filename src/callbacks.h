/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * callbacks.h
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

#include <gtk/gtk.h>

#include "yaup.h"

void on_window_destroy (GtkWidget *widget,
                        gpointer      *user_data);
void on_window_about_delete_event (GtkWidget *widget,
                                   Yaup      *yaup);
void on_window_preferences_delete_event (GtkWidget *widget,
                                         gpointer  *user_data);
void on_add_clicked (GtkButton *button,
                     Yaup      *yaup);
void on_remove_clicked (GtkButton *button,
                        Yaup      *yaup);
void on_remove_row_clicked (GtkButton *button,
                            Yaup      *yaup);
void on_move_up_clicked (GtkButton *button,
                         Yaup      *yaup);
void on_move_down_clicked (GtkButton *button,
                           Yaup      *yaup);
void on_reload_clicked (GtkButton *button,
                        Yaup      *yaup);
void on_preferences_clicked(GtkButton *button,
                            Yaup      *yaup);
void on_bt_about_clicked(GtkButton *button,
                      Yaup      *yaup);
void on_duplicate_clicked (GtkButton *button,
                           Yaup      *yaup);
void on_listbox_row_selected (GtkListBox    *listbox,
                              GtkListBoxRow *row,
                              Yaup          *yaup);
gboolean on_switch_state_set (GtkSwitch *widget,
                              gboolean   state,
                              Yaup      *yaup);
void on_menu_about_activate(GtkMenuItem *menuitem, Yaup *yaup);

void on_check_iport_toggled (GtkToggleButton *togglebutton,
                             Yaup            *yaup);
void on_check_ip_toggled (GtkToggleButton *togglebutton,
                          Yaup            *yaup);
void on_check_control_url_toggled (GtkToggleButton *togglebutton,
                                   Yaup            *yaup);
void on_ispin_button_value_changed(GtkSpinButton *spin_button,
                                  Yaup           *yaup);
void on_ospin_button_value_changed(GtkSpinButton *spin_button,
                                  Yaup           *yaup);
void on_info_bar_response (GtkInfoBar *info_bar,
                           gint        response_id,
                           Yaup       *yaup);
void on_combo_box_changed (GtkComboBox *combo, Yaup *yaup);
void on_combo_contorl_url_changed (GtkComboBox *combo, Yaup *yaup);
void on_entry_ip_changed (GtkEditable *editable, Yaup *yaup);
void on_entry_name_changed (GtkEditable *editable, Yaup *yaup);
void on_entry_control_url_changed (GtkEditable *editable, Yaup *yaup);
void on_reset_control_url_clicked (GtkButton *button, Yaup *yaup);
void on_about_close_clicked (GtkButton *button, Yaup *yaup);
void on_preferences_close_clicked (GtkButton *button, Yaup *yaup);
void on_bt_sort_enabled_clicked(GtkButton *button, Yaup *yaup);
void on_bt_sort_name_clicked(GtkButton *button, Yaup *yaup);
void on_bt_sort_port_clicked(GtkButton *button, Yaup *yaup);
void on_window_about_response (GtkDialog *dialog,
                               gint       response_id,
                               Yaup      *yaup);
void on_window_preferences_response (GtkDialog *dialog,
                                     gint       response_id,
                                     Yaup      *yaup);
