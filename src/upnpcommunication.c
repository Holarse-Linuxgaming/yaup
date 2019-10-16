/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * upnpcommunication.c
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
#include <stdlib.h>
#include <string.h>
#include <miniupnpc/miniwget.h>
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

#include "upnpcommunication.h"
#include "yaup.h"
#include "listboxthings.h"
#include "io.h"
#include "yaup-i18n.h"
#include "debug.h"

void
upnp_get_external_ip_finish (GObject      *source_object,
                             GAsyncResult *res,
                             gpointer      user_data)
{
	Yaup	*yaup = user_data;
	GTask *task;
	int		r;

	GET_UI_ELEMENT (GtkStatusbar, statusbar);

	r = g_task_propagate_int (G_TASK (res), NULL);

	if(r != UPNPCOMMAND_SUCCESS
		 || strcmp(yaup->externalIPAddress, "0.0.0.0") == 0
		 || strcmp(yaup->externalIPAddress, "") == 0)
		{
			debug_print("GetExternalIPAddress failed.\n");

			GET_UI_ELEMENT(GtkInfoBar, info_bar);

			gtk_widget_show(GTK_WIDGET(info_bar));
		}
	else
		{
			GET_UI_ELEMENT(GtkWidget, info_bar);

			debug_print("ExternalIPAddress = %s\n", yaup->externalIPAddress);

			gtk_widget_hide(info_bar);
		}

	gtk_statusbar_pop(statusbar, yaup->statusbar_notice_id);
	gtk_statusbar_push(statusbar, yaup->statusbar_notice_id, _("Searching for opened ports…"));

	/* Get available devices in the network in a new thread */
	task = g_task_new (NULL, NULL, upnp_list_redirections_finish, yaup);
	g_task_set_task_data (task, yaup, NULL);
	g_task_run_in_thread (task, upnp_list_redirections);
	g_object_unref (task);

}

void
upnp_get_external_ip(GTask        *task,
                     gpointer      source_object,
                     gpointer      task_data,
                     GCancellable *cancellable)
{
	Yaup *yaup = (Yaup *) task_data;
	/* char	externalIPAddress[40]; */
	int		r;

	r = UPNP_GetExternalIPAddress(yaup->controlURL,
																yaup->igddata.first.servicetype,
																yaup->externalIPAddress);

	g_task_return_int (task, r);
}

void
upnp_get_devices_finish (GObject      *source_object,
                         GAsyncResult *res,
                         gpointer      user_data)
{
	Yaup	*yaup = user_data;
	GTask *task;
	char	*buffer;

	GET_UI_ELEMENT (GtkStatusbar, statusbar);
	GET_UI_ELEMENT (GtkComboBoxText, combo_control_url);

	/* TODO: returned error */

	/* Add found controlURLs to preferences dialog */
	for(int i = 0;
			(buffer = g_slist_nth_data (yaup->controlURLs, i)) != NULL;
			i++)
		{
			gtk_combo_box_text_append_text(combo_control_url, buffer);
		}

	/*  Add rows from config file */
	read_config(yaup);

	gtk_statusbar_pop(statusbar, yaup->statusbar_notice_id);
	gtk_statusbar_push(statusbar, yaup->statusbar_notice_id, _("Checking IP…"));

	/* Get available devices in the network in a new thread */
	task = g_task_new (NULL, NULL, upnp_get_external_ip_finish, yaup);
	g_task_set_task_data (task, yaup, NULL);
	g_task_run_in_thread (task, upnp_get_external_ip);
	g_object_unref (task);

}

