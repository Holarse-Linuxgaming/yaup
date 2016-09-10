/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * listboxthings.c
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

#include <string.h>

#include "listboxthings.h"
#include "callbacks.h"
#include "yaup-i18n.h"
#include "debug.h"

Row *
get_list_box_row (Yaup          *yaup,
                  GtkListBoxRow *row)
{
  Row	 *new_row;
	int	 i;

  if(row == NULL)
    {return NULL;}

	i = gtk_list_box_row_get_index(row);
	new_row = g_slist_nth_data (yaup->rows, i);

  return new_row;
}

/* This predefines some values for the Row struct so that two Rows can
 * easily be compared.
 * We can later add this Row to the listbox with add_list_item_from_row.
 * But we can't access the widgets of the ListBoxRow yet!
 */
void
set_list_box_row (Yaup     *yaup,
                  Row      *row,
                  gboolean  enabled,
                  char     *name,
                  gdouble   iport,
									char			*ip,
                  gdouble   oport,
                  char     *protocol)
{
  row->enabled = enabled;
  strcpy(row->name,name);
  row->iport = iport;
	strcpy(row->ip, ip);
	if(iport != oport)
		{
				row->iport_enabled = TRUE;
		}
	else
		{
			row->iport_enabled = FALSE;
		}

  row->oport = oport;
  strcpy(row->protocol,protocol);
	/* row->index = -1; */


	//TODO: return row
  return;
}

/* This function reads the values of an existing Row and forms an ListBoxRow
 * according to that values. It stores the widgets in the Row to be able to
 * access them later through the GSList with all Rows.
 * This combines add_list_item and add_list_item_from_config so it's very
 * redundant.
 * TODO: clean up/combine all three functions.
 * TODO: Use templates.
 */
