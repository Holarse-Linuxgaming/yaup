/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * yaup.c
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
#include <miniupnpc/upnpcommands.h>

#include "resources.c"
#include "callbacks.h"
#include "yaup.h"
#include "io.h"
#include "listboxthings.h"
#include "upnpcommunication.h"
#include "yaup-i18n.h"
#include "debug.h"

/* Pass TRUE to your compiler to use the resource file, previously generated
 * with "glib-compile-resources --generate-source resources.gresource.xml"
 * example: gcc -o yaup yaup.c -DRESOURCE=TRUE
 */
#ifndef RESOURCE
	#define RESOURCE FALSE
#endif

/* Same goes to this variable. Set it to use the files located in
 * the src directory.
 */
#ifndef LOCAL_FILES
	/* Use the installed program files */
	#define UI_FILE PACKAGE_DATA_DIR"/ui/yaup.ui"
	#define MENU_FILE PACKAGE_DATA_DIR"/ui/menus.ui"
	#define ICON_FILE_DARK PACKAGE_DATA_DIR"/ui/yaup-dark.png"
	#define ICON_FILE_LIGHT PACKAGE_DATA_DIR"/ui/yaup-light.png"
#else
	/* For testing propose use the local (not installed) ui file */
	#define UI_FILE "src/yaup.ui"
	#define MENU_FILE "src/menus.ui"
	#define ICON_FILE_DARK "src/yaup-dark.png"
	#define ICON_FILE_LIGHT "src/yaup-light.png"
#endif

inline GObject *
yaup_get_ui_element (Yaup * yaup, const gchar * name)
{
  return gtk_builder_get_object(yaup->definitions, name);
}

static void
quit_clicked (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  Yaup *yaup = (Yaup *) user_data;
	g_application_quit(G_APPLICATION(yaup->application));

	return;
}

static void
preferences_clicked (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       yaup)
{
	on_preferences_clicked(NULL, (Yaup *)yaup);

	return;
}

static void
reload_clicked (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       yaup)
{
	on_reload_clicked(NULL, (Yaup *)yaup);

	return;
}

static void
about_clicked (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       yaup)
{
	on_bt_about_clicked(NULL, (Yaup *)yaup);

	return;
}

static GActionEntry app_entries[] =
{
	{ "reload", reload_clicked, NULL, NULL, NULL },
  { "preferences", preferences_clicked, NULL, NULL, NULL },
	{ "about", about_clicked, NULL, NULL, NULL },
  { "quit", quit_clicked, NULL, NULL, NULL }
};

void
yaup_init (GApplication *app, gpointer user_data)
{
	Yaup				*yaup = (Yaup *) user_data;
	const gchar *user_config_dir;
	GError*			gerror = NULL;

	yaup->definitions = gtk_builder_new();
	user_config_dir = g_get_user_config_dir ();
  yaup->config_dir = g_build_filename (user_config_dir, "yaup", NULL);
	yaup->config_file = g_build_filename (yaup->config_dir, "config.conf", NULL);
	strcpy(yaup->local_ip, "0.0.0.0");
	yaup->devicelist  = 0;
	yaup->rows = NULL;
	yaup->controlURLs = NULL;
	yaup->own_controlURL = FALSE;

	if(RESOURCE)
		{
			if(!gtk_builder_add_from_resource (yaup->definitions, "/de/holarse-linuxgaming/yaup/gtk/menus.ui", &gerror))
				{
					g_critical ("Couldn't load builder file: %s", gerror->message);
					// TODO: exit
					//g_error_free (gerror);
				}

			if (!gtk_builder_add_from_resource (yaup->definitions, "/de/holarse-linuxgaming/yaup/yaup.ui", &gerror))
				{
					g_critical ("Couldn't load builder file: %s", gerror->message);
					// TODO: exit
					//g_error_free (gerror);
				}
		}
	else
		{
			if(!gtk_builder_add_from_file (yaup->definitions, MENU_FILE, &gerror))
				{
					g_critical ("Couldn't load builder file: %s", gerror->message);
					// TODO: exit
					//g_error_free (gerror);
				}

			if (!gtk_builder_add_from_file (yaup->definitions, UI_FILE, &gerror))
				{
					g_critical ("Couldn't load builder file: %s", gerror->message);
					// TODO: exit
					//g_error_free (gerror);
				}
		}


	/* Autoconnect signals from the ui file to the callbacks in callbacks.h */
	gtk_builder_connect_signals (yaup->definitions, yaup);
	yaup->objects = gtk_builder_get_objects (yaup->definitions);

	/* Now we can get our named ui elements with this handy function:
	 */
	GET_UI_ELEMENT (GtkStatusbar, statusbar);

	/* Get context ids to categorize our future messages in the statusbar.
	 */
	yaup->statusbar_error_id = gtk_statusbar_get_context_id (statusbar,"error");
	yaup->statusbar_notice_id = gtk_statusbar_get_context_id (statusbar,"notice");

	/* This connects our functions to the app-menu.
	 */
	g_action_map_add_action_entries (G_ACTION_MAP (yaup->application),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   yaup);

	return;
}