void
upnp_get_devices(GTask        *task,
                 gpointer      source_object,
                 gpointer      task_data,
                 GCancellable *cancellable)
{
	Yaup *yaup = (Yaup *) task_data;
	int localport = UPNP_LOCAL_PORT_ANY;
	unsigned char ttl = 2; /* defaulting to 2 */
	const char * rootdescurl = 0;
	const char * multicastif = 0;
	const char * minissdpdpath = 0;
	int error = 0;
	int ipv6 = 0;
	//int retcode = 0;
	int i;

	// Get available devices in the network
	if(rootdescurl
		 || (yaup->devicelist = upnpDiscover(2000,
																				 multicastif,
																				 minissdpdpath,
																				 localport,
																				 ipv6,
																				 ttl,
																				 &error)))
		{
			if(yaup->devicelist)
				{
					debug_print("List of UPNP devices found on the network :\n");
					for(yaup->device = yaup->devicelist;
							yaup->device;
							yaup->device = yaup->device->pNext)
						{
							debug_print(" desc: %s\n st: %s\n\n",
										 yaup->device->descURL,
										 yaup->device->st);
							i = 1;
							if( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl,
																										 &yaup->urls,
																										 &yaup->igddata,
																										 yaup->local_ip,
																										 sizeof(yaup->local_ip)))
								|| (i = UPNP_GetValidIGD(yaup->device,
																				 &yaup->urls,
																				 &yaup->igddata,
																				 yaup->local_ip,
																				 sizeof(yaup->local_ip))))
								{
									switch(i)
										{
										case 1:
												{
													debug_print("Found valid IGD : %s\n",
																			yaup->urls.controlURL);
													break;
												}
										case 2:
												{
													debug_print("Found a (not connected?) IGD : %s\n",
																			yaup->urls.controlURL);
													debug_print("Trying to continue anyway\n");
													break;
												}
										case 3:
												{
													debug_print("UPnP device found. "
																			"Is it an IGD ? : %s\n",
																			yaup->urls.controlURL);
													debug_print("Trying to continue anyway\n");
													break;
												}
										default:
												{
													debug_print("Found device (igd ?) : %s\n",
																			yaup->urls.controlURL);
													debug_print("Trying to continue anyway\n");
												}
										}

									debug_print("Local LAN ip address : %s\n",
															yaup->local_ip);
									yaup->controlURLs = g_slist_append (yaup->controlURLs,
																											yaup->urls.controlURL);
								}
							else
								{
									fprintf(stderr, "No valid UPNP Internet "
																	"Gateway Device found.\n");
									//retcode = 1;
									g_task_return_error (task, NULL);
								}
						}
				}
			else if(!rootdescurl)
				{
					debug_print("upnpDiscover() error code=%d\n", error);
				}

			freeUPNPDevlist(yaup->devicelist); yaup->devicelist = 0;

		}

	debug_print("Beende thread\n");

	g_task_return_pointer (task, yaup, NULL);

}

void
upnp_list_redirections_finish (GObject      *source_object,
                               GAsyncResult *res,
                               gpointer      user_data)
{
	Yaup	*yaup = user_data;

	GET_UI_ELEMENT (GtkSpinner, spinner);
	GET_UI_ELEMENT (GtkRevealer, revealer);
	GET_UI_ELEMENT (GtkButton, add);
	GET_UI_ELEMENT (GtkStatusbar, statusbar);

	refresh_listbox(yaup);

	/* printf("Aktiviere Buttons und Liste\n"); */
	/* gtk_widget_set_sensitive(GTK_WIDGET(reload),TRUE); */
	gtk_widget_set_sensitive(GTK_WIDGET(add),TRUE);
	/* TODO: why this doesnt look nice?: */
	gtk_revealer_set_reveal_child(revealer, TRUE);
	gtk_spinner_stop(spinner);
	gtk_statusbar_pop(statusbar, yaup->statusbar_notice_id);
}