Row *
add_list_item_from_row (Row *row, Yaup *yaup, int index)
{
	GET_UI_ELEMENT (GtkListBox, listbox);

	int							type = 0;
	GtkBox					*box, *box2;
	GtkWidget				*popover_menu;
	GtkWidget				*popover_iport;
	GtkStyleContext *context;

	/* row = add_list_item (yaup, index); */

	row->row = GTK_LIST_BOX_ROW(gtk_list_box_row_new ());
  row->box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL,6));
  row->sw = GTK_SWITCH(gtk_switch_new ());
  row->entry_name = GTK_ENTRY(gtk_entry_new ());
	row->entry_ip = GTK_ENTRY(gtk_entry_new ());
  row->ispin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0,65535,1));
  row->ospin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0,65535,1));
  row->combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new ());
	row->bt_menu = GTK_MENU_BUTTON(gtk_menu_button_new());
	row->bt_iport = GTK_MENU_BUTTON(gtk_menu_button_new());
	row->bt_up = GTK_BUTTON(gtk_button_new_from_icon_name ("go-up",
																												 GTK_ICON_SIZE_BUTTON));
	row->bt_down = GTK_BUTTON(
									gtk_button_new_from_icon_name ("go-down",
																								 GTK_ICON_SIZE_BUTTON));
	row->bt_duplicate = GTK_BUTTON(
											gtk_button_new_from_icon_name ("edit-copy",
																										 GTK_ICON_SIZE_BUTTON));
	row->bt_remove = GTK_BUTTON(
										gtk_button_new_from_icon_name ("edit-delete",
																									 GTK_ICON_SIZE_BUTTON));
	row->check_iport = GTK_CHECK_BUTTON(
											gtk_check_button_new_with_label(_("Use different "
																												"internal port")));
	row->check_ip = GTK_CHECK_BUTTON(
									gtk_check_button_new_with_label(_("Use different IP "
																										"to redirect to")));

	gtk_widget_set_tooltip_text (GTK_WIDGET(row->sw), _("Activate this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->entry_name),
															 _("Set a description for this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->entry_ip),
															 _("Set a different IP than your "
																	"own for the portmapping."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->ispin),
															 _("Set a different internal port "
																	"than the external port."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->ospin),
															 _("Set the port for the portmapping."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->combo),
															 _("Set the protocol for the mapping.\n"
																	"Most of the time you'd probably want "
																	"'TCP & UDP'."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_menu),
															 _("Show options which influence this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_iport),
															 _("Show advanced options for "
																	"this portmapping."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_up),
															 _("Move this row one line up."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_down),
															 _("Move this row one line down."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_duplicate),
															 _("Duplicate this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_remove),
															 _("Remove this row from the list."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->check_iport),
															 _("Activate the option to specify a "
																	"different internal port."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->check_ip),
															 _("Activate the option to specify a "
																	"different IP."));

	/* Here we're using a fancy gnome theme styling option.
	 * If the theme has set this, the button appears in a special color.
	 * In Adwaite it's red.
	 * Same goes in green for the add Button but that's set in the UI_FILE.
	 */
	context = gtk_widget_get_style_context(GTK_WIDGET(row->bt_remove));
  gtk_style_context_add_class (context, "destructive-action");


	/* Pack all toghether */
	/* Generate popover menu ingredients */
	box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,4));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_up));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_down));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_duplicate));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_remove));

	/* Generate popover iport ingredients */
	box2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,4));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->check_iport));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->ispin));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->check_ip));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->entry_ip));
	gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(row->entry_ip), FALSE);

	/* Generate popover menu	*/
  popover_menu = gtk_popover_new (GTK_WIDGET(row->bt_menu));
  gtk_popover_set_position (GTK_POPOVER (popover_menu), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (popover_menu), GTK_WIDGET(box));
  gtk_container_set_border_width (GTK_CONTAINER (popover_menu), 5);
	gtk_container_set_border_width(GTK_CONTAINER(row->check_iport), 5);
	gtk_container_set_border_width(GTK_CONTAINER(row->check_ip), 5);
  gtk_widget_show_all (GTK_WIDGET(box));
	gtk_menu_button_set_popover(row->bt_menu, popover_menu);

	/* Generate popover iport	*/
  popover_iport = gtk_popover_new (GTK_WIDGET(row->bt_iport));
  gtk_popover_set_position (GTK_POPOVER (popover_iport), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (popover_iport), GTK_WIDGET(box2));
  gtk_container_set_border_width (GTK_CONTAINER (popover_iport), 5);
  gtk_widget_show_all (GTK_WIDGET(box2));
	gtk_menu_button_set_popover(row->bt_iport, popover_iport);

  gtk_widget_set_hexpand (GTK_WIDGET(row->entry_name), TRUE);
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->sw));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->entry_name));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->bt_iport));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->ospin));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->combo));
	gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->bt_menu));
	gtk_container_set_border_width (GTK_CONTAINER (row->box), 5);

  /* We can set text to the ComboBoxText but we can't set the active item
	 * to a specific entry from the combos list with text.
	 * So we set them later through these values with gtk_combo_box_set_active.
   * TCP & UDP = 0
   * TCP = 1
   * UDP = 2
   */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (row->combo), "TCP & UDP");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (row->combo), "TCP");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (row->combo), "UDP");
  /* gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "ESP"); */
  /* gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "GRE"); */
  /* gtk_combo_box_set_active (GTK_COMBO_BOX(row->combo), 0); */

  gtk_entry_set_placeholder_text (GTK_ENTRY(row->entry_name), _("Description"));
	gtk_entry_set_placeholder_text (GTK_ENTRY(row->entry_ip), _("127.0.0.1"));
	/* gtk_entry_set_text(GTK_ENTRY(row->entry_ip), yaup->local_ip); */
	/* strcpy(row->ip, yaup->local_ip); */

  gtk_switch_set_active (row->sw,FALSE);

	/* Here we're connecting the signal handlers to our functions.
	 * For the UI_FILE this is done automatically by the builder.
	 */
  g_signal_connect(G_OBJECT(row->sw),
				   "state-set",
				   G_CALLBACK(on_switch_state_set),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_remove),
				   "clicked",
				   G_CALLBACK(on_remove_row_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_up),
				   "clicked",
				   G_CALLBACK(on_move_up_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_down),
				   "clicked",
				   G_CALLBACK(on_move_down_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_duplicate),
				   "clicked",
				   G_CALLBACK(on_duplicate_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->check_iport),
				   "toggled",
				   G_CALLBACK(on_check_iport_toggled),
				   yaup);
	g_signal_connect(G_OBJECT(row->check_ip),
				   "toggled",
				   G_CALLBACK(on_check_ip_toggled),
				   yaup);
	g_signal_connect(G_OBJECT(row->ospin),
				   "value-changed",
				   G_CALLBACK(on_ospin_button_value_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->ispin),
				   "value-changed",
				   G_CALLBACK(on_ispin_button_value_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->combo),
				   "changed",
				   G_CALLBACK(on_combo_box_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->entry_ip),
				   "changed",
				   G_CALLBACK(on_entry_ip_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->entry_name),
				   "changed",
				   G_CALLBACK(on_entry_name_changed),
				   yaup);

	/* Pack the GtkBox into the GtkListBoxRow */
	gtk_container_add (GTK_CONTAINER(row->row), GTK_WIDGET(row->box));

  /* Pack the GtkListBoxRow into the GtkListBox */
  gtk_list_box_insert (listbox, GTK_WIDGET(row->row), index);

	/* Here we're blocking the signal handlers we've connected above so they don't
	 * call their corresponding functions when we're changing the widgets state.
	 */
	g_signal_handlers_block_by_func (G_OBJECT(row->entry_name),
								   on_entry_name_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->entry_ip),
								   on_entry_ip_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->ispin),
								   on_ispin_button_value_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->ospin),
								   on_ospin_button_value_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->combo),
								   on_combo_box_changed,
								   yaup);

  if (strcmp (row->protocol, "TCP & UDP") == 0)
		{
			type = 0;
		}
  else if (strcmp (row->protocol, "TCP") == 0)
		{
			type = 1;
		}
  else if (strcmp (row->protocol, "UDP") == 0)
		{
			type = 2;
		}
	else
		{
			type = 0;
			strcpy(row->protocol, "TCP & UDP");
		}

	if(row->iport != row->oport)
		{
			/* gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), TRUE); */
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_iport), TRUE);
		}
	else
		{
			/* gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), FALSE); */
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_iport), FALSE);
		}

	if(strcmp(yaup->local_ip, row->ip) != 0)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_ip), TRUE);
		}
	else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_ip), FALSE);
		}

	/* Now it's safe to change values and states */
  gtk_combo_box_set_active (GTK_COMBO_BOX(row->combo), type);
  gtk_entry_set_text (row->entry_name, row->name);
	gtk_entry_set_text (row->entry_ip, row->ip);
  gtk_spin_button_set_value (row->ispin, row->iport);
  gtk_spin_button_set_value (row->ospin, row->oport);

	/* And now it's time to unblock them so it reacts to the users input. */
	g_signal_handlers_unblock_by_func (G_OBJECT(row->entry_name),
								   on_entry_name_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->entry_ip),
								   on_entry_ip_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->ispin),
								   on_ispin_button_value_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->ospin),
								   on_ospin_button_value_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->combo),
								   on_combo_box_changed,
								   yaup);

  if(row->enabled)
	{
	  activate_row (row, yaup);
	}
  else// if(!row->enabled)
	{
	  disable_row (row, yaup);
	}

	/* Make the row visible to the user */
	gtk_widget_show_all (GTK_WIDGET(row->row));

	return row;
}

