/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * callbacks.c
 *
 * Copyright (C) 2016 Lucki <lucki@holarse-linuxgaming.de>
 *
 * yaup is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * yaup is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <string.h>
#include <miniupnpc/upnpcommands.h>

#include "upnpcommunication.h"
#include "listboxthings.h"
#include "callbacks.h"
#include "yaup.h"
#include "io.h"
#include "yaup-i18n.h"
#include "debug.h"

/* Signal handlers */
/* Note: These may not be declared static because
 * signal autoconnection
 * only works with non-static methods
 */

/* Called when the window is closed */
void
on_window_destroy (GtkWidget *widget,
                   gpointer  *user_data)
{
	/* Nothing useful to do here because the shutdown function gets called later */
}

void
on_window_about_delete_event (GtkWidget *widget,
                              Yaup      *yaup)
{
	gtk_widget_hide(widget);
}

void
on_window_preferences_delete_event (GtkWidget *widget,
                                    gpointer  *user_data)
{
	gtk_widget_hide(widget);
}

void
on_window_about_response (GtkDialog *dialog,
                          gint       response_id,
                          Yaup      *yaup)
{
	switch(response_id)
		{
		case GTK_RESPONSE_CLOSE:
				{
					on_about_close_clicked(NULL, yaup);
					break;
				}
		}
}

void
on_window_preferences_response (GtkDialog *dialog,
                                gint       response_id,
                                Yaup      *yaup)
{
	switch(response_id)
		{
		case GTK_RESPONSE_CLOSE:
				{
					on_preferences_close_clicked(NULL, yaup);
					break;
				}
		}
}

void
on_about_close_clicked (GtkButton *button,
                        Yaup      *yaup)
{
	GET_UI_ELEMENT(GtkWidget, window_about);
	gtk_widget_hide(window_about);
}

void
on_preferences_close_clicked (GtkButton *button,
                              Yaup      *yaup)
{
	GET_UI_ELEMENT(GtkWidget, window_preferences);
	gtk_widget_hide(window_preferences);
}

void
on_add_clicked (GtkButton *button,
                Yaup      *yaup)
{
	Row *row;
	row = get_clean_list_box_row (yaup, -1, NULL);

	row = add_row_to_list_box(yaup, -1, row);

	row = add_row_to_gslist(yaup, row);

}

void
on_reset_control_url_clicked (GtkButton *button,
                              Yaup      *yaup)
{
	GET_UI_ELEMENT(GtkEntry, entry_control_url);
	GET_UI_ELEMENT(GtkToggleButton, check_control_url);

	gtk_entry_set_text(entry_control_url, yaup->urls.controlURL);
	gtk_toggle_button_set_active(check_control_url, FALSE);
}

void
on_duplicate_clicked (GtkButton *button,
                      Yaup      *yaup)
{
  Row	*row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(button))))))))))));

  add_list_item_from_config(FALSE,
														row->name,
														row->iport,
														row->ip,
														row->oport,
														row->oport2,
														row->protocol,
														row->index + 1,
														yaup);
}

void on_remove_clicked (GtkButton *button,
                        Yaup      *yaup)
{
	GtkListBoxRow   *box_row;
	Row							*row;

	GET_UI_ELEMENT(GtkListBox, listbox);

	box_row = gtk_list_box_get_selected_row (listbox);
	row = get_list_box_row (yaup, box_row);
	remove_list_item(row, yaup);
	/* gtk_container_remove (GTK_CONTAINER(listbox), GTK_WIDGET(row)); */
	gtk_widget_set_sensitive (GTK_WIDGET(button), FALSE);
}

void on_remove_row_clicked (GtkButton *button,
                            Yaup      *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(button))))))))))));

	remove_list_item(row, yaup);

	g_free(row);

	return;
}

void on_move_up_clicked (GtkButton *button,
                         Yaup      *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(button))))))))))));

	/* We can use the index of the row as it represents the index in the GSList */
	yaup->rows = g_slist_remove(yaup->rows, row);

	/* Now we can add it one item above */
	row->index = row->index - 1;
	yaup->rows = g_slist_insert(yaup->rows, row, row->index);

	refresh_listbox(yaup);

	/* As we can run to over the ends of the GSList we have to get the new real
	 * index for our row from the GSList/GtkListBox.
	 */
	row->index = gtk_list_box_row_get_index(row->row);

	gtk_button_clicked(GTK_BUTTON(row->bt_menu));

	return;
}

void on_move_down_clicked (GtkButton *button,
                           Yaup      *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(button))))))))))));

	/* We can use the index of the row as it represents the index in the GSList */
	yaup->rows = g_slist_remove(yaup->rows, row);

	/* Now we can add it one item above */
	row->index = row->index + 1;
	yaup->rows = g_slist_insert(yaup->rows, row, row->index);

	refresh_listbox(yaup);

	/* As we can run to over the ends of the GSList we have to get the new real
	 * index for our row from the GSList/GtkListBox.
	 */
	row->index = gtk_list_box_row_get_index(row->row);

	gtk_button_clicked (GTK_BUTTON(row->bt_menu));

	return;
}