void
upnp_list_redirections(GTask        *task,
                       gpointer      source_object,
                       gpointer      task_data,
                       GCancellable *cancellable)
{
	int r = 0;
	int i = 0;
	char index[6];
	char intClient[40];
	char intPort[6];
	char extPort[6];
	char protocol[4];
	char desc[80];
	char enabled[6];
	char rHost[64];
	char duration[16];
	Yaup *yaup = (Yaup *) task_data;
	//unsigned int num=0;

	GET_UI_ELEMENT(GtkStatusbar, statusbar);

	/*UPNP_GetPortMappingNumberOfEntries(urls->controlURL, igddata->first.servicetype, &num);
	printf("PortMappingNumberOfEntries : %u\n", num);	*/
	debug_print(" i protocol exPort->inAddr:inPort description remoteHost leaseTime\n");

	do {
		snprintf(index, 6, "%d", i);
		rHost[0] = '\0'; enabled[0] = '\0';
		duration[0] = '\0'; desc[0] = '\0';
		extPort[0] = '\0'; intPort[0] = '\0'; intClient[0] = '\0';
		r = UPNP_GetGenericPortMappingEntry(yaup->controlURL, yaup->igddata.first.servicetype,
																				index, extPort, intClient, intPort,
																				protocol, desc, enabled, rHost,
																				duration);
		if(r==0)
			{
				/*
				 * printf("%02d - %s %s->%s:%s\tenabled=%s leaseDuration=%s\n"
				   "     desc='%s' rHost='%s'\n",
				   i, protocol, extPort, intClient, intPort,
				   enabled, duration,
				   desc, rHost);
				   */

				debug_print("%2d %s %5s->%s:%-5s '%s' '%s' %s\n",
				 			 i, protocol, extPort, intClient, intPort,
				 			 desc, rHost, duration);

				gchar notice[200];
				sprintf(notice,_("Found port forwarding for %s with protocol %s: %s->%s"),
								desc, protocol, extPort, intPort);

				gtk_statusbar_push (statusbar,yaup->statusbar_notice_id,notice);
				g_timeout_add_seconds (5, statusbar_refresh_notice, yaup);

				Row *new_row;
				Row *old_row;
				Row *counterpart;
				Row *tcp_udp;

		  	debug_print("Composing row for id %i\n", i);

				new_row = g_new (Row, 1);
				set_list_box_row(yaup, new_row, FALSE, desc, (int) strtol(intPort, (char **)NULL, 10), intClient, (int) strtol(extPort, (char **)NULL, 10), protocol);

				debug_print("looking for %s in list\n", new_row->name);


				debug_print("Looking for available row with same specs\n");
				old_row = get_exact_row (new_row, yaup);

				debug_print("Looking for available row opposite protocol\n");
				counterpart = get_counterpart(new_row, yaup);

				debug_print("Looking for available row with both protocols\n");
				tcp_udp = get_udp_tcp (new_row, yaup);

				/* if(new_row == NULL) */
				/* 	{ */
				/* 		debug_print("Something's wrong here, bad things happened!"); */
				/* 	} */

				if( counterpart == NULL && old_row != NULL && tcp_udp == NULL)
					{
						debug_print("Found existing row\n");
						old_row->enabled = TRUE;

					}
				else if( counterpart != NULL && old_row == NULL && tcp_udp == NULL)
					{
						// set combo to 0 on counterpart & delete new
						debug_print("Found counterpart\n");

						if(counterpart->enabled)
							{
								debug_print("Setting counterpart to both protocols\n");
								strcpy(counterpart->protocol, "TCP & UDP");
								counterpart->enabled = TRUE;

							}
						else
						 {
							 debug_print("Counterpart is disabled, activating new row\n");
							 new_row->enabled = TRUE;
							 yaup->rows = g_slist_append (yaup->rows,new_row);

						 }
					}
				else if(counterpart != NULL
								&& old_row != NULL
								&& tcp_udp == NULL)
					{
						 // set combo to 0 on counterpart & delete new +  old

						 debug_print("Found counterpart & old row\n");
						if(counterpart->enabled)
							{
								debug_print("Counterpart enabled, setting counterpart to both protocols and delete old row\n");
								strcpy(counterpart->protocol, "TCP & UDP");
								yaup->rows = g_slist_remove (yaup->rows, old_row);
							}
						else
							{
								debug_print("Counterpart diabled, activating old row\n");
								old_row->enabled = TRUE;
							}
					}
				else if(counterpart != NULL && old_row == NULL && tcp_udp != NULL)
					{
						// TCP & UDP available

						 debug_print("Found counterpart & tcp_udp\n");
						if(counterpart->enabled)
							{
								debug_print("Counterpart enabled, deleting counterpart, activating tcp_udp\n");
								tcp_udp->enabled = TRUE;
								yaup->rows = g_slist_remove (yaup->rows, counterpart);
							}
					}
				else if(counterpart != NULL && old_row != NULL && tcp_udp != NULL)
					{
						debug_print("Found counterpart, old row and tcp_udp\n");
						if(counterpart->enabled)
							{
								debug_print("Counterpart enabled, deleting counterpart and old row, acitvating tcp_udp\n");
								tcp_udp->enabled = TRUE;
								yaup->rows = g_slist_remove (yaup->rows, old_row);
								yaup->rows = g_slist_remove (yaup->rows, counterpart);
							}
						else if(!tcp_udp->enabled)
							{
								debug_print("Counterpart disabled, tcp_udp disabled, activating old row\n");
								old_row->enabled = TRUE;
							}
						else
							{
								 debug_print("Counterpart and tcp_udp disabled, activating old row\n");
								old_row->enabled = TRUE;
							}
					}
				else if(counterpart == NULL && old_row != NULL && tcp_udp != NULL)
					{
						debug_print("Found old row and tcp_udp\n");

						if(!tcp_udp->enabled
					  && strcmp(old_row->protocol, tcp_udp->protocol) != 0)
							{
								debug_print("tcp_udp different old row and disabled, activating old row\n");
								old_row->enabled = TRUE;
							}
						else
							{
								debug_print("tcp_udp avtivated and same as old_row, activating tcp_udp, deleting old row\n");
								tcp_udp->enabled = TRUE;
								yaup->rows = g_slist_remove (yaup->rows, old_row);
							}
					}
				else if(counterpart == NULL && old_row == NULL && tcp_udp != NULL)
					{
						/* do nothing */
						debug_print("Found tcp_udp\n");

						if(!tcp_udp->enabled)
							{
								debug_print("tcp_udp disabled, activating new_row\n");
								new_row->enabled = TRUE;
								yaup->rows = g_slist_append (yaup->rows,new_row);
							}
						else
							{
								/* remove_list_item(&new_row, yaup); */
							}
					}
				else
					{
						debug_print("Found nothing similar, activating new row\n");
						new_row->enabled = TRUE;
						yaup->rows = g_slist_append (yaup->rows,new_row);
					}
			}
		else
			{
				/* debug_print("No other rows found with error %d\n", r); */
				//debug_print("GetGenericPortMappingEntry() returned %d (%s)\n",
				//r, strupnperror(r));
			}

		i++;
	}
	while(r==0);

	debug_print("got all entrys\n");
	g_task_return_pointer (task, yaup, NULL);
}