Row *
add_list_item_from_config (gboolean  enabled,
                           char     *name,
                           gdouble   iport,
                           char     *ip,
                           gdouble   oport,
                           char     *protocol,
                           int       index,
                           Yaup     *yaup)
{
  Row	*row;
  int	type = 0;

  row = add_list_item (yaup, index);
  set_list_box_row (yaup, row, enabled, name, iport, ip, oport, protocol);

	row->index = gtk_list_box_row_get_index(row->row);

	g_signal_handlers_block_by_func (G_OBJECT(row->entry_name),
								   on_entry_name_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->entry_ip),
								   on_entry_ip_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->ispin),
								   on_ispin_button_value_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->ospin),
								   on_ospin_button_value_changed,
								   yaup);
	g_signal_handlers_block_by_func (G_OBJECT(row->combo),
								   on_combo_box_changed,
								   yaup);

  if (strcmp (protocol, "TCP & UDP") == 0)
		{
			type = 0;
		}
  else if (strcmp (protocol, "TCP") == 0)
		{
			type = 1;
		}
  else if (strcmp (protocol, "UDP") == 0)
		{
			type = 2;
		}
	else
		{
			type = 0;
			strcpy(row->protocol, "TCP & UDP");
		}

	if(iport != oport)
		{
			/* gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), TRUE); */
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_iport), TRUE);
		}
	else
		{
			/* gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), FALSE); */
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_iport), FALSE);
		}

	if(strcmp(yaup->local_ip, row->ip) != 0)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_ip), TRUE);
		}
	else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->check_ip), FALSE);
		}


  gtk_combo_box_set_active (GTK_COMBO_BOX(row->combo), type);
  gtk_entry_set_text (row->entry_name, row->name);
	gtk_entry_set_text (row->entry_ip, row->ip);
  gtk_spin_button_set_value (row->ispin, row->iport);
  gtk_spin_button_set_value (row->ospin, row->oport);

	g_signal_handlers_unblock_by_func (G_OBJECT(row->entry_name),
								   on_entry_name_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->entry_ip),
								   on_entry_ip_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->ispin),
								   on_ispin_button_value_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->ospin),
								   on_ospin_button_value_changed,
								   yaup);
	g_signal_handlers_unblock_by_func (G_OBJECT(row->combo),
								   on_combo_box_changed,
								   yaup);

  if(enabled)
	{
	  activate_row (row, yaup);
	}
  else if(!enabled)
	{
	  disable_row (row, yaup);
	}

  return (row);
}