void
activate (GApplication *app, gpointer user_data)
{
	Yaup				*yaup = (Yaup *) user_data;
	GTask				*task;
	gboolean		*dark;
	GdkPixbuf		*icon = NULL;

	GET_UI_ELEMENT (GtkWidget, window);
	GET_UI_ELEMENT (GtkSpinner, spinner);
	GET_UI_ELEMENT (GtkButton, add);
	GET_UI_ELEMENT (GtkStatusbar, statusbar);
	GET_UI_ELEMENT (GtkAboutDialog, window_about);
	GET_UI_ELEMENT (GtkListBox, listbox);
	GET_UI_ELEMENT (GtkWidget, info_bar);
	GET_UI_ELEMENT (GtkContainer, top_grid);
	GET_UI_ELEMENT (GtkHeaderBar, headerbar);
	GET_UI_ELEMENT (GtkWidget, header_right);

	/* Get user defined value for using a dark theme.
	 */
	g_object_get (gtk_widget_get_settings (window),
		            "gtk-application-prefer-dark-theme", &dark,
		            NULL);

	/* Glade is not capable of adding something to the right in a headerbar: */
	gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar), header_right);

	if(dark)
		{
			debug_print("user prefers dark theme\n");

			if(RESOURCE)
				{
					icon = gdk_pixbuf_new_from_resource ("/de/holarse-linuxgaming/\
																								yaup/yaup-light.png", NULL);

				}
			else
				{
					icon = gdk_pixbuf_new_from_file (ICON_FILE_LIGHT, NULL);

				}

			gtk_window_set_icon (GTK_WINDOW(window), icon);
			gtk_about_dialog_set_logo (window_about, icon);
		}
	else
		{
			// TODO: why we never get here?
			debug_print("user prefers light theme or has nothing set\n");

			if(RESOURCE)
				{
					icon = gdk_pixbuf_new_from_resource ("/de/holarse-linuxgaming/\
																								yaup/yaup-dark.png", NULL);

				}
			else
				{
					icon = gdk_pixbuf_new_from_file (ICON_FILE_DARK, NULL);

				}

			gtk_window_set_icon (GTK_WINDOW(window), icon);
			gtk_about_dialog_set_logo (window_about, icon);
		}

	gtk_application_add_window(GTK_APPLICATION(yaup->application),
														 GTK_WINDOW(window));

	gtk_list_box_set_placeholder (listbox,
																GTK_WIDGET(gtk_image_new_from_pixbuf (icon)));

	/* There's a bug where we can't show an infobar after we have hidden it.
	 * https://bugzilla.gnome.org/show_bug.cgi?id=710888
	 * So we're removing it from the container, but holding a reference so it
	 * doesn't get destroyed.
	 */
	g_object_ref (info_bar);
	gtk_container_remove(top_grid, info_bar);

	/* Now we can show all at once. */
	gtk_widget_show_all (window);

	/* Then we're adding the infobar back in and removing our own
	 * additional reference. */
	gtk_grid_attach (GTK_GRID(top_grid), info_bar, 0, 0, 1, 1);
	g_object_unref (info_bar);

	/* Stuff for the user interface */
	gtk_widget_set_sensitive(GTK_WIDGET(add),FALSE);
	gtk_spinner_start(spinner);
	gtk_statusbar_push(statusbar, yaup->statusbar_notice_id,
										 _("Searching for available devices in the network…"));

	/* Get available devices in the network in a new thread to prevent
	 * this of blocking the UI. */
	task = g_task_new (NULL, NULL, upnp_get_devices_finish, yaup);
	g_task_set_task_data (task, yaup, NULL);
	g_task_run_in_thread (task, upnp_get_devices);
	g_object_unref (task);

	return;
}

void
app_shutdown (GApplication *app,
              gpointer      user_data)
{
	Yaup *yaup = (Yaup *) user_data;

	/* Save our data to file. However, in case of a crash this is too late!
	 */
	write_config (yaup);

	return;
}