void
upnp_set_redirect_finish (GObject      *source_object,
                          GAsyncResult *res,
                          gpointer      user_data)
{
	Yaup	*yaup = user_data;
	int r = g_task_propagate_int (G_TASK (res), NULL);

	/* 1 = success
	 * 2 = (both) failed
	 * 3 = TCP failed
	 * 4 = UDP failed
	 */

	GET_UI_ELEMENT(GtkStatusbar, statusbar);
	GET_UI_ELEMENT(GtkSpinner, spinner);

	switch(r)
		{
		case -1:
				{
					/* couldn't read return value from thread */
				}
		case 1:
				{
					activate_row(yaup->row, yaup);

					char str[80];
					sprintf(str, _("Activated external %i to internal %i for %s to %s"),
						      yaup->row->oport,
						      yaup->row->iport,
						      yaup->row->protocol,
									yaup->row->ip);

					gtk_statusbar_push(statusbar,
						                 yaup->statusbar_notice_id,
						                 str);
					break;
				}
		case 2:
				{
					char str[80];
					/* sprintf(str, _("Activating failed with error %i and error %i"), */
					/* 	      ret1, */
					/* 	      ret2); */
					gtk_statusbar_push(statusbar,
						                 yaup->statusbar_error_id,
						                 str);
					disable_row (yaup->row, yaup);
					break;
				}
		case 3:
				{
					char str[80];
					/* sprintf(str, _("Activating TCP failed with error code %i"), ret2); */
					gtk_statusbar_push(statusbar,
						                 yaup->statusbar_error_id,
						                 str);

					add_list_item_from_config(TRUE, yaup->row->name, yaup->row->iport, yaup->row->ip, yaup->row->oport,
																		"UDP", yaup->row->index + 1, yaup);

					disable_row (yaup->row, yaup);
					break;
				}
		case 4:
				{
					char str[80];
					/* sprintf(str, _("Activating UDP failed with error code %i"), ret1); */
					gtk_statusbar_push(statusbar,
						                 yaup->statusbar_error_id,
						                 str);

					add_list_item_from_config(TRUE, yaup->row->name, yaup->row->iport, yaup->row->ip, yaup->row->oport,
						                        "TCP", yaup->row->index + 1, yaup);

					disable_row (yaup->row, yaup);
					break;
				}
		case UPNPCOMMAND_INVALID_ARGS:
				{
					break;
				}
		case UPNPCOMMAND_HTTP_ERROR:
				{
					break;
				}
		default:
				{
					char str[80];
					/* sprintf(str, _("Activating failed with error %i"), */
					/* 	      ret1); */
					gtk_statusbar_push(statusbar,
						                 yaup->statusbar_error_id,
						                 str);
					disable_row (yaup->row, yaup);
					break;
				}
		}

	g_timeout_add_seconds (5, statusbar_refresh_error, yaup);
	g_timeout_add_seconds (5, statusbar_refresh_notice, yaup);
	gtk_spinner_stop(spinner);
}