Row *
add_list_item (Yaup *yaup,
               int   index)
{
  GET_UI_ELEMENT (GtkListBox, listbox);

  Row							*row;
	GtkBox					*box, *box2;
	GtkWidget				*popover_menu;
	GtkWidget				*popover_iport;
	GtkStyleContext *context;

  row = g_new (Row, 1);

	row->enabled = FALSE;
	row->iport_enabled = FALSE;
	row->ip_enabled = FALSE;
  strcpy(row->name, "");
  row->iport = 0;
  row->oport = 0;
  strcpy(row->protocol, "TCP & UDP");
  row->index = -1;
	strcpy(row->ip, yaup->local_ip);

  row->row = GTK_LIST_BOX_ROW(gtk_list_box_row_new ());
  row->box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL,6));
  row->sw = GTK_SWITCH(gtk_switch_new ());
  row->entry_name = GTK_ENTRY(gtk_entry_new ());
	row->entry_ip = GTK_ENTRY(gtk_entry_new ());
  row->ispin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0,65535,1));
  row->ospin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0,65535,1));
  row->combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new ());
	row->bt_menu = GTK_MENU_BUTTON(gtk_menu_button_new());
	row->bt_iport = GTK_MENU_BUTTON(gtk_menu_button_new());
	row->bt_up = GTK_BUTTON(gtk_button_new_from_icon_name ("go-up",
																												 GTK_ICON_SIZE_BUTTON));
	row->bt_down = GTK_BUTTON(
									gtk_button_new_from_icon_name ("go-down",
																								 GTK_ICON_SIZE_BUTTON));
	row->bt_duplicate = GTK_BUTTON(
											gtk_button_new_from_icon_name ("edit-copy",
																										 GTK_ICON_SIZE_BUTTON));
	row->bt_remove = GTK_BUTTON(
										gtk_button_new_from_icon_name ("edit-delete",
																									 GTK_ICON_SIZE_BUTTON));
	row->check_iport = GTK_CHECK_BUTTON(
											gtk_check_button_new_with_label(_("Use different "
																													"internal port")));
	row->check_ip = GTK_CHECK_BUTTON(
									gtk_check_button_new_with_label(_("Use different IP to "
																											"redirect to")));

	gtk_widget_set_tooltip_text (GTK_WIDGET(row->sw), _("Activate this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->entry_name),
															 _("Set a description for this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->entry_ip),
															 _("Set a different IP than your own for the "
																	"portmapping."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->ispin),
															 _("Set a different internal port than the "
																	"external port."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->ospin),
															 _("Set the port for the portmapping."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->combo),
															 _("Set the protocol for the mapping.\n"
																	"Most of the time you'd probably want "
																	"'TCP & UDP'."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_menu),
															 _("Show options which influence this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_iport),
															 _("Show advanced options for this "
																	"portmapping."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_up),
															 _("Move this row one line up."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_down),
															 _("Move this row one line down."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_duplicate),
															 _("Duplicate this row."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->bt_remove),
															 _("Remove this row from the list."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->check_iport),
															 _("Activate the option to specify a "
																	"different internal port."));
	gtk_widget_set_tooltip_text (GTK_WIDGET(row->check_ip),
															 _("Activate the option to specify a "
																	"different IP."));

	context = gtk_widget_get_style_context (GTK_WIDGET(row->bt_remove));
  gtk_style_context_add_class (context, "destructive-action");

	/* Generate popover menu ingredients */
	box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,4));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_up));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_down));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_duplicate));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(row->bt_remove));

	/* Generate popover iport ingredients */
	box2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,4));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->check_iport));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->ispin));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->check_ip));
	gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(row->entry_ip));
	gtk_widget_set_sensitive(GTK_WIDGET(row->ispin), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(row->entry_ip), FALSE);

	/* Generate popover menu	*/
  popover_menu = gtk_popover_new (GTK_WIDGET(row->bt_menu));
  gtk_popover_set_position (GTK_POPOVER (popover_menu), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (popover_menu), GTK_WIDGET(box));
  gtk_container_set_border_width (GTK_CONTAINER (popover_menu), 5);
	gtk_container_set_border_width(GTK_CONTAINER(row->check_iport), 5);
	gtk_container_set_border_width(GTK_CONTAINER(row->check_ip), 5);
  gtk_widget_show_all (GTK_WIDGET(box));
	gtk_menu_button_set_popover(row->bt_menu, popover_menu);

	/* Generate popover iport	*/
  popover_iport = gtk_popover_new (GTK_WIDGET(row->bt_iport));
  gtk_popover_set_position (GTK_POPOVER (popover_iport), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (popover_iport), GTK_WIDGET(box2));
  gtk_container_set_border_width (GTK_CONTAINER (popover_iport), 5);
  gtk_widget_show_all (GTK_WIDGET(box2));
	gtk_menu_button_set_popover(row->bt_iport, popover_iport);

  gtk_widget_set_hexpand (GTK_WIDGET(row->entry_name), TRUE);
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->sw));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->entry_name));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->bt_iport));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->ospin));
  gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->combo));
	gtk_container_add (GTK_CONTAINER(row->box), GTK_WIDGET(row->bt_menu));
	gtk_container_set_border_width (GTK_CONTAINER (row->box), 5);

  /*
   * TCP & UDP = 0
   * TCP = 1
   * UDP = 2
   */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (row->combo), "TCP & UDP");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (row->combo), "TCP");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (row->combo), "UDP");
  //gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "ESP");
  //gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "GRE");
  gtk_combo_box_set_active (GTK_COMBO_BOX(row->combo), 0);

  gtk_entry_set_placeholder_text (GTK_ENTRY(row->entry_name), _("Description"));
	gtk_entry_set_placeholder_text (GTK_ENTRY(row->entry_ip), _("127.0.0.1"));
	gtk_entry_set_text(GTK_ENTRY(row->entry_ip), yaup->local_ip);
	strcpy(row->ip, yaup->local_ip);

  gtk_switch_set_active (row->sw,FALSE);
  g_signal_connect(G_OBJECT(row->sw),
				   "state-set",
				   G_CALLBACK(on_switch_state_set),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_remove),
				   "clicked",
				   G_CALLBACK(on_remove_row_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_up),
				   "clicked",
				   G_CALLBACK(on_move_up_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_down),
				   "clicked",
				   G_CALLBACK(on_move_down_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->bt_duplicate),
				   "clicked",
				   G_CALLBACK(on_duplicate_clicked),
				   yaup);
	g_signal_connect(G_OBJECT(row->check_iport),
				   "toggled",
				   G_CALLBACK(on_check_iport_toggled),
				   yaup);
	g_signal_connect(G_OBJECT(row->check_ip),
				   "toggled",
				   G_CALLBACK(on_check_ip_toggled),
				   yaup);
	g_signal_connect(G_OBJECT(row->ospin),
				   "value-changed",
				   G_CALLBACK(on_ospin_button_value_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->ispin),
				   "value-changed",
				   G_CALLBACK(on_ispin_button_value_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->combo),
				   "changed",
				   G_CALLBACK(on_combo_box_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->entry_ip),
				   "changed",
				   G_CALLBACK(on_entry_ip_changed),
				   yaup);
	g_signal_connect(G_OBJECT(row->entry_name),
				   "changed",
				   G_CALLBACK(on_entry_name_changed),
				   yaup);

	gtk_container_add (GTK_CONTAINER(row->row), GTK_WIDGET(row->box));

  /* --- Add row to the listbox --- */
  gtk_list_box_insert (listbox, GTK_WIDGET(row->row),index);

	row->index = gtk_list_box_row_get_index (row->row);
	yaup->rows = g_slist_insert (yaup->rows, row, row->index);

  /* --- Make it visible --- */
  gtk_widget_show_all (GTK_WIDGET(row->row));

  return (row);
}

