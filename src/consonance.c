/*************************************************************************/
/* Copyright (C) 2007,2008 sacamano <m.sacamano@gmail.com>		 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "consonance.h"

gboolean show_debug = FALSE;

/* FIXME: Cleanup track refs */
static void common_cleanup(struct con_win *cwin)
{
	CDEBUG("(%s): Cleaning up", __func__);

	if ((cwin->cstate->state == ST_STOPPED) && (cwin->cstate->curr_mobj_clear))
		delete_musicobject(cwin->cstate->curr_mobj);

	if ((cwin->cstate->state == ST_PLAYING) || (cwin->cstate->state == ST_PAUSED))
		stop_playback(cwin);

	save_preferences(cwin);

	g_object_unref(cwin->library_store);
	g_object_unref(cwin->pixbuf->image_play);
	g_object_unref(cwin->pixbuf->image_pause);

	if (cwin->pixbuf->pixbuf_dir)
		g_object_unref(cwin->pixbuf->pixbuf_dir);
	if (cwin->pixbuf->pixbuf_file)
		g_object_unref(cwin->pixbuf->pixbuf_file);
	if (cwin->pixbuf->pixbuf_artist)
		g_object_unref(cwin->pixbuf->pixbuf_artist);
	if (cwin->pixbuf->pixbuf_album)
		g_object_unref(cwin->pixbuf->pixbuf_album);
	if (cwin->pixbuf->pixbuf_track)
		g_object_unref(cwin->pixbuf->pixbuf_track);
	if (cwin->pixbuf->pixbuf_genre)
		g_object_unref(cwin->pixbuf->pixbuf_genre);
	g_slice_free(struct pixbuf, cwin->pixbuf);

	if (cwin->album_art)
		gtk_widget_destroy(cwin->album_art);

	g_free(cwin->cpref->configrc_file);
	g_free(cwin->cpref->installed_version);
	g_free(cwin->cpref->audio_sink);
	g_free(cwin->cpref->album_art_pattern);
	g_key_file_free(cwin->cpref->configrc_keyfile);
	free_str_list(cwin->cpref->library_dir);
	free_str_list(cwin->cpref->lib_add);
	free_str_list(cwin->cpref->lib_delete);
	free_str_list(cwin->cpref->library_tree_nodes);
	free_str_list(cwin->cpref->playlist_columns);
	g_slist_free(cwin->cpref->playlist_column_widths);
	g_slice_free(struct con_pref, cwin->cpref);

	g_rand_free(cwin->cstate->rand);
	g_free(cwin->cstate->file_tree_pwd);
	g_mutex_free(cwin->cstate->c_mutex);
	g_cond_free(cwin->cstate->c_cond);
	g_slice_free(struct con_state, cwin->cstate);

	g_free(cwin->cdbase->db_file);
	sqlite3_close(cwin->cdbase->db);
	g_slice_free(struct con_dbase, cwin->cdbase);

	cwin->cmixer->deinit_mixer(cwin);
	g_slice_free(struct con_mixer, cwin->cmixer);

	if (cwin->clibao->ao_dev) {
		CDEBUG("Freeing ao dev");
		ao_close(cwin->clibao->ao_dev);
	}
	ao_shutdown();
	g_slice_free(struct con_libao, cwin->clibao);

	dbus_connection_remove_filter(cwin->con_dbus,
				      dbus_filter_handler,
				      cwin);
	dbus_bus_remove_match(cwin->con_dbus,
			      "type='signal',path='/org/consonance/DBus'",
			      NULL);
	dbus_connection_unref(cwin->con_dbus);

	if (notify_is_initted())
		notify_uninit();

	g_option_context_free(cwin->cmd_context);

	g_slice_free(struct con_win, cwin);
}

void exit_consonance(GtkWidget *widget, struct con_win *cwin)
{
	if (cwin->cpref->save_playlist)
		save_current_playlist_state(cwin);
	common_cleanup(cwin);
	gtk_main_quit();

	CDEBUG("(%s): Halt.", __func__);
}

gint main(gint argc, gchar *argv[])
{
	struct con_win *cwin;

	cwin = g_slice_new0(struct con_win);
	cwin->pixbuf = g_slice_new0(struct pixbuf);
	cwin->cpref = g_slice_new0(struct con_pref);
	cwin->cstate = g_slice_new0(struct con_state);
	cwin->cdbase = g_slice_new0(struct con_dbase);
	cwin->cmixer = g_slice_new0(struct con_mixer);
	cwin->clibao = g_slice_new0(struct con_libao);
	show_debug = FALSE;

	if (init_dbus(cwin) == -1) {
		g_critical("(%s): Unable to init dbus connection", __func__);
		return -1;
	}

	if (init_dbus_handlers(cwin) == -1) {
		g_critical("(%s): Unable to initialize DBUS filter handlers",
			   __func__);
		return -1;
	}

	if (init_options(cwin, argc, argv) == -1)
		return -1;

	/* Allow only one instance */

	if (!cwin->cstate->unique_instance)
		return 0;

	if (init_config(cwin) == -1) {
		g_critical("(%s): Unable to init configuration", __func__);
		return -1;
	}

	if (init_musicdbase(cwin) == -1) {
		g_critical("(%s): Unable to init music dbase", __func__);
		return -1;
	}

	if (init_audio(cwin) == -1) {
		g_critical("(%s): Unable to init audio", __func__);
		return -1;
	}

	if (init_threads(cwin) == -1) {
		g_critical("(%s): Unable to init threads", __func__);
		return -1;
	}

	if (init_notify(cwin) == -1) {
		g_critical("(%s): Unable to initialize libnotify", __func__);
		return -1;
	}

	init_state(cwin);

	gdk_threads_enter();
	init_gui(argc, argv, cwin);
	CDEBUG("(%s): Init done. Running ...", __func__);
	gtk_main();
	gdk_threads_leave();

	return 0;
}