/* Test function
 * 1 - get connection type
 * 2 - get extenal ip address
 * 3 - Add port mapping
 * 4 - get this port mapping from the IGD */
void
upnp_set_redirect(GTask        *task,
                  gpointer      source_object,
                  gpointer      task_data,
                  GCancellable *cancellable)
{
	Yaup	*yaup;
	char	externalIPAddress[40];
	char	intClient[40];
	char	intPort[6];
	char	duration[16];
	char	ibuffer[6];
	char	obuffer[6];
	int		ret1, ret2;
	char	leaseDuration[] = "0";

	yaup = task_data;
	sprintf(ibuffer, "%i", yaup->row->iport);
	sprintf(obuffer, "%i", yaup->row->oport);
	ret1 = UPNPCOMMAND_SUCCESS;
	ret2 = UPNPCOMMAND_SUCCESS;

	if(!yaup->externalIPAddress || !yaup->row->iport || !yaup->row->oport || !yaup->row->protocol)
		{
			fprintf(stderr, "Wrong arguments\n");
			/* return UPNPCOMMAND_INVALID_ARGS; */
			g_task_return_int (task, UPNPCOMMAND_INVALID_ARGS);
		}

	/* proto = protofix(proto); */

	if(!yaup->row->protocol)
		{
			fprintf(stderr, "invalid protocol\n");
			/* return UPNPCOMMAND_HTTP_ERROR; */
			g_task_return_int (task, UPNPCOMMAND_HTTP_ERROR);
		}

	ret1 = UPNP_GetExternalIPAddress(yaup->controlURL,
				      yaup->igddata.first.servicetype,
				      yaup->externalIPAddress);
	if(ret1 != UPNPCOMMAND_SUCCESS)
		{
			debug_print("GetExternalIPAddress failed.\n");
		}
	else
		{
			debug_print("ExternalIPAddress = %s\n", yaup->externalIPAddress);
		}

	ret1 = UPNPCOMMAND_SUCCESS;

	if(strcmp(yaup->row->protocol, "TCP & UDP") == 0)
		{
		  ret1 = UPNP_AddPortMapping(yaup->controlURL, yaup->igddata.first.servicetype,
					obuffer, ibuffer, yaup->row->ip, yaup->row->name,
					"TCP", 0, leaseDuration);
		  ret2 = UPNP_AddPortMapping(yaup->controlURL, yaup->igddata.first.servicetype,
					obuffer, ibuffer, yaup->row->ip, yaup->row->name,
					"UDP", 0, leaseDuration);

		}
	else
		{
		  ret1 = UPNP_AddPortMapping(yaup->controlURL, yaup->igddata.first.servicetype,
					obuffer, ibuffer, yaup->row->ip, yaup->row->name,
					yaup->row->protocol, 0, leaseDuration);

		}


	if(ret1!=UPNPCOMMAND_SUCCESS)
		{
			debug_print("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
			       obuffer, ibuffer, yaup->row->ip, ret1, strupnperror(ret1));
		}

	if(ret2!=UPNPCOMMAND_SUCCESS)
		{
			debug_print("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
			       obuffer, ibuffer, yaup->row->ip, ret2, strupnperror(ret2));
		}

	if(strcmp(yaup->row->protocol, "TCP & UDP") == 0)
		{
			ret1 = UPNP_GetSpecificPortMappingEntry(yaup->controlURL,
					     yaup->igddata.first.servicetype,
					     obuffer, "TCP", NULL/*remoteHost*/,
					     intClient, intPort, NULL/*desc*/,
					     NULL/*enabled*/, duration);
			ret2 = UPNP_GetSpecificPortMappingEntry(yaup->controlURL,
					     yaup->igddata.first.servicetype,
					     obuffer, "UDP", NULL/*remoteHost*/,
					     intClient, intPort, NULL/*desc*/,
					     NULL/*enabled*/, duration);

			if(ret2 != UPNPCOMMAND_SUCCESS)
				{
					debug_print("GetSpecificPortMappingEntry() failed with code %d (%s)\n",
								 ret2, strupnperror(ret2));
					//return r;
				}
				else {
					debug_print("InternalIP:Port = %s:%s\n", intClient, intPort);
					debug_print("external %s:%s %s is redirected to internal %s:%s (duration=%s)\n",
								 externalIPAddress, obuffer, yaup->row->protocol, intClient, intPort, duration);
				}
		}
	else
		{
			ret1 = UPNP_GetSpecificPortMappingEntry(yaup->controlURL,
					     yaup->igddata.first.servicetype,
					     obuffer, yaup->row->protocol, NULL/*remoteHost*/,
					     intClient, intPort, NULL/*desc*/,
					     NULL/*enabled*/, duration);
		}

	if(ret1 != UPNPCOMMAND_SUCCESS)
		{
			debug_print("GetSpecificPortMappingEntry() failed with code %d (%s)\n",
									ret1, strupnperror(ret1));
			/* return r; */
		}
	else
		{
			debug_print("InternalIP:Port = %s:%s\n", intClient, intPort);
			debug_print("external %s:%s %s is redirected to internal %s:%s \
									(duration=%s)\n", externalIPAddress, obuffer,
									yaup->row->protocol, intClient, intPort, duration);
		}

	/* return UPNPCOMMAND_SUCCESS; */

	if(UPNPCOMMAND_SUCCESS == ret1 && UPNPCOMMAND_SUCCESS == ret2 )
		{
		  g_task_return_int (task, 1);

		}
	else if(strcmp(yaup->row->protocol, "TCP & UDP") == 0
		      && ret1 != UPNPCOMMAND_SUCCESS
		      && ret2 != UPNPCOMMAND_SUCCESS)
		{
			g_task_return_int (task, 2);

		}
	else if(strcmp(yaup->row->protocol, "TCP & UDP") == 0
		      && ret1 == UPNPCOMMAND_SUCCESS
		      && ret2 != UPNPCOMMAND_SUCCESS)
		{
		  g_task_return_int (task, 3);

		}
	else if(strcmp(yaup->row->protocol, "TCP & UDP") == 0
		      && ret1 != UPNPCOMMAND_SUCCESS
		      && ret2 == UPNPCOMMAND_SUCCESS)
		{
			g_task_return_int (task, 4);

		}
	else
		{
			g_task_return_int (task, 0);

		}
}