void
remove_list_item(Row *row, Yaup *yaup)
{
	/* GET_UI_ELEMENT(GtkListBox, listbox); */

	debug_print("Removing %s from the GSList.", row->name);
	yaup->rows = g_slist_remove (yaup->rows, row);

	debug_print("Removing %s from the GtkListBox.", row->name);
	/* gtk_container_remove (GTK_CONTAINER(listbox), GTK_WIDGET(row->row)); */
	gtk_widget_destroy(GTK_WIDGET(row->row));

	return;
}

void
activate_row(Row  *row,
             Yaup *yaup)
{
  /* Set switch on */
  g_signal_handlers_block_by_func (G_OBJECT(row->sw),
								   on_switch_state_set,
								   yaup);
  gtk_switch_set_active (row->sw, TRUE);
  gtk_switch_set_state (row->sw,TRUE);
	row->enabled = TRUE;
  g_signal_handlers_unblock_by_func (G_OBJECT(row->sw),
									 on_switch_state_set,
									 yaup);

  /* Make unusable */
  gtk_widget_set_sensitive (GTK_WIDGET(row->entry_name), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->entry_ip), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET(row->ispin), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET(row->ospin), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET(row->combo), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->bt_remove), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->check_iport), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->check_ip), FALSE);

  return;
}

