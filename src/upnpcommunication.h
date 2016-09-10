/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * upnpcommunication.h
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

#include <miniupnpc/miniupnpc.h>

#include "yaup.h"

void
upnp_get_external_ip (GTask        *task,
                      gpointer      source_object,
                      gpointer      task_data,
                      GCancellable *cancellable);
void
upnp_get_devices (GTask        *task,
                  gpointer      source_object,
                  gpointer      task_data,
                  GCancellable *cancellable);
void
upnp_list_redirections (GTask        *task,
                        gpointer      source_object,
                        gpointer      task_data,
                        GCancellable *cancellable);
void
upnp_set_redirect (GTask        *task,
                   gpointer      source_object,
                   gpointer      task_data,
                   GCancellable *cancellable);
void
upnp_set_redirect_finish (GObject      *source_object,
                          GAsyncResult *res,
                          gpointer      user_data);
void
upnp_remove_redirect (GTask        *task,
                      gpointer      source_object,
                      gpointer      task_data,
                      GCancellable *cancellable);
void
upnp_remove_redirect_finish (GObject      *source_object,
                             GAsyncResult *res,
                             gpointer      user_data);
void
upnp_get_devices_finish (GObject      *source_object,
                         GAsyncResult *res,
                         gpointer      user_data);

void
upnp_list_redirections_finish (GObject      *source_object,
                               GAsyncResult *res,
                               gpointer      user_data);