void
upnp_remove_redirect_finish (GObject      *source_object,
                             GAsyncResult *res,
                             gpointer      user_data)
{
	Yaup	*yaup = user_data;
	int r = g_task_propagate_int (G_TASK (res), NULL);

	/* 1 = success
	 * 2 = (both) failed
	 * 3 = TCP failed
	 * 4 = UDP failed
	 */

	GET_UI_ELEMENT(GtkStatusbar, statusbar);
	GET_UI_ELEMENT(GtkSpinner, spinner);

	switch(r)
		{
		case -1:
				{
					//couldn't read
				}
		case 1:
				{
					disable_row (yaup->row, yaup);

				  char str[80];
				  sprintf(str, _("Removed external %i to internal %i for %s"),
				          yaup->row->oport,
				          yaup->row->iport,
				          yaup->row->protocol);

				  gtk_statusbar_push(statusbar,
				                     yaup->statusbar_notice_id,
				                     str);
					break;
				}
		case 2:
				{
					activate_row (yaup->row, yaup);

				  char str[80];
				  /* sprintf(str, */
				  /*         _("Removing failed with error %i and error %i"), */
				  /*         ret1, */
				  /*         ret2); */
				 sprintf(str, _("Removing failed"));
				  gtk_statusbar_push(statusbar,
				                     yaup->statusbar_error_id,
				                     str);
					break;
				}
		case 3:
				{
					char str[80];
					sprintf(str, _("Removing TCP failed"));
				  /* sprintf(str, _("Removing TCP failed with error code %i"), ret2); */
				  gtk_statusbar_push(statusbar,
				                     yaup->statusbar_error_id,
				                     str);

				  add_list_item_from_config(TRUE, yaup->row->name, yaup->row->iport,
																		yaup->row->ip, yaup->row->oport,
				                            "UDP", yaup->row->index + 1, yaup);

				  disable_row (yaup->row, yaup);
					break;
				}
		case 4:
				{
					char str[80];
				  /* sprintf(str, _("Removing UDP failed with error code %i"), ret1); */
					sprintf(str, _("Removing UDP failed"));
				  gtk_statusbar_push(statusbar,
				                     yaup->statusbar_error_id,
				                     str);

				  add_list_item_from_config(TRUE,	yaup->row->name, yaup->row->iport,
																		yaup->row->ip, yaup->row->oport,
				                            "TCP", yaup->row->index + 1, yaup);

				  disable_row (yaup->row, yaup);
					break;
				}
		default:
				{
					activate_row (yaup->row, yaup);

				  char str[80];
				  /* sprintf(str, */
				  /*         _("Removing failed with error %i and error %i"), */
				  /*         ret1, */
				  /*         ret2); */
				 sprintf(str, _("Removing failed"));
				  gtk_statusbar_push(statusbar,
				                     yaup->statusbar_error_id,
				                     str);
					break;
				}
		}

	g_timeout_add_seconds (5, statusbar_refresh_error, yaup);
	g_timeout_add_seconds (5, statusbar_refresh_notice, yaup);
	gtk_spinner_stop(spinner);
}