void
disable_row(Row  *row,
            Yaup *yaup)
{
  /* Set switch off */
  g_signal_handlers_block_by_func (G_OBJECT(row->sw),
								   on_switch_state_set,
								   yaup);
  gtk_switch_set_active (row->sw, FALSE);
  gtk_switch_set_state (row->sw,FALSE);
	row->enabled = FALSE;
  g_signal_handlers_unblock_by_func (G_OBJECT(row->sw),
									 on_switch_state_set,
									 yaup);

  /* Make usable */
  gtk_widget_set_sensitive (GTK_WIDGET(row->entry_name), TRUE);
	if(row->ip_enabled)
		{
			gtk_widget_set_sensitive (GTK_WIDGET(row->entry_ip), TRUE);
		}
	if(row->iport_enabled)
		{
			gtk_widget_set_sensitive (GTK_WIDGET(row->ispin), TRUE);
		}
  gtk_widget_set_sensitive (GTK_WIDGET(row->ospin), TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET(row->combo), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->bt_remove), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->check_iport), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(row->check_ip), TRUE);

  return;
}

Row *
get_port_in_use(Row  *arow,
                Yaup *yaup)
{
  Row *brow;

  for(int i = 0;
      (brow = g_slist_nth_data (yaup->rows, i)) != NULL;
      i++)
	{
	  if(brow->enabled == TRUE
		 && (arow->iport == brow->iport || arow->oport == brow->oport)
		 && (strcmp(arow->protocol, brow->protocol) == 0
			 || strcmp(arow->protocol, "TCP & UDP") == 0
			 || strcmp(brow->protocol, "TCP & UDP") == 0)
	  	 && arow->index != brow->index)
		{
			return brow;
		}
	}

	return NULL;
}

