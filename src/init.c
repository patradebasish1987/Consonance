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

static gchar *audio_backend = NULL;
static gchar *audio_mixer = NULL;

GOptionEntry cmd_entries[] = {
	{"version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_version, "Version", NULL},
	{"debug", 'e', 0, G_OPTION_ARG_NONE, &show_debug, "Enable Debug", NULL},
	{"play", 'p', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_play, "Play", NULL},
	{"stop", 's', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_stop, "Stop", NULL},
	{"pause", 't', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_pause, "Pause/Resume", NULL},
	{"prev", 'r', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_prev, "Prev", NULL},
	{"next", 'n', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_next, "Next", NULL},
	{"inc_vol", 'i', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_inc_volume, "Increase volume by 1", NULL},
	{"dec_vol", 'd', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_dec_volume, "Decrease volume by 1", NULL},
	{"show_osd", 'o', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_show_osd, "Show OSD notification", NULL},
	{"current_state", 'c', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_current_state, "Get current player state", NULL},
	{"audio_backend", 'a', 0, G_OPTION_ARG_STRING,
	 &audio_backend, "Audio backend (valid options: alsa/oss)", NULL},
	{"audio_mixer", 'm', 0, G_OPTION_ARG_STRING,
	 &audio_mixer, "Mixer Element (For ALSA: Master, PCM, etc.., For OSS: /dev/mixer, etc...)", NULL},
	{G_OPTION_REMAINING, 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_CALLBACK,
	 cmd_add_file, "", "[URI1 [URI2...]]"},
	{NULL}
};

static gboolean _init_gui_state(gpointer data)
{
	struct con_win *cwin = data;

	if (gtk_main_iteration_do(FALSE))
		return TRUE;
	init_playlist_view(cwin);

	if (gtk_main_iteration_do(FALSE))
		return TRUE;
	init_tag_completion(cwin);

	if (gtk_main_iteration_do(FALSE))
		return TRUE;
	init_library_view(cwin);

	if (gtk_main_iteration_do(FALSE))
		return TRUE;
	if (cwin->cpref->save_playlist)
		init_current_playlist_view(cwin);

	return TRUE;
}

static gint init_audio_mixer(struct con_win *cwin)
{
	if (!g_ascii_strcasecmp(ao_driver_info(cwin->clibao->ao_driver_id)->short_name,
				PULSE_SINK))
		set_alsa_mixer(cwin, audio_mixer);
	else if (!g_ascii_strcasecmp(ao_driver_info(cwin->clibao->ao_driver_id)->short_name,
                                ALSA_SINK))
                set_alsa_mixer(cwin, audio_mixer);
	else if (!g_ascii_strcasecmp(ao_driver_info(cwin->clibao->ao_driver_id)->short_name,
				     OSS_SINK))
		set_oss_mixer(cwin, audio_mixer);
	else
		return -1;

	return 0;
}

gint init_dbus(struct con_win *cwin)
{
	DBusConnection *conn = NULL;
	DBusError error;
	gint ret = 0;

	dbus_error_init(&error);
	conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (!conn) {
		g_critical("(%s): Unable to get a DBUS connection", __func__);
		dbus_error_free(&error);
		return -1;
	}

	ret = dbus_bus_request_name(conn, DBUS_NAME, 0, &error);
	if (ret == -1) {
		g_critical("(%s): Unable to request for DBUS service name", __func__);
		dbus_error_free(&error);
		return -1;
	}

	if (ret & DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		cwin->cstate->unique_instance = TRUE;
	else if (ret & DBUS_REQUEST_NAME_REPLY_EXISTS)
		cwin->cstate->unique_instance = FALSE;

	dbus_connection_setup_with_g_main(conn, NULL);
	cwin->con_dbus = conn;

	return 0;
}

gint init_dbus_handlers(struct con_win *cwin)
{
	DBusError error;

	dbus_error_init(&error);
	if (!cwin->con_dbus) {
		g_critical("No DBUS connection");
		return -1;
	}

	dbus_bus_add_match(cwin->con_dbus,
			   "type='signal',path='/org/consonance/DBus'",
			   &error);
	if (dbus_error_is_set(&error)) {
		g_critical("(%s): Unable to register match rule for DBUS", __func__);
		dbus_error_free(&error);
		return -1;
	}

	if (!dbus_connection_add_filter(cwin->con_dbus, dbus_filter_handler, cwin, NULL)) {
		g_critical("(%s): Unable to allocate memory for DBUS filter", __func__);
		return -1;
	}

	return 0;
}

gint init_options(struct con_win *cwin, int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;
	GOptionGroup *group;

	CDEBUG("Initializing Command line options");

	context = g_option_context_new("- A lightweight music manager");
	group = g_option_group_new("General", "General", "General Options", cwin, NULL);
	g_option_group_add_entries(group, cmd_entries);
	g_option_context_set_main_group(context, group);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_critical("(%s): Unable to parse options: %s\n", __func__, error->message);
		g_print(g_option_context_get_help(context, TRUE, NULL));
		g_option_context_free(context);
		g_error_free(error);
		return -1;
	}

	/* Store reference */

	cwin->cmd_context = context;
	return 0;
}

gint init_config(struct con_win *cwin)
{
	GError *error = NULL;
	gint *col_widths, *win_size;
	gchar *conrc, *condir, **libs, **columns, **nodes, *last_rescan_time;
	gchar *u_file;
	const gchar *home;
	gboolean err = FALSE;
	gboolean libs_f,
		lib_add_f,
		lib_delete_f,
		columns_f,
		nodes_f,
		col_widths_f,
		cur_lib_view_f,
		hidden_f,
		file_tree_pwd_f,
		album_f,
		album_art_pattern_f,
		osd_f,
		save_playlist_f,
		software_mixer_f,
		shuffle_f,
		repeat_f,
		window_size_f,
		audio_sink_f,
		all_f;
	gsize cnt = 0, i;

	CDEBUG("(%s): Initializing configuration", __func__);

	libs_f = lib_add_f = lib_delete_f = columns_f = nodes_f = cur_lib_view_f = FALSE;
	file_tree_pwd_f = hidden_f = album_f = osd_f = FALSE;
	software_mixer_f = save_playlist_f = album_art_pattern_f = FALSE;
	shuffle_f = repeat_f = window_size_f = all_f = FALSE;
	audio_sink_f = FALSE;

	home = g_get_home_dir();
	condir = g_strdup_printf("%s%s", home, "/.consonance");
	conrc = g_strdup_printf("%s%s", home, "/.consonance/conrc");

	/* Does .consonance exist ? */

	if (g_file_test(condir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) == FALSE) {
		if (g_mkdir(condir, S_IRWXU) == -1) {
			g_critical("(%s): Unable to create preferences directory, err: %s",
				   __func__,
				   strerror(errno));
			err = TRUE;
		}
		CDEBUG("(%s): Created .consonance", __func__);
	}

	cwin->cpref->configrc_file = g_strdup(conrc);
	cwin->cpref->configrc_keyfile = g_key_file_new();

	/* Does conrc exist ? */

	if (g_file_test(conrc, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == FALSE) {
		if (g_creat(conrc, S_IRWXU) == -1) {
			g_critical("(%s): Unable to create conrc, err: %s",
				   __func__, strerror(errno));
			err = TRUE;
		}
		CDEBUG("(%s): Created conrc", __func__);
	}

	/* Load the settings file */

	if (!g_key_file_load_from_file(cwin->cpref->configrc_keyfile,
				       conrc,
				       G_KEY_FILE_NONE,
				       &error)) {
		g_critical("(%s): Unable to load conrc, err: %s", __func__, error->message);
		g_error_free(error);
		all_f = TRUE;
	}
	else {
		/* Retrieve version */

		cwin->cpref->installed_version =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_INSTALLED_VERSION,
					      &error);
		if (!cwin->cpref->installed_version) {
			g_error_free(error);
			error = NULL;
		}

		/* Retrieve window size */

		win_size = g_key_file_get_integer_list(cwin->cpref->configrc_keyfile,
						       GROUP_GENERAL,
						       KEY_WINDOW_SIZE,
						       &cnt,
						       &error);
		if (win_size) {
			cwin->cpref->window_width = MAX(win_size[0], MIN_WINDOW_WIDTH);
			cwin->cpref->window_height = MAX(win_size[1], MIN_WINDOW_HEIGHT);
			g_free(win_size);
		}
		else {
			g_error_free(error);
			error = NULL;
			window_size_f = TRUE;
		}

		/* Retrieve list of libraries */

		libs = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						  GROUP_LIBRARY,
						  KEY_LIBRARY_DIR,
						  &cnt,
						  &error);
		if (libs) {
			for (i=0; i<cnt; i++) {
				gchar *file = g_filename_from_utf8(libs[i],
						   -1, NULL, NULL, &error);
				if (!file) {
					g_warning("(%s): Unable to get filename "
						  "from UTF-8 string: %s\n",
						  __func__, libs[i]);
					g_error_free(error);
					error = NULL;
					continue;
				}
				cwin->cpref->library_dir =
					g_slist_append(cwin->cpref->library_dir,
						       g_strdup(file));
				g_free(file);
			}
			g_strfreev(libs);
		}
		else {
			g_error_free(error);
			error = NULL;
			libs_f = TRUE;
		}

		/* Retrieve list of columns to show in playlist view */

		columns = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						     GROUP_GENERAL,
						     KEY_PLAYLIST_COLUMNS,
						     &cnt,
						     &error);

		if (columns) {
			for (i=0; i<cnt; i++) {
				cwin->cpref->playlist_columns =
					g_slist_append(cwin->cpref->playlist_columns,
						       g_strdup(columns[i]));
			}
			g_strfreev(columns);
		}
		else {
			g_error_free(error);
			error = NULL;
			columns_f = TRUE;
		}

		/* Retrieve list of column widths */

		col_widths = g_key_file_get_integer_list(cwin->cpref->configrc_keyfile,
							 GROUP_GENERAL,
							 KEY_PLAYLIST_COLUMN_WIDTHS,
							 &cnt,
							 &error);

		if (col_widths) {
			for (i = 0; i < cnt; i++) {
				cwin->cpref->playlist_column_widths =
					g_slist_append(cwin->cpref->playlist_column_widths,
						       GINT_TO_POINTER(col_widths[i]));
			}
			g_free(col_widths);
		}
		else {
			g_error_free(error);
			error = NULL;
			col_widths_f = TRUE;
		}

		/* Retrieve list of library tree nodes */

		nodes = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						   GROUP_LIBRARY,
						   KEY_LIBRARY_TREE_NODES,
						   &cnt,
						   &error);
		if (nodes) {
			for (i=0; i<cnt; i++) {
				cwin->cpref->library_tree_nodes =
					g_slist_append(cwin->cpref->library_tree_nodes,
						       g_strdup(nodes[i]));
			}
			g_strfreev(nodes);
		}
		else {
			g_error_free(error);
			error = NULL;
			nodes_f = TRUE;
		}

		/* Retrieve library view order */

		cwin->cpref->cur_library_view =
			g_key_file_get_integer(cwin->cpref->configrc_keyfile,
					       GROUP_LIBRARY,
					       KEY_LIBRARY_VIEW_ORDER,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			cur_lib_view_f = TRUE;
		}

		/* Retrieve last rescan time */

		last_rescan_time =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_LIBRARY,
					      KEY_LIBRARY_LAST_SCANNED,
					      &error);
		if (!last_rescan_time) {
			g_error_free(error);
			error = NULL;
		} else {
			if (!g_time_val_from_iso8601(last_rescan_time, &cwin->cpref->last_rescan_time))
				g_warning("(%s): Unable to convert last rescan time\n", __func__);
			g_free(last_rescan_time);
		}

		/* Retrieve list of libraries to be added */

		libs = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						  GROUP_LIBRARY,
						  KEY_LIBRARY_ADD,
						  &cnt,
						  &error);
		if (libs) {
			for (i = 0; i < cnt; i++) {
				gchar *file = g_filename_from_utf8(libs[i],
						   -1, NULL, NULL, &error);
				if (!file) {
					g_warning("(%s): Unable to get filename "
						  "from UTF-8 string: %s\n",
						  __func__, libs[i]);
					g_error_free(error);
					error = NULL;
					continue;
				}
				cwin->cpref->lib_add =
					g_slist_append(cwin->cpref->lib_add,
						       g_strdup(file));
				g_free(file);
			}
			g_strfreev(libs);
		}
		else {
			g_error_free(error);
			error = NULL;
			lib_add_f = TRUE;
		}

		/* Retrieve list of libraries to be deleted */

		libs = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						  GROUP_LIBRARY,
						  KEY_LIBRARY_DELETE,
						  &cnt,
						  &error);
		if (libs) {
			for (i = 0; i < cnt; i++) {
				gchar *file = g_filename_from_utf8(libs[i],
						   -1, NULL, NULL, &error);
				if (!file) {
					g_warning("(%s): Unable to get filename "
						  "from UTF-8 string: %s\n",
						  __func__, libs[i]);
					g_error_free(error);
					error = NULL;
					continue;
				}
				cwin->cpref->lib_delete =
					g_slist_append(cwin->cpref->lib_delete,
						       g_strdup(file));
				g_free(file);
			}
			g_strfreev(libs);
		}
		else {
			g_error_free(error);
			error = NULL;
			lib_delete_f = TRUE;
		}

		/* Retrieve hidden files option */

		cwin->cpref->show_hidden_files =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_HIDDEN_FILE,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			hidden_f = TRUE;
		}

		/* Retrieve filetree pwd */

		u_file = g_key_file_get_string(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_FILETREE_PWD,
					       &error);
		if (!u_file) {
			g_error_free(error);
			error = NULL;
			file_tree_pwd_f = TRUE;
		} else {
			cwin->cstate->file_tree_pwd = g_filename_from_utf8(u_file,
							   -1, NULL, NULL, &error);
			if (!cwin->cstate->file_tree_pwd) {
				g_warning("(%s): Unable to get filename "
					  "from UTF-8 string: %s\n",
					  __func__, u_file);
				g_error_free(error);
				error = NULL;
				file_tree_pwd_f = TRUE;
			}
			g_free(u_file);
		}

		/* Retrieve album art option */

		cwin->cpref->show_album_art =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_ALBUM_ART,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			album_f = TRUE;
		}

		/* Retrieve album art pattern */

		cwin->cpref->album_art_pattern =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_ALBUM_ART_PATTERN,
					      &error);
		if (!cwin->cpref->album_art_pattern) {
			g_error_free(error);
			error = NULL;
			album_art_pattern_f = TRUE;
		}

		/* Retrieve OSD option */

		cwin->cpref->show_osd =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_OSD,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			osd_f = TRUE;
		}

		/* Retrieve Save playlist option */

		cwin->cpref->save_playlist =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SAVE_PLAYLIST,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			save_playlist_f = TRUE;
		}

		/* Retrieve software mixer option */

		cwin->cpref->software_mixer =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_AUDIO,
					       KEY_SOFWARE_MIXER,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			software_mixer_f = TRUE;
		}

		/* Retrieve shuffle & repeat options */

		cwin->cpref->shuffle =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHUFFLE,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			shuffle_f = TRUE;
		}

		cwin->cpref->repeat =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_REPEAT,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			repeat_f= TRUE;
		}

		/* Retrieve audio sink */

		cwin->cpref->audio_sink =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_AUDIO,
					      KEY_AUDIO_SINK,
					      &error);
		if (!cwin->cpref->audio_sink) {
			g_error_free(error);
			error = NULL;
			audio_sink_f = TRUE;
		}
	}

	/* Fill up with failsafe defaults */

	if (all_f || window_size_f) {
		cwin->cpref->window_width = MIN_WINDOW_WIDTH;
		cwin->cpref->window_height = MIN_WINDOW_HEIGHT;
	}
	if (all_f || libs_f)
		cwin->cpref->library_dir = NULL;
	if (all_f || lib_add_f)
		cwin->cpref->lib_add = NULL;
	if (all_f || lib_delete_f)
		cwin->cpref->lib_delete = NULL;
	if (all_f || album_art_pattern_f)
		cwin->cpref->album_art_pattern = NULL;
	if (all_f || columns_f) {
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(P_TITLE_STR));
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(P_ARTIST_STR));
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(P_ALBUM_STR));
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(P_LENGTH_STR));
	}
	if (all_f || nodes_f) {
		cwin->cpref->library_tree_nodes =
			g_slist_append(cwin->cpref->library_tree_nodes,
				       g_strdup(P_ARTIST_STR));
		cwin->cpref->library_tree_nodes =
			g_slist_append(cwin->cpref->library_tree_nodes,
				       g_strdup(P_ALBUM_STR));
		cwin->cpref->library_tree_nodes =
			g_slist_append(cwin->cpref->library_tree_nodes,
				       g_strdup(P_TITLE_STR));
	}
	if (all_f || col_widths_f) {
		for (i=0; i<4; i++) {
			cwin->cpref->playlist_column_widths =
				g_slist_append(cwin->cpref->playlist_column_widths,
					       GINT_TO_POINTER(DEFAULT_PLAYLIST_COL_WIDTH));
		}
	}
	if (all_f || cur_lib_view_f)
		cwin->cpref->cur_library_view = ARTIST_ALBUM_TRACK;
	if (all_f || hidden_f)
		cwin->cpref->show_hidden_files = FALSE;
	if (all_f || file_tree_pwd_f)
		cwin->cstate->file_tree_pwd = (gchar*)g_get_home_dir();
	if (all_f || album_f)
		cwin->cpref->show_album_art = FALSE;
	if (all_f || osd_f)
		cwin->cpref->show_osd = FALSE;
	if (all_f || save_playlist_f)
		cwin->cpref->save_playlist = TRUE;
	if (all_f || software_mixer_f)
		cwin->cpref->software_mixer = FALSE;
	if (all_f || shuffle_f)
		cwin->cpref->shuffle = FALSE;
	if (all_f || repeat_f)
		cwin->cpref->repeat = FALSE;

	if (all_f || audio_sink_f)
		cwin->cpref->audio_sink = g_strdup(DEFAULT_SINK);

	/* Cleanup */

	g_free(conrc);
	g_free(condir);

	if (err)
		return -1;
	else
		return 0;
}

