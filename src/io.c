/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * io.c
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

#include "io.h"
#include "listboxthings.h"
#include "debug.h"


void
read_config (Yaup *yaup)
{
	GET_UI_ELEMENT (GtkStatusbar, statusbar);
	GET_UI_ELEMENT (GtkCheckButton, check_control_url);
	GET_UI_ELEMENT (GtkComboBox, combo_control_url);

	if( g_file_test(yaup->config_dir,G_FILE_TEST_IS_DIR))
		{
			// file exists
			debug_print("dir exists\n");
		}
	else
		{
			// file doesn't exist
			debug_print("dir doesnt exist at %s\n", yaup->config_file);
			gtk_statusbar_push (statusbar,
								yaup->statusbar_error_id,
								"Directory doesnt exist");
			g_timeout_add_seconds (5, statusbar_refresh_error, yaup);
			g_mkdir_with_parents (yaup->config_dir,0744);
			debug_print("dir created\n");
		}

	if( g_file_test(yaup->config_file,G_FILE_TEST_EXISTS))
		{
			// file exists
			debug_print("file exists\n");
			GKeyFile	*conf;
			gchar			**groups;
			char			*buffer;
			gboolean  test = FALSE;
			int				i = 0;

			conf = g_key_file_new();

			g_key_file_load_from_file (conf,yaup->config_file,G_KEY_FILE_NONE,NULL);
			groups = g_key_file_get_groups(conf,NULL);

			strcpy(yaup->my_controlURL, g_key_file_get_string(conf,
																												"general",
																												"controlURL",
																												NULL));

			/* set the found controlURL or the specified */
			for(i = 0;
					(buffer = g_slist_nth_data (yaup->controlURLs, i)) != NULL;
					i++)
				{
					if(strcmp(yaup->my_controlURL, buffer) == 0)
						{
							gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_control_url),
																					 FALSE);
							gtk_combo_box_set_active (combo_control_url, i);
							test = TRUE;
						}
				}

		if(test == FALSE)
			{
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combo_control_url),
																				yaup->my_controlURL);
				gtk_combo_box_set_active(combo_control_url, i);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_control_url),
																		 TRUE);

			}

		for (int i = 1; groups[i] != NULL; i++)
			{
				char *protocol;
				int iport, oport, oport2;
				char *name;
				char *ip;

				name = g_key_file_get_string (conf,groups[i],
																			"name",
																			NULL);
				protocol = g_key_file_get_string (conf,groups[i],
																					"protocol",
																					NULL);
				oport = g_key_file_get_integer (conf,groups[i],
																				"external_port",
																				NULL);
				oport2 = g_key_file_get_integer (conf,groups[i],
																				 "external_port_range",
																				 NULL);
				if(oport2 == 0)
					{
						oport2 = oport;
					}

				iport = g_key_file_get_integer (conf,groups[i],
																				"internal_port",
																				NULL);
				ip = g_key_file_get_string (conf,groups[i],
																		"ip",
																		NULL);

				//TODO: Key file get error

				debug_print("Generating row for config %s\n", name);
				add_list_item_from_config (FALSE,
																	 name,
																	 iport,
																	 ip,
																	 oport,
																	 oport2,
																	 protocol,
																	 i,
																	 yaup);
			}

			debug_print("Done reading save file\n");
			g_key_file_free (conf);
		}
	else
		{
			// file doesn't exist
			debug_print("file doesnt exist at %s\n", yaup->config_file);
			gtk_statusbar_push (statusbar,
													yaup->statusbar_error_id,
													"Config file doesnt exist");
			g_timeout_add_seconds (5, statusbar_refresh_error, yaup);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_control_url), FALSE);
			gtk_combo_box_set_active (combo_control_url, 0);
		}
}

void
write_config (Yaup *yaup)
{
  GKeyFile *conf;
	Row *row;

	GET_UI_ELEMENT (GtkStatusbar, statusbar);

  conf = g_key_file_new ();

	if( g_file_test(yaup->config_dir, G_FILE_TEST_IS_DIR))
	{
		// file exists
		debug_print("dir exists\n");
	}
	else
	{
		// file doesn't exist
		debug_print("dir doesnt exist at %s\n", yaup->config_file);
		gtk_statusbar_push (statusbar,
		                    yaup->statusbar_error_id,
		                    "dir doesnt exist");
		g_timeout_add_seconds (5, statusbar_refresh_error, yaup);
		return;
	}

		g_key_file_set_string(conf, "general", "controlURL", yaup->controlURL);

	for(int i = 0;
			(row = g_slist_nth_data (yaup->rows, i)) != NULL;
			i++)
	  {
			gchar j[200];
			sprintf(j,"%i",i);
			g_key_file_set_string (conf,j,"name",row->name);
			g_key_file_set_integer (conf,j,"internal_port",row->iport);
			g_key_file_set_string (conf,j,"ip",row->ip);
			g_key_file_set_integer (conf,j,"external_port",row->oport);
			g_key_file_set_integer (conf,j,"external_port_range",row->oport2);
			g_key_file_set_string (conf,j,"protocol",row->protocol);
	  }

	g_key_file_save_to_file (conf,yaup->config_file,NULL);
	g_key_file_free (conf);
}

gboolean
statusbar_refresh_error(gpointer data)
{
	Yaup *yaup = (Yaup *) data;
	GET_UI_ELEMENT (GtkStatusbar, statusbar);

	gtk_statusbar_pop(statusbar, yaup->statusbar_error_id);

	return G_SOURCE_REMOVE;
}

gboolean
statusbar_refresh_notice(gpointer data)
{
	Yaup *yaup = (Yaup *) data;
	GET_UI_ELEMENT (GtkStatusbar, statusbar);

	gtk_statusbar_pop(statusbar, yaup->statusbar_notice_id);

	return G_SOURCE_REMOVE;
}