Row *
get_exact_row(Row *arow,
              Yaup *yaup)
{
  Row *brow;

  for(int i = 0;
      (brow = g_slist_nth_data (yaup->rows, i)) != NULL;
       i++)
	{
    if(strcmp(arow->name, brow->name) == 0
       && arow->iport == brow->iport
       && arow->oport == brow->oport
       && strcmp(arow->protocol, brow->protocol) == 0
       && arow->index != brow->index)
      {
				return brow;
      }
	}

	return NULL;
}

Row *
get_udp_tcp(Row *arow,
            Yaup *yaup)
{
  Row *brow;

  for(int i = 0;
      (brow = g_slist_nth_data (yaup->rows, i)) != NULL;
       i++)
	{
    if(strcmp(arow->name, brow->name) == 0
       && arow->iport == brow->iport
       && arow->oport == brow->oport
       && strcmp(brow->protocol, "TCP & UDP") == 0
       && arow->index != brow->index)
      {
        return brow;
      }
	}

	return NULL;
}

Row *
get_counterpart(Row *arow,
                Yaup *yaup)
{
  Row *brow;

  for(int i = 0;
      (brow = g_slist_nth_data (yaup->rows, i)) != NULL;
       i++)
	{
    if(strcmp(arow->name, brow->name) == 0
       && arow->iport == brow->iport
       && arow->oport == brow->oport
       && ((strcmp(arow->protocol, "TCP") == 0
            && strcmp(arow->protocol, "UDP") == 0)
          || (strcmp(arow->protocol, "UDP") == 0
              && strcmp(brow->protocol, "TCP") == 0))
       && arow->index != brow->index)
      {
        return brow;
      }
	}

	return NULL;
}

void
delete_row (GtkWidget *widget,
            gpointer   data)
{
	/* destroying the widget removes the widget
	 * from the container and frees the data when there's no reference left.
	 */
	gtk_widget_destroy(widget);

	return;
}

void
refresh_listbox (Yaup *yaup)
{
	GET_UI_ELEMENT(GtkListBox, listbox);
	Row *row;

	/* delete every ListBoxRow */
	gtk_container_foreach (GTK_CONTAINER(listbox), delete_row, yaup);

	/* add ListBoxRow corresponding to the GSList */
	for(int i = 0;
			(row = g_slist_nth_data (yaup->rows, i)) != NULL;
			i++)
	  {
			debug_print("adding %s to listbox\n", row->name);
			add_list_item_from_row(row, yaup, i);

		}

	return;
}