void
on_reload_clicked(GtkButton *button,
                  Yaup      *yaup)
{
	GTask *task;

	GET_UI_ELEMENT (GtkSpinner, spinner);
	GET_UI_ELEMENT (GtkButton, add);
	GET_UI_ELEMENT (GtkStatusbar, statusbar);

	gtk_widget_set_sensitive(GTK_WIDGET(add),FALSE);
	gtk_spinner_start(spinner);
	gtk_statusbar_push(statusbar, yaup->statusbar_notice_id,
										 _("Searching for opened ports…"));

	/* Get available devices in the network in a new thread */
	task = g_task_new (NULL, NULL, upnp_list_redirections_finish, yaup);
	g_task_set_task_data (task, yaup, NULL);
	g_task_run_in_thread (task, upnp_list_redirections);
	g_object_unref (task);
}

void
on_info_bar_preferences_clicked (GtkButton *button,
                                 Yaup      *yaup)
{
	GET_UI_ELEMENT(GtkInfoBar, info_bar);

	/* This has the same effect as the menu item */
	on_preferences_clicked(NULL, yaup);

	/* Now that the user is informed we can hide this infobar.
	 * We aren't able to show it again!
	 * https://bugzilla.gnome.org/show_bug.cgi?id=710888
	 */
	gtk_widget_hide(GTK_WIDGET(info_bar));

	return;
}

void
on_preferences_clicked(GtkButton *button,
                       Yaup      *yaup)
{
	GET_UI_ELEMENT(GtkWindow, window_preferences);

	gtk_widget_show_all(GTK_WIDGET(window_preferences));

	return;
}

void
on_bt_about_clicked(GtkButton *button,
                    Yaup      *yaup)
{
	GET_UI_ELEMENT (GtkAboutDialog, window_about);

	gtk_widget_show_all(GTK_WIDGET(window_about));
}

gint
sort_func_name (gconstpointer a,
                gconstpointer b)
{
	/* < 0 if row1 should be before row2
	 * 0 if they are equal
	 * > 0 otherwise
	 */

	Row *arow = (Row*) a;
	Row *brow = (Row*) b;

	int r = strcmp(arow->name, brow->name);

	/* sort empty names to the end of the list */
	if(strcmp(arow->name, "") == 0
		 && r != 0)
		{
			return 1;
		}
	else if(strcmp(brow->name, "") == 0
					&& r != 0)
		{
			return -1;
		}

	/* sort for the first different letter */
	if(r < 0)
		{
			return -1;
		}
	else if(r > 0)
		{
			return 1;
		}
	else if(r == 0)
		{
			return 0;
		}

	/* We should never get here but the compiler wasn't statisfied
	 * without this return value
	 */
	return 1;
}

gint
sort_func_port (gconstpointer a,
                gconstpointer b)
{
	/* < 0 if row1 should be before row2
	 * 0 if they are equal
	 * > 0 otherwise
	 */

	Row *arow = (Row*) a;
	Row *brow = (Row*) b;

	if(arow->oport < brow->oport)
		{
			return -1;
		}
	else if(arow->oport > brow->oport)
		{
			return 1;
		}
	else if(arow->oport == brow->oport)
		{
			return 0;
		}

	/* We should never get here but the compiler wasn't statisfied
	 * without this return value
	 */
	return 1;
}

gint
sort_func_enabled (gconstpointer a,
                   gconstpointer b)
{
	/* < 0 if row1 should be before row2
	 * 0 if they are equal
	 * > 0 otherwise
	 */

	Row *arow = (Row*) a;
	Row *brow = (Row*) b;

	if(arow->enabled && !brow->enabled)
		{
			return -1;
		}
	else if(!arow->enabled && brow->enabled)
		{
			return 1;
		}
	else if(arow->enabled == brow->enabled)
		{
			return 0;
		}

	/* We should never get here but the compiler wasn't statisfied
	 * without this return value
	 */
	return 0;
}

void
on_bt_sort_enabled_clicked(GtkButton *button,
                           Yaup      *yaup)
{
	/* sort the GSList */
	yaup->rows = g_slist_sort (yaup->rows, sort_func_enabled);

	refresh_listbox(yaup);

	return;
}

void
on_bt_sort_name_clicked(GtkButton *button,
                        Yaup      *yaup)
{
	/* sort the GSList */
	yaup->rows = g_slist_sort (yaup->rows, sort_func_name);

	refresh_listbox(yaup);

	return;
}

