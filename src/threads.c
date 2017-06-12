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

static gpointer play_thread_wav(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_wav(cwin);

	return NULL;
}

static gpointer play_thread_mp3(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_mp3(cwin);

	return NULL;
}

static gpointer play_thread_flac(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_flac(cwin);

	return NULL;
}

static gpointer play_thread_oggvorbis(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_oggvorbis(cwin);

	return NULL;
}

#ifdef ENABLE_MODPLUG

static gpointer play_thread_modplug(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_modplug(cwin);

	return NULL;
}

#endif

gboolean advance_playback(gpointer data)
{
	struct con_win *cwin = data;
	GtkTreePath *path;
	struct musicobject *mobj = NULL;
	GThread *thread;

	if (cwin->cstate->advance_track) {

		CDEBUG("(%s): Advancing to next track", __func__);

		if (cwin->cstate->c_thread)
			g_thread_join(cwin->cstate->c_thread);

		gdk_threads_enter();
		unset_current_song_info(cwin);
		unset_track_progress_bar(cwin);
		gdk_threads_leave();

		cwin->cstate->c_thread = NULL;

		if (cwin->cstate->curr_mobj_clear)
			delete_musicobject(cwin->cstate->curr_mobj);

		/* Get the next track to be played */

		path = current_playlist_get_next(cwin);

		cwin->cstate->state = ST_STOPPED;
		play_button_toggle_state(cwin);

		/* No more tracks */

		if (!path)
			return FALSE;

		/* Start playing new track */

		mobj = current_playlist_mobj_at_path(path, cwin);

		thread = start_playback(mobj, cwin);
		if (!thread)
			g_critical("(%s): Unable to create playback thread",
				   __func__);
		else
			update_current_state(thread, path, PLAYLIST_NEXT, cwin);

		gtk_tree_path_free(path);
	}

	return FALSE;
}

gboolean update_track_progress_bar(gpointer data)
{
	struct con_win *cwin = data;

	gdk_threads_enter();
	__update_track_progress_bar(cwin, cwin->cstate->newsec);
	gdk_threads_leave();

	return FALSE;
}

gboolean update_current_song_info(gpointer data)
{
	struct con_win *cwin = data;

	gdk_threads_enter();
	__update_current_song_info(cwin, cwin->cstate->newsec);
	gdk_threads_leave();

	return FALSE;
}

GThread* start_playback(struct musicobject *mobj, struct con_win *cwin)
{
	GThread *thread = NULL;
	GError *error = NULL;

	if (!mobj) {
		g_critical("(%s): Dangling entry in current playlist", __func__);
		return NULL;
	}

	if ((cwin->cstate->state == ST_PLAYING) ||
	    (cwin->cstate->state == ST_PAUSED)) {
		stop_playback(cwin);
	}

	cwin->cstate->curr_mobj = mobj;
	cwin->cstate->cmd = 0;
	cwin->cstate->curr_mobj_clear = FALSE;

	switch(mobj->file_type) {
	case FILE_WAV:
		thread = g_thread_create(play_thread_wav, cwin, TRUE, &error);
		break;
	case FILE_MP3:
		thread = g_thread_create(play_thread_mp3, cwin, TRUE, &error);
		break;
	case FILE_FLAC:
		thread = g_thread_create(play_thread_flac, cwin, TRUE, &error);
		break;
	case FILE_OGGVORBIS:
		thread = g_thread_create(play_thread_oggvorbis, cwin, TRUE, &error);
		break;
#ifdef ENABLE_MODPLUG
	case FILE_MODPLUG:
		thread = g_thread_create(play_thread_modplug, cwin, TRUE, &error);
		break;
#endif
	default:
		g_warning("(%s): Unknown file type", __func__);
	}

	if (error) {
		g_critical("(%s): Unable to create playback thread : %s",
			   __func__, error->message);
		return NULL;
	}
	else {
		cwin->cstate->state = ST_PLAYING;
		play_button_toggle_state(cwin);

		CDEBUG("(%s): Starting playback", __func__);

		return thread;
	}
}