void
upnp_remove_redirect(GTask        *task,
                     gpointer      source_object,
                     gpointer      task_data,
                     GCancellable *cancellable)
{
	Yaup *yaup = (Yaup *) task_data;
  char ebuffer[33];
  sprintf(ebuffer, "%d", yaup->row->oport);
  int ret1 = 0;
  int ret2 = 0;
	char remoteHost[2] = "\0";

	if(strcmp(yaup->row->protocol, "TCP & UDP") == 0)
    {
      ret1 = UPNP_DeletePortMapping(yaup->controlURL,
																		yaup->igddata.first.servicetype,
																		ebuffer, "TCP", remoteHost);
      ret2 = UPNP_DeletePortMapping(yaup->controlURL,
																		yaup->igddata.first.servicetype,
																		ebuffer, "UDP", remoteHost);

    }
  else
    {
			ret1 = UPNP_DeletePortMapping(yaup->controlURL,
																		yaup->igddata.first.servicetype,
																		ebuffer, yaup->row->protocol, remoteHost);

    }

	if( 0 == ret1 && 0 == ret2)
    {
			// success
			g_task_return_int (task, 1);

    }
  else if(strcmp(yaup->row->protocol, "TCP & UDP") == 0
          && ret1 == 0
          && ret2 != 0)
    {
      // TCP failed
			g_task_return_int (task, 3);

    }
  else if(strcmp(yaup->row->protocol, "TCP & UDP") == 0
          && ret1 != 0
          && ret2 == 0)
    {
      // UDP failed
			g_task_return_int (task, 4);

    }
	else
		{
			// (both) failed
			g_task_return_int (task, 2);
		}

}