void
on_bt_sort_port_clicked(GtkButton *button,
                        Yaup      *yaup)
{
	/* sort the GSList */
	yaup->rows = g_slist_sort (yaup->rows, sort_func_port);

	refresh_listbox(yaup);

	return;
}

void
on_listbox_row_selected (GtkListBox    *listbox,
                         GtkListBoxRow *row,
                         Yaup          *yaup)
{
	/* this shouldn't be relevant anymore since i've disabled selections
	 * For drag&drop this can become useful again*/

  Row *arow;

  GET_UI_ELEMENT (GtkButton, remove);
	GET_UI_ELEMENT (GtkButton, move_up);
	GET_UI_ELEMENT (GtkButton, move_down);
  GET_UI_ELEMENT (GtkButton, duplicate);

  arow = get_list_box_row(yaup, row);

	if(arow != NULL)
	{
		gtk_widget_set_sensitive (GTK_WIDGET(move_up), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(move_down), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(duplicate), TRUE);

		if( gtk_switch_get_active (arow->sw) == FALSE)
		{
		  gtk_widget_set_sensitive (GTK_WIDGET(remove), TRUE);
		}
		else
		{
		  gtk_widget_set_sensitive (GTK_WIDGET(remove), FALSE);
		}
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET(move_up), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(move_down), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(remove), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(duplicate), FALSE);
	}
}

/* Since toggling the switch activates this function again, we're
 * setting the state of the switch on our own (return TRUE) in the
 * disable_row and activate_row functions
 */
gboolean
on_switch_state_set (GtkSwitch *widget,
                     gboolean   state,
                     Yaup      *yaup)
{
  Row *row, *row2;
  GtkWidget *box;

  GET_UI_ELEMENT (GtkStatusbar, statusbar);
	GET_UI_ELEMENT (GtkSpinner, spinner);

	gtk_spinner_start(spinner);
	gtk_widget_grab_focus (GTK_WIDGET(widget));

	box = gtk_widget_get_parent (GTK_WIDGET(widget));
	row = get_list_box_row(yaup,
                         GTK_LIST_BOX_ROW(
                            gtk_widget_get_parent (GTK_WIDGET(box))));

	yaup->row = row;

	if(strcmp(row->name, "") == 0)
    {
      disable_row(row, yaup);

      gtk_statusbar_push(statusbar,
                       yaup->statusbar_error_id,
                       _("No description found"));

			g_timeout_add_seconds (5,
														 statusbar_refresh_error,
														 yaup);

      return TRUE;

    }
	else
    {
		  row2 = get_port_in_use (row,yaup);

      if(state == TRUE
				 && (row2 == NULL
						 || (row2 != NULL
								 && row2->enabled == FALSE)
						 )
				 )
        {
					GTask *task;

					/* Add in a new thread */
					task = g_task_new (NULL, NULL, upnp_set_redirect_finish, yaup);
					g_task_set_task_data (task, yaup, NULL);
					g_task_run_in_thread (task, upnp_set_redirect);
					g_object_unref (task);

        }
      else if (state == FALSE && row2 == NULL)
        {
					GTask *task;

					/* Remove in a new thread */
					task = g_task_new (NULL, NULL, upnp_remove_redirect_finish, yaup);
					g_task_set_task_data (task, yaup, NULL);
					g_task_run_in_thread (task, upnp_remove_redirect);
					g_object_unref (task);

        }
      else if (row2 != NULL )
        {
			    disable_row (row, yaup);

          char str[80];
          sprintf(str, _("Port and Protocol are already in use"));
          gtk_statusbar_push(statusbar,
                             yaup->statusbar_error_id,
                             str);
					gtk_spinner_stop(spinner);
        }
      else
		    {
			    disable_row (row, yaup);

          char str[80];
          sprintf(str, "How do we got here?");
          gtk_statusbar_push(statusbar,
                             yaup->statusbar_error_id,
                             str);

		    }

    }

	return TRUE;
}

void
on_menu_about_activate(GtkMenuItem *menuitem,
                       Yaup        *yaup)
{

}

void
on_check_iport_toggled (GtkToggleButton *togglebutton,
                        Yaup            *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(togglebutton))))))))))));

	if(gtk_toggle_button_get_active(togglebutton))
		{
			gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), TRUE);
		}
	else
		{
			row->iport = row->oport;
			gtk_spin_button_set_value(row->ispin,(gdouble) row->iport);
			gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), FALSE);
		}
}

void
on_check_ip_toggled (GtkToggleButton *togglebutton,
                     Yaup            *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(togglebutton))))))))))));

	if(gtk_toggle_button_get_active(togglebutton))
		{
			gtk_widget_set_sensitive(GTK_WIDGET(row->entry_ip), TRUE);
		}
	else
		{
			strcpy(row->ip, yaup->local_ip);
			gtk_entry_set_text(row->entry_ip, yaup->local_ip);
			gtk_widget_set_sensitive(GTK_WIDGET(row->entry_ip), FALSE);
		}
}