void pause_playback(struct con_win *cwin)
{
	CDEBUG("(%s): Pause playback", __func__);

	if (cwin->cstate->state == ST_PLAYING) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_PAUSE;
		g_mutex_unlock(cwin->cstate->c_mutex);

		cwin->cstate->state = ST_PAUSED;
		play_button_toggle_state(cwin);
	}
}

void resume_playback(struct con_win *cwin)
{
	CDEBUG("(%s): Resuming playback", __func__);

	if (cwin->cstate->state == ST_PAUSED) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_RESUME;
		g_cond_signal(cwin->cstate->c_cond);
		g_mutex_unlock(cwin->cstate->c_mutex);

		cwin->cstate->state = ST_PLAYING;
		play_button_toggle_state(cwin);
	}
}

void stop_playback(struct con_win *cwin)
{
	CDEBUG("(%s): Stopping playback", __func__);

	if ((cwin->cstate->state == ST_PAUSED) ||
	    (cwin->cstate->state == ST_PLAYING)) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_STOP;
		g_cond_signal(cwin->cstate->c_cond);
		g_mutex_unlock(cwin->cstate->c_mutex);

		if (cwin->cstate->c_thread)
			g_thread_join(cwin->cstate->c_thread);

		if (cwin->cstate->curr_mobj_clear) {
			delete_musicobject(cwin->cstate->curr_mobj);
			cwin->cstate->curr_mobj_clear = FALSE;
		}

		unset_current_song_info(cwin);
		unset_track_progress_bar(cwin);
		unset_album_art(cwin);
		unset_status_icon_tooltip(cwin);

		cwin->cstate->c_thread = NULL;
		cwin->cstate->state = ST_STOPPED;
		play_button_toggle_state(cwin);
	}
}

void seek_playback(struct con_win *cwin, gint seek, gdouble fraction)
{
	CDEBUG("(%s): Seeking to new len: %d", __func__, seek);

	if (cwin->cstate->state == ST_PLAYING) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_SEEK;
		cwin->cstate->seek_len = seek;
		cwin->cstate->seek_fraction = fraction;
		g_mutex_unlock(cwin->cstate->c_mutex);
	}
}

gint process_thread_command(struct con_win *cwin)
{
	gint ret = 0, cmd = 0;

	g_mutex_lock(cwin->cstate->c_mutex);
	cmd = cwin->cstate->cmd;
	g_mutex_unlock(cwin->cstate->c_mutex);

	/* Stop playing */

	if (cmd == CMD_PLAYBACK_STOP)
		ret = CMD_PLAYBACK_STOP;

	/* Pause playing */

	else if (cmd == CMD_PLAYBACK_PAUSE) {

		/* Wait for resume command to come through */

		ao_close(cwin->clibao->ao_dev);
		g_mutex_lock(cwin->cstate->c_mutex);

		while (cwin->cstate->cmd == CMD_PLAYBACK_PAUSE)
			g_cond_wait(cwin->cstate->c_cond, cwin->cstate->c_mutex);

		/* A stop command may have been given while waiting for resume */

		if (cwin->cstate->cmd == CMD_PLAYBACK_STOP)
			ret = CMD_PLAYBACK_STOP;

		/* Received a resume command */

		else if (cwin->cstate->cmd == CMD_PLAYBACK_RESUME)
			ret = CMD_PLAYBACK_RESUME;

		g_mutex_unlock(cwin->cstate->c_mutex);
		if (open_audio_device(0, 0, TRUE, cwin) == -1) {
			g_warning("(%s): Unable to resume", __func__);
		}
	}

	/* Seek to a new position */

	else if (cmd == CMD_PLAYBACK_SEEK)
		ret = CMD_PLAYBACK_SEEK;

	return ret;
}