gint init_musicdbase(struct con_win *cwin)
{
	gint ret;
	const gchar *home;

	CDEBUG("(%s): Initializing music dbase", __func__);

	home = g_get_home_dir();
	cwin->cdbase->db_file = g_strdup_printf("%s%s", home, "/.consonance/condb");

	/* Create the database file */

	ret = sqlite3_open(cwin->cdbase->db_file, &cwin->cdbase->db);
	if (ret) {
		g_critical("(%s): Unable to open/create DB file : %s",
			   __func__, cwin->cdbase->db_file);
		g_free(cwin->cdbase->db_file);
		return -1;
	}

	return init_dbase_schema(cwin);
}

gint init_audio(struct con_win *cwin)
{
	gint id = 0, ret = 0;
	ao_sample_format format;
	ao_device *ao_dev;
	gchar *audio_sink = NULL;

	CDEBUG("(%s): Initializing audio", __func__);

	ao_initialize();

	if (audio_backend) {
		if (!g_ascii_strcasecmp(audio_backend, PULSE_SINK)) {
			audio_sink = PULSE_SINK;
		} else if (!g_ascii_strcasecmp(audio_backend, OSS_SINK)) {
			audio_sink = OSS_SINK;
		} else if (!g_ascii_strcasecmp(audio_backend, ALSA_SINK)) {
			audio_sink = ALSA_SINK;
		}
		else {
			g_warning("Invalid cmdline audio device: %s",
				  audio_backend);
			ao_shutdown();
			ret = -1;
			goto bad1;
		}
	}

	if (!audio_sink)
		audio_sink = cwin->cpref->audio_sink;

	if (!g_ascii_strcasecmp(audio_sink, DEFAULT_SINK))
		id = ao_default_driver_id();
	else
		id = ao_driver_id(audio_sink);

	if (id == -1) {
		g_critical("(%s): %s driver not found",
			   __func__, cwin->cpref->audio_sink);
		ao_shutdown();
		ret = -1;
		goto bad1;
	}

	cwin->clibao->ao_driver_id = id;
	CDEBUG("(%s): Using audio driver: %s (%s) with id: %d",
	       __func__,
	       audio_sink,
	       ao_driver_info(id)->name,
	       id);

	/* Check if audio device is accessible */

	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;

	ao_dev = ao_open_live(id, &format, NULL);

	if (ao_dev == NULL) {
		g_critical("(%s): Unable to open audio device", __func__);
		ret = -1;
		goto bad1;
	}

	/* Check if user wants software mixer */

	if (cwin->cpref->software_mixer) {
		set_soft_mixer(cwin);
		cwin->cmixer->init_mixer(cwin);
		goto bad2;
	}

	/* If not, try initializing HW mixers */

	if (init_audio_mixer(cwin) == -1) {
		g_critical("(%s): Unable to choose a HW audio mixer", __func__);
		set_soft_mixer(cwin);
	}

	if (cwin->cmixer->init_mixer(cwin) == -1) {
		ret = -1;
		goto bad2;
	}

bad2:
	ao_close(ao_dev);
bad1:
	return ret;
}