void on_check_range_toggled(GtkToggleButton *togglebutton,
                            Yaup            *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(togglebutton))))))))))));

	if(gtk_toggle_button_get_active(togglebutton))
		{
			row->oport2_enabled = TRUE;
			gtk_widget_set_sensitive(GTK_WIDGET(row->ospin2), TRUE);
		}
	else
		{
			row->oport2_enabled = FALSE;
			gtk_widget_set_sensitive(GTK_WIDGET(row->ospin2), FALSE);
		}
}

void
on_check_control_url_toggled (GtkToggleButton *togglebutton,
                              Yaup            *yaup)
{
	GET_UI_ELEMENT(GtkComboBoxText, combo_control_url);

	if(gtk_toggle_button_get_active(togglebutton))
		{
			gtk_widget_set_sensitive(GTK_WIDGET(combo_control_url), TRUE);
			yaup->own_controlURL = TRUE;
		}
	else
		{
			gtk_widget_set_sensitive(GTK_WIDGET(combo_control_url), FALSE);
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo_control_url), 0);
			yaup->own_controlURL = FALSE;
		}
}

void
on_ospin_button_value_changed(GtkSpinButton *spin_button,
                              Yaup          *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(spin_button))))));

	row->oport = gtk_spin_button_get_value_as_int(row->ospin);

	if(!row->iport_enabled && row->ispin != spin_button)
		{
			gtk_spin_button_set_value(row->ispin, row->oport);
			row->iport = row->oport;
		}

	if(!row->oport2_enabled)
		{
			gtk_spin_button_set_value(row->ospin2, row->oport);
			row->oport2 = row->oport;
		}
	else if(row->oport2_enabled && row->oport > row->oport2)
		{
			gtk_spin_button_set_value(row->ospin2, row->oport);
			row->oport2 = row->oport;
		}
}

void on_ospin2_button_value_changed(GtkSpinButton *spin_button,
                                    Yaup          *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent
																					(GTK_WIDGET
																					 (gtk_widget_get_parent
																						(GTK_WIDGET(spin_button)
																						 )
																						)
																					 )
																					)
												 );

	row->oport2 = gtk_spin_button_get_value_as_int(row->ospin2);

	if(row->oport2_enabled && row->oport2 < row->oport)
		{
			gtk_spin_button_set_value(row->ospin, row->oport2);
			row->oport = row->oport2;
		}
}

void
on_ispin_button_value_changed(GtkSpinButton *spin_button,
                              Yaup          *yaup)
{
	Row   *row;

	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(spin_button))))))))))));

	row->iport = gtk_spin_button_get_value_as_int(row->ispin);
}

void
on_info_bar_response (GtkInfoBar *info_bar,
                      gint        response_id,
                      Yaup       *yaup)
{
	switch(response_id)
		{
		case GTK_RESPONSE_CLOSE:
				{
					gtk_widget_hide(GTK_WIDGET(info_bar));
					break;
				}
			case 123:
				{
					on_preferences_clicked(NULL, yaup);
					gtk_widget_hide(GTK_WIDGET(info_bar));
					break;
				}
		}

}

void
on_combo_box_changed (GtkComboBox *combo,
                      Yaup        *yaup)
{
	Row *row;
	row = get_list_box_row(yaup,
                         GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(combo))))));

	strcpy(row->protocol, gtk_combo_box_text_get_active_text(row->combo));
}

void
on_combo_contorl_url_changed (GtkComboBox *combo,
                              Yaup        *yaup)
{
	strcpy(yaup->controlURL,
				 gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo)));
}

void
on_entry_name_changed (GtkEditable *editable,
                       Yaup        *yaup)
{
	Row *row;
	row = get_list_box_row(yaup,
                         GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(editable))))));

	strcpy(row->name, gtk_entry_get_text (row->entry_name));
}

void
on_entry_ip_changed (GtkEditable *editable,
                     Yaup        *yaup)
{
	Row *row;
	row = get_list_box_row(yaup,
												 GTK_LIST_BOX_ROW(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(gtk_popover_get_relative_to(
													GTK_POPOVER(gtk_widget_get_parent(
													GTK_WIDGET(gtk_widget_get_parent(
													GTK_WIDGET(editable))))))))))));

	strcpy(row->ip, gtk_entry_get_text(row->entry_ip));
}

void
on_entry_control_url_changed (GtkEditable *editable,
                              Yaup        *yaup)
{
	GET_UI_ELEMENT(GtkEntry, entry_control_url);

	strcpy(yaup->controlURL, gtk_entry_get_text(entry_control_url));
}