gint init_threads(struct con_win *cwin)
{
	CDEBUG("(%s): Initializing threads", __func__);

	if (!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();
	cwin->cstate->c_thread = NULL;
	cwin->cstate->c_mutex = g_mutex_new();
	cwin->cstate->c_cond = g_cond_new();

	return 0;
}

gint init_notify(struct con_win *cwin)
{
	if (cwin->cpref->show_osd) {
		if (!notify_init(PACKAGE_NAME))
			return -1;
	}

	return 0;
}

void init_state(struct con_win *cwin)
{
	CDEBUG("(%s): Initializing state", __func__);

	cwin->cstate->rand = g_rand_new();
	cwin->cstate->rand_track_refs = NULL;
	cwin->cstate->state = ST_STOPPED;
}


void init_tag_completion(struct con_win *cwin)
{
	GtkListStore *artist_tag_model, *album_tag_model, *genre_tag_model;

	/* Artist */

	artist_tag_model = gtk_list_store_new(1, G_TYPE_STRING);
	cwin->completion[0] = gtk_entry_completion_new();
	gtk_entry_completion_set_model(cwin->completion[0],
				       GTK_TREE_MODEL(artist_tag_model));
	gtk_entry_completion_set_text_column(cwin->completion[0], 0);
	g_object_unref(artist_tag_model);

	/* Album */

	album_tag_model = gtk_list_store_new(1, G_TYPE_STRING);
	cwin->completion[1] = gtk_entry_completion_new();
	gtk_entry_completion_set_model(cwin->completion[1],
				       GTK_TREE_MODEL(album_tag_model));
	gtk_entry_completion_set_text_column(cwin->completion[1], 0);
	g_object_unref(album_tag_model);

	/* Genre */

	genre_tag_model = gtk_list_store_new(1, G_TYPE_STRING);
	cwin->completion[2] = gtk_entry_completion_new();
	gtk_entry_completion_set_model(cwin->completion[2],
				       GTK_TREE_MODEL(genre_tag_model));
	gtk_entry_completion_set_text_column(cwin->completion[2], 0);
	g_object_unref(genre_tag_model);
}

void init_gui(gint argc, gchar **argv, struct con_win *cwin)
{
	GtkUIManager *menu;
	GtkWidget *vbox, *hbox_panel, *hbox_main, *status_bar, *search_bar, *hsep;
	GError *error = NULL;

	CDEBUG("(%s): Initializing gui", __func__);

	gtk_init(&argc, &argv);

	/* Main window */

	cwin->mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	cwin->pixbuf->pixbuf_app = gdk_pixbuf_new_from_file(SHAREDIR "/data/consonance.png",
							    &error);
	if (!cwin->pixbuf->pixbuf_app) {
		g_warning("(%s): Unable to load app png : %s", __func__, error->message);
		g_error_free(error);
		error = NULL;
	}
	else
		gtk_window_set_icon(GTK_WINDOW(cwin->mainwindow), cwin->pixbuf->pixbuf_app);

	gtk_window_set_title(GTK_WINDOW(cwin->mainwindow), "Consonance");
	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "delete_event",
			 G_CALLBACK(exit_gui), cwin);
	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "destroy",
			 G_CALLBACK(exit_consonance), cwin);

	/* Set Default Size */

	gtk_window_set_default_size(GTK_WINDOW(cwin->mainwindow),
				    cwin->cpref->window_width,
				    cwin->cpref->window_height);

	/* Systray */

	create_status_icon(cwin);

	/* Main Vbox */

	vbox = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(cwin->mainwindow), vbox);

	/* Create hboxen */

	menu = create_menu(cwin);
	hbox_main = create_main_region(cwin);
	hbox_panel = create_panel(cwin);
	status_bar = create_status_bar(cwin);
	search_bar = create_search_bar(cwin);
	hsep = gtk_hseparator_new();

	/* Pack all hboxen into vbox */

	gtk_box_pack_start(GTK_BOX(vbox),
			   gtk_ui_manager_get_widget(menu, "/Menubar"),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(hbox_panel),
			   FALSE,FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(hsep),
			   FALSE,FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(hbox_main),
			   TRUE,TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(search_bar),
			   FALSE,FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(status_bar),
			   FALSE,FALSE, 0);

	/* Show main window, hide search bar */

	gtk_widget_show_all(cwin->mainwindow);
	gtk_widget_hide(cwin->search_bar);

	/* Set initial size of album art frame */

	resize_album_art_frame(cwin);

	gtk_init_add(_init_gui_state, cwin);
}
