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

#ifndef CONSONANCE_H
#define CONSONANCE_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libnotify/notify.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <ao/ao.h>

#include "mp3.h"
#include "wav.h"
#include "flac.h"
#include "oggvorbis.h"

#ifdef ENABLE_MODPLUG
#include "mod.h"
#endif

#define MIN_WINDOW_WIDTH           640
#define MIN_WINDOW_HEIGHT          480
#define BROWSE_MODE_SIZE           30
#define ALBUM_ART_SIZE             60
#define PROGRESS_BAR_WIDTH         300
#define COL_WIDTH_THRESH           30
#define DEFAULT_PLAYLIST_COL_WIDTH 200
#define OSD_TIMEOUT                5000
#define ALBUM_ART_PATTERN_LEN      1024
#define ALBUM_ART_NO_PATTERNS      6
#define TAG_MAX_LEN                256

#define P_TRACK_NO_STR      "#"
#define P_TNO_FULL_STR      "Track No"
#define P_TITLE_STR         "Title"
#define P_ARTIST_STR        "Artist"
#define P_ALBUM_STR         "Album"
#define P_GENRE_STR         "Genre"
#define P_BITRATE_STR       "Bitrate"
#define P_YEAR_STR          "Year"
#define P_LENGTH_STR        "Length"
#define P_FILENAME_STR      "Filename"

/* These are not seen in the playlist columns */
/* Used for library view and preferences */

#define P_FOLDER_STR        "Folder"	/* Containing folder */
#define P_BASENAME_STR      "Basename"	/* Base name of the file */

#define DEFAULT_SINK "default"
#define ALSA_SINK "alsa"
#define OSS_SINK "oss"
#define PULSE_SINK "pulse"
#define DEFAULT_MIXER "default"
#define ALSA_MIXER "alsa"
#define OSS_MIXER "oss"
#define SOFT_MIXER "software"

#define PROGRESS_BAR_TEXT "Scanning"
#define TRACK_PROGRESS_BAR_STOPPED "Stopped"
#define UP_DIR ".."
#define SAVE_PLAYLIST_STATE "con_playlist"

#define DBUS_PATH "/org/consonance/DBus"
#define DBUS_NAME "org.consonance.DBus"
#define DBUS_INTERFACE "org.consonance.DBus"

#define DBUS_SIG_PLAY     "play"
#define DBUS_SIG_STOP     "stop"
#define DBUS_SIG_PAUSE    "pause"
#define DBUS_SIG_NEXT     "next"
#define DBUS_SIG_PREV     "prev"
#define DBUS_SIG_INC_VOL  "inc_vol"
#define DBUS_SIG_DEC_VOL  "dec_vol"
#define DBUS_SIG_SHOW_OSD "show_osd"
#define DBUS_SIG_ADD_FILE "add_files"

#define DBUS_METHOD_CURRENT_STATE "curent_state"

#define GROUP_GENERAL "General"
#define GROUP_LIBRARY "Library"
#define GROUP_AUDIO "Audio"

#define KEY_INSTALLED_VERSION      "installed_version"
#define KEY_LIBRARY_DIR            "library_dir"
#define KEY_LIBRARY_DELETE         "library_delete"
#define KEY_LIBRARY_ADD            "library_add"
#define KEY_FILETREE_PWD           "filetree_pwd"
#define KEY_SHOW_HIDDEN_FILE       "show_hidden_files"
#define KEY_SHOW_ALBUM_ART         "show_album_art"
#define KEY_ALBUM_ART_PATTERN      "album_art_pattern"
#define KEY_SHOW_OSD               "show_osd"
#define KEY_SAVE_PLAYLIST          "save_playlist"
#define KEY_SHUFFLE                "shuffle"
#define KEY_REPEAT                 "repeat"
#define KEY_PLAYLIST_COLUMNS       "playlist_columns"
#define KEY_PLAYLIST_COLUMN_WIDTHS "playlist_column_widths"
#define KEY_LIBRARY_TREE_NODES     "library_tree_nodes"
#define KEY_LIBRARY_VIEW_ORDER     "library_view_order"
#define KEY_LIBRARY_LAST_SCANNED   "library_last_scanned"
#define KEY_WINDOW_SIZE            "window_size"
#define KEY_AUDIO_SINK             "audio_sink"
#define KEY_SOFWARE_MIXER          "software_mixer"

#define TAG_TNO_CHANGED    1<<0
#define TAG_TITLE_CHANGED  1<<1
#define TAG_ARTIST_CHANGED 1<<2
#define TAG_ALBUM_CHANGED  1<<3
#define TAG_GENRE_CHANGED  1<<4
#define TAG_YEAR_CHANGED   1<<5

/* Current playlist movement */

enum {
	PLAYLIST_CURR = 1,
	PLAYLIST_NEXT,
	PLAYLIST_PREV
};

/* Thread commands */

enum {
	CMD_PLAYBACK_STOP = 1,
	CMD_PLAYBACK_PAUSE,
	CMD_PLAYBACK_RESUME,
	CMD_PLAYBACK_SEEK
};

/* Player state */

enum {
	ST_PLAYING = 1,
	ST_STOPPED,
	ST_PAUSED
};

/* Main view notebook pages */

enum {
	PAGE_LIBRARY,
	PAGE_FILE,
};

/* Node types in library view */

enum {
	NODE_GENRE,
	NODE_ARTIST,
	NODE_ALBUM,
	NODE_TRACK,
	NODE_FOLDER,
	NODE_BASENAME
};

/* Columns in Library view */

enum {
	L_PIXBUF,
	L_NODE_DATA,
	L_NODE_TYPE,
	L_LOCATION_ID,
	L_VISIBILE,
	N_L_COLUMNS
};

/* Columns in Playlist view */

enum {
	P_PIXBUF,
	P_PLAYLIST,
	N_PL_COLUMNS
};

/* Columns in File tree view */

enum {
	F_PIXBUF,
	F_NAME,
	F_FILE_TYPE,
	N_F_COLUMNS
};

/* Columns in current playlist view */

enum {
	P_MOBJ_PTR,
	P_TRACK_NO,
	P_TITLE,
	P_ARTIST,
	P_ALBUM,
	P_GENRE,
	P_BITRATE,
	P_YEAR,
	P_LENGTH,
	P_FILENAME,
	P_PLAYED,
	N_P_COLUMNS
};

/* DnD */

enum {
	TARGET_LOCATION_ID,
	TARGET_FILENAME,
	TARGET_PLAYLIST
};

/* Library Views */

enum {
	FOLDER_FILE,
	ARTIST_TRACK,
	ALBUM_TRACK,
	GENRE_TRACK,
	ARTIST_ALBUM_TRACK,
	GENRE_ARTIST_TRACK,
	GENRE_ALBUM_TRACK,
	GENRE_ARTIST_ALBUM_TRACK
};

/* Playlist management */

enum {
	NEW_PLAYLIST,
	APPEND_PLAYLIST,
	SAVE_COMPLETE,
	SAVE_SELECTED
};

/* File Type */

enum {
	FILE_WAV,
	FILE_MP3,
	FILE_FLAC,
	FILE_OGGVORBIS,

#ifdef ENABLE_MODPLUG
	FILE_MODPLUG
#endif
};

/* Node type in Filetree */

enum {
	F_TYPE_FILE,
	F_TYPE_DIR
};

struct tags {
	gchar *title;
	gchar *artist;
	gchar *album;
	gchar *genre;
	guint year;
	guint track_no;
	gint length;
	gint bitrate;
	gint channels;
	gint samplerate;
};

struct pixbuf {
	GdkPixbuf *pixbuf_dir;		/* Folder image in File tree */
	GdkPixbuf *pixbuf_file;		/* File image in File tree */
	GdkPixbuf *pixbuf_artist;	/* Artist image in Library tree */
	GdkPixbuf *pixbuf_album;	/* Album image in Library tree */
	GdkPixbuf *pixbuf_track;	/* Track image in Library tree */
	GdkPixbuf *pixbuf_genre;	/* Genre image in Library tree */
	GdkPixbuf *pixbuf_app;		/* Application logo */
	GtkWidget *image_pause;		/* Play button image */
	GtkWidget *image_play;		/* Pause button image */

};

struct con_pref {
	gchar *installed_version;	/* Installed version from conrc */
	gchar *audio_sink;		/* Audio output */
	gchar *album_art_pattern;	/* Album art file patterns */
	gint window_width;		/* Main window width */
	gint window_height;		/* Main window height */
	gint cur_library_view;		/* Current library view */
	GTimeVal last_rescan_time;	/* Last library rescan timeval */
	GKeyFile *configrc_keyfile;	/* conrc keyfile handle */
	gchar *configrc_file;		/* File location of conrc */
	gboolean show_hidden_files;	/* Flag to show hidden files in File tree */
	gboolean show_album_art;	/* Flag to show album art in Panel */
	gboolean show_osd;		/* Flag to show OSD on desktop */
	gboolean shuffle;		/* Toggle shuffle on/off */
	gboolean repeat;		/* Toggle repeat on/off */
	gboolean save_playlist;		/* Save current playlist on quitting */
	gboolean software_mixer;	/* Use software mixer */
	GSList *library_dir;		/* List of library locations on the filesystem */
	GSList *playlist_columns;	/* List of columns set to visible */
	GSList *playlist_column_widths;	/* List of corresponding column widths */
	GSList *library_tree_nodes;	/* List of nodes in library tree */
	GSList *lib_delete;		/* Libraries deleted from prefs */
	GSList *lib_add;		/* Libraries added to prefs */
	GtkWidget *hidden_files;	/* Hidden files widget */
	GtkWidget *album_art;		/* Album art widget */
	GtkWidget *osd;			/* OSD */
	GtkWidget *save_playlist_w;	/* Save playlist */
	GtkWidget *album_art_pattern_w;	/* Album art file pattern */
	GtkWidget *soft_mixer;		/* Software mixer widget */
	GtkWidget *audio_sink_combo;	/* Audio output driver */
	GtkWidget *library_view;	/* Library list widget */
};

struct musicobject {
	struct tags *tags;		/* Tags of a single musicobject */
	gchar *file;			/* Filename of this musicobject */
	gint file_type;			/* Filetype { FILE_MP3,... } */
};

struct db_result {
	gchar **resultp;		/* The actual result values of a SQLITE3 query*/
	gint no_rows;			/* No. of rows returned in the result */
	gint no_columns;		/* No. of columns returned in the result */
};

struct con_state {
	gboolean unique_instance;		/* If current invocation of app is unique */
	gboolean stop_scan;			/* Flag to stop rescan process */
	gboolean view_change;			/* If library view change is in progress */
	gboolean curr_mobj_clear;		/* Clear curr_mobj flag */
	gboolean advance_track;                 /* Advance next track - set by playback thread */
	gint state;				/* State of the player { ST_STOPPED, ... } */
	gint cmd;				/* Thread Command {CMD_PLAYBACK_STOP, ... } */
	gint seek_len;				/* New seek length to pass to playback thread */
	gint tracks_curr_playlist;		/* Total no. of tracks in the current playlist */
	gint unplayed_tracks;			/* Total no. of tracks that haven't been played */
	gint newsec;				/* Arg for idle func invoked from playback thread */
	gdouble seek_fraction;			/* New seek fraction to pass to playback thread */
	gchar *file_tree_pwd;			/* Current folder on display in File tree */
	gchar *filter_entry;			/* Search entry for filtering library */
	GRand *rand;				/* To generate random numbers */
	GThread *c_thread;			/* Playback thread */
	GMutex *c_mutex;			/* Mutex between playback thread and main process */
	GCond *c_cond;				/* Cond Between playback thread and main process */
	GList *rand_track_refs;			/* List of references maintained in Shuffle mode */
	GtkTreeRowReference *curr_rand_ref;	/* Currently playing track in Shuffle mode */
	GtkTreeRowReference *curr_seq_ref;	/* Currently playing track in non-Shuffle mode */
	struct musicobject *curr_mobj;		/* musicobject of currently playing track */
};

struct con_mixer {
	gchar *mixer_elem;			/* Mixer elem from cmdline */
	glong min_vol;				/* Min. volume obtained from ALSA */
	glong max_vol;				/* Max. volume obtained from ALSA */
	glong curr_vol;				/* Current volume */
	void (*set_volume)(struct con_win *);	/* Callback to set volume */
	void (*inc_volume)(struct con_win *);	/* Increase vol by 1 */
	void (*dec_volume)(struct con_win *);	/* Decrease vol by 1 */
	gint (*init_mixer)(struct con_win *);	/* Callback to initialize mixer */
	void (*deinit_mixer)(struct con_win *); /* Callback to deinit mixer */
	gint (*mute_mixer)(struct con_win *);	/* Callback to mute mixer */
};

struct con_dbase {
	gchar *db_file;		/* Filename of the DB file (~/.condb) */
	sqlite3 *db;		/* SQLITE3 handle of the opened DB */
};

struct con_libao {
	gint ao_driver_id;		/* libao device ID */
	ao_device *ao_dev;		/* libao handle */
	ao_sample_format format;	/* Sample format */
};

struct con_win {
	struct pixbuf *pixbuf;				/* Various pixbufs */
	struct con_pref *cpref;				/* Preferences */
	struct con_state *cstate;			/* Player state */
	struct con_dbase *cdbase;			/* Library DB */
	struct con_mixer *cmixer;			/* Mixer */
	struct con_libao *clibao;			/* libao */
	GtkWidget *mainwindow;				/* Main GTK window */
	GtkWidget *hbox_panel;				/* Panel hbox */
	GtkWidget *album_art_frame;			/* Album art frame */
	GtkWidget *album_art;				/* Album art image */
	GtkWidget *track_progress_bar;			/* Progress bar */
	GtkWidget *shuffle_button;			/* Shuffle tracks */
	GtkWidget *repeat_button;			/* Repeat tracks */
	GtkWidget *prev_button;				/* Previous track */
	GtkWidget *play_button;				/* Play track */
	GtkWidget *stop_button;				/* Stop playing */
	GtkWidget *next_button;				/* Next track */
	GtkWidget *vol_button;				/* Adjust volume */
	GtkWidget *current_playlist;			/* Current playlist view */
	GtkWidget *status_bar;				/* Status bar */
	GtkWidget *search_bar;				/* Search bar (simple) */
	GtkWidget *search_entry;			/* Text entry field for searching */
	GtkWidget *browse_mode;				/* Browse mode ( file / library ) */
	GtkWidget *library_tree;			/* Library tree view */
	GtkWidget *playlist_tree;			/* Playlist tree view */
	GtkWidget *file_tree;				/* File tree view */
	GtkWidget *header_context_menu;			/* Column header right click menu */
	GtkTreeStore *library_store;			/* Library child model */
	GOptionContext *cmd_context;			/* Cmd Line option context */
	GtkStatusIcon *status_icon;			/* Status icon in the system tray */
	GtkEntryCompletion *completion[3];		/* Tag completion */
	GtkUIManager *cp_context_menu;			/* Current playlist right click menu */
	GtkUIManager *playlist_tree_context_menu;	/* Playlist tree right click menu */
	GtkUIManager *library_tree_context_menu;	/* Library tree right click menu */
	GtkUIManager *library_page_context_menu;	/* Library page right click menu */
	GtkUIManager *file_tree_dir_context_menu;	/* File tree right click menu (Dir) */
	GtkUIManager *file_tree_file_context_menu;	/* File tree right click menu (File) */
	GtkUIManager *systray_menu;			/* System tray right click menu */
	DBusConnection *con_dbus;			/* DBUS connection */
};

extern gulong switch_cb_id;
extern gboolean show_debug;
extern const gchar *mime_mpeg[];
extern const gchar *mime_wav[];
extern const gchar *mime_flac[];
extern const gchar *mime_ogg[];
extern const gchar *mime_image[];

/* Conversion routine from alsaplayer */

static inline long volume_convert(glong val, long omin, long omax, long nmin, long nmax) {
        float orange = omax - omin, nrange = nmax - nmin;

        if (orange == 0)
                return 0;
        return ((nrange * (val - omin)) + (orange / 2)) / orange + nmin;
}

/* Convenience macros */

#define for_each_result_row(result, i)					\
	for (i=result.no_columns; i<((result.no_rows+1)*(result.no_columns)); i+=result.no_columns)

#define SCALE_UP_VOL(vol) volume_convert(vol, cwin->cmixer->min_vol, cwin->cmixer->max_vol, 0, 100)
#define SCALE_DOWN_VOL(vol) volume_convert(vol, 0, 100, cwin->cmixer->min_vol, cwin->cmixer->max_vol)

/* Debugging */

#define CDEBUG(fmt, ...)			\
	if (show_debug)				\
		g_debug(fmt, ##__VA_ARGS__);

/* Menu actions */

void open_file_action(GtkAction *action, struct con_win *cwin);
void quit_action(GtkAction *action, struct con_win *cwin);
void expand_all_action(GtkAction *action, struct con_win *cwin);
void collapse_all_action(GtkAction *action, struct con_win *cwin);
void search_library_action(GtkAction *action, struct con_win *cwin);
void search_playlist_action(GtkAction *action, struct con_win *cwin);
void pref_action(GtkAction *action, struct con_win *cwin);
void rescan_library_action(GtkAction *action, struct con_win *cwin);
void update_library_action(GtkAction *action, struct con_win *cwin);
void add_all_action(GtkAction *action, struct con_win *cwin);
void statistics_action(GtkAction *action, struct con_win *cwin);
void about_action(GtkAction *action, struct con_win *cwin);

/* Panel actions */

gboolean update_current_song_info(gpointer data);
void __update_current_song_info(struct con_win *cwin, gint length);
void unset_current_song_info(struct con_win *cwin);
gboolean update_track_progress_bar(gpointer data);
void __update_track_progress_bar(struct con_win *cwin, gint length);
void unset_track_progress_bar(struct con_win *cwin);
void track_progress_change_cb(GtkWidget *widget,
			      GdkEventButton *event,
			      struct con_win *cwin);
void update_album_art(struct musicobject *mobj, struct con_win *cwin);
void unset_album_art(struct con_win *cwin);
void shuffle_button_handler(GtkToggleButton *button, struct con_win *cwin);
void repeat_button_handler(GtkToggleButton *button, struct con_win *cwin);
void play_button_handler(GtkButton *button, struct con_win *cwin);
void stop_button_handler(GtkButton *button, struct con_win *cwin);
void prev_button_handler(GtkButton *button, struct con_win *cwin);
void next_button_handler(GtkButton *button, struct con_win *cwin);
void vol_button_handler(GtkScaleButton *button, gdouble value, struct con_win *cwin);
void play_button_toggle_state(struct con_win *cwin);
void album_art_toggle_state(struct con_win *cwin);
void resize_album_art_frame(struct con_win *cwin);

/* File tree functions */

void __non_recur_add(gchar *dir_name, gboolean init, struct con_win *cwin);
void __recur_add(gchar *dir_name, struct con_win *cwin);
void update_file_tree(gchar *new_dir, struct con_win *cwin);
void populate_file_tree(const gchar *new_dir, struct con_win *cwin);
void file_tree_row_activated_cb(GtkTreeView *file_tree,
				GtkTreePath *path,
				GtkTreeViewColumn *column,
				struct con_win *cwin);
gboolean file_tree_right_click_cb(GtkWidget *widget,
				  GdkEventButton *event,
				  struct con_win *cwin);
void file_tree_play(GtkAction *action, struct con_win *cwin);
void file_tree_enqueue(GtkAction *action, struct con_win *cwin);
void file_tree_enqueue_recur(GtkAction *action, struct con_win *cwin);
void file_tree_enqueue_non_recur(GtkAction *action, struct con_win *cwin);
void dnd_file_tree_get(GtkWidget *widget,
		       GdkDragContext *context,
		       GtkSelectionData *data,
		       guint info,
		       guint time,
		       struct con_win *cwin);
void browse_mode_switch_page_cb(GtkNotebook *notebook,
				GtkNotebookTab arg1,
				guint pgnum,
				struct con_win *cwin);
gint file_tree_sort_func(GtkTreeModel *model, GtkTreeIter *a,
			 GtkTreeIter *b, gpointer data);

/* Musicobject functions */

struct musicobject* new_musicobject_from_file(gchar *file);
struct musicobject* new_musicobject_from_db(gint location_id, struct con_win *cwin);
void delete_musicobject(struct musicobject *mobj);
void test_delete_musicobject(struct musicobject *mobj, struct con_win *cwin);

/* Tag functions */

gboolean get_wav_info(gchar *file, struct tags *tags);
gboolean get_mp3_info(gchar *file, struct tags *tags);
gboolean get_flac_info(gchar *file, struct tags *tags);
gboolean get_ogg_info(gchar *file, struct tags *tags);
#ifdef ENABLE_MODPLUG
gboolean get_mod_info(gchar *file, struct tags *tags);
#endif
gboolean save_tags_to_file(gchar *file, struct tags *tags,
			   int changed, struct con_win *cwin);
void tag_update(GArray *loc_arr, GArray *file_arr, gint changed, struct tags *ntag,
		struct con_win *cwin);
gint tag_edit_dialog(struct tags *otag, struct tags *ntag,
		     struct con_win *cwin);
void refresh_tag_completion_entries(struct con_win *cwin);

/* Library manipulation functions */

void library_tree_row_activated_cb(GtkTreeView *library_tree,
				   GtkTreePath *path,
				   GtkTreeViewColumn *column,
				   struct con_win *cwin);
gboolean library_tree_right_click_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin);
gboolean library_page_right_click_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin);
void dnd_library_tree_get(GtkWidget *widget,
			  GdkDragContext *context,
			  GtkSelectionData *data,
			  guint info,
			  guint time,
			  struct con_win *cwin);
gboolean simple_library_search_keyrelease_handler(GtkWidget *entry,
						  GdkEventKey *event,
						  struct con_win *cwin);
void cancel_simple_library_search_handler(GtkButton *button, struct con_win *cwin);
void clear_library_search(struct con_win *cwin);
void folder_file_library_tree(GtkAction *action, struct con_win *cwin);
void artist_track_library_tree(GtkAction *action, struct con_win *cwin);
void album_track_library_tree(GtkAction *action, struct con_win *cwin);
void genre_track_library_tree(GtkAction *action, struct con_win *cwin);
void artist_album_track_library_tree(GtkAction *action, struct con_win *cwin);
void genre_album_track_library_tree(GtkAction *action, struct con_win *cwin);
void genre_artist_track_library_tree(GtkAction *action, struct con_win *cwin);
void genre_artist_album_track_library_tree(GtkAction *action, struct con_win *cwin);
void library_tree_play(GtkAction *action, struct con_win *cwin);
void library_tree_enqueue(GtkAction *action, struct con_win *cwin);
void library_tree_edit_tags(GtkAction *action, struct con_win *cwin);
void init_library_view(struct con_win *cwin);

/* DB (Sqlite) Functions */

gint add_new_artist_db(gchar *artist, struct con_win *cwin);
gint add_new_album_db(gchar *album, struct con_win *cwin);
gint add_new_genre_db(gchar *genre, struct con_win *cwin);
gint add_new_year_db(guint year, struct con_win *cwin);
gint add_new_location_db(gchar *location, struct con_win *cwin);
void add_track_playlist_db(gchar *file, gint playlist_id, struct con_win *cwin);
gint find_artist_db(const gchar *artist, struct con_win *cwin);
gint find_album_db(const gchar *album, struct con_win *cwin);
gint find_genre_db(const gchar *genre, struct con_win *cwin);
gint find_year_db(gint year, struct con_win *cwin);
gint find_location_db(const gchar *location, struct con_win *cwin);
gint find_playlist_db(const gchar *playlist, struct con_win *cwin);
void update_track_db(gint location_id, gint changed,
		     gint track_no, gchar *title,
		     gint artist_id, gint album_id, gint genre_id, gint year_id,
		     struct con_win *cwin);
gint add_new_playlist_db(const gchar *playlist, struct con_win *cwin);
gchar** get_playlist_names_db(struct con_win *cwin);
void delete_playlist_db(gchar *playlist, struct con_win *cwin);
void flush_playlist_db(gint playlist_id, struct con_win *cwin);
void flush_db(struct con_win *cwin);
void rescan_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin);
void update_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin);
void delete_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin);
gint init_dbase_schema(struct con_win *cwin);
gint drop_dbase_schema(struct con_win *cwin);
gboolean exec_sqlite_query(gchar *query, struct con_win *cwin,
			   struct db_result *result);

/* Playlist mgmt functions */

void add_playlist_current_playlist(gchar *playlist, struct con_win *cwin);
void playlist_tree_row_activated_cb(GtkTreeView *playlist_tree,
				    GtkTreePath *path,
				    GtkTreeViewColumn *column,
				    struct con_win *cwin);
gboolean playlist_tree_right_click_cb(GtkWidget *widget,
				      GdkEventButton *event,
				      struct con_win *cwin);
void playlist_tree_play(GtkAction *action, struct con_win *cwin);
void playlist_tree_enqueue(GtkAction *action, struct con_win *cwin);
void playlist_tree_delete(GtkAction *action, struct con_win *cwin);
void dnd_playlist_tree_get(GtkWidget *widget,
			   GdkDragContext *context,
			   GtkSelectionData *data,
			   guint info,
			   guint time,
			   struct con_win *cwin);
void save_playlist(gint playlist_id, gint type, struct con_win *cwin);
void new_playlist(const gchar *playlist, gint type, struct con_win *cwin);
void append_playlist(const gchar *playlist, gint type, struct con_win *cwin);
void init_playlist_view(struct con_win *cwin);

/* Current playlist */

void update_current_state(GThread *thread,
			  GtkTreePath *path,
			  gint action,
			  struct con_win *cwin);
struct musicobject* current_playlist_mobj_at_path(GtkTreePath *path,
						  struct con_win *cwin);
void reset_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin);
void current_playlist_clear_dirty_all(struct con_win *cwin);
GtkTreePath* current_playlist_get_selection(struct con_win *cwin);
GtkTreePath* current_playlist_get_next(struct con_win *cwin);
GtkTreePath* current_playlist_get_prev(struct con_win *cwin);
void init_current_playlist_columns(struct con_win *cwin);
void remove_current_playlist(GtkAction *action, struct con_win *cwin);
void crop_current_playlist(GtkAction *action, struct con_win *cwin);
void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin);
void track_properties_current_playlist(GtkAction *action, struct con_win *cwin);
void clear_current_playlist(GtkAction *action, struct con_win *cwin);
void append_current_playlist(struct musicobject *mobj, struct con_win *cwin);
void clear_sort_current_playlist(GtkAction *action, struct con_win *cwin);
void save_selected_playlist(GtkAction *action, struct con_win *cwin);
void save_current_playlist(GtkAction *action, struct con_win *cwin);
void play_first_current_playlist(struct con_win *cwin);
void play_prev_track(struct con_win *cwin);
void play_next_track(struct con_win *cwin);
void play_track(struct con_win *cwin);
void pause_resume_track(struct con_win *cwin);
void current_playlist_row_activated_cb(GtkTreeView *current_playlist,
				       GtkTreePath *path,
				       GtkTreeViewColumn *column,
				       struct con_win *cwin);
gboolean current_playlist_right_click_cb(GtkWidget *widget,
					 GdkEventButton *event,
					 struct con_win *cwin);
gboolean header_right_click_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       struct con_win *cwin);
gboolean dnd_current_playlist_drop(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   guint time,
				   struct con_win *cwin);
void dnd_current_playlist_received(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   GtkSelectionData *data,
				   guint info,
				   guint time,
				   struct con_win *cwin);
gboolean current_playlist_search_compare(GtkTreeModel *model,
					 gint column,
					 const gchar *key,
					 GtkTreeIter *iter,
					 gpointer data);
void save_current_playlist_state(struct con_win *cwin);
void init_current_playlist_view(struct con_win *cwin);
void playlist_track_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_title_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_artist_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_album_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_genre_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_bitrate_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_year_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_length_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
void playlist_filename_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin);
gint compare_track_no(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);
gint compare_bitrate(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);
gint compare_year(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);
gint compare_length(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);

/* Preferences */

void save_preferences(struct con_win *cwin);
void preferences_dialog(struct con_win *cwin);
void free_library_dir(struct con_win *cwin);
void free_library_add_dir(struct con_win *cwin);
void free_library_delete_dir(struct con_win *cwin);
void free_playlist_columns(struct con_win *cwin);
void free_library_tree_nodes(struct con_win *cwin);

/* Threads */

GThread* start_playback(struct musicobject *mobj, struct con_win *cwin);
gboolean advance_playback(gpointer data);
void pause_playback(struct con_win *cwin);
void resume_playback(struct con_win *cwin);
void stop_playback(struct con_win *cwin);
void seek_playback(struct con_win *cwin, gint seek_len, gdouble seek_fraction);
gint process_thread_command(struct con_win *cwin);

/* Audio functions */

void set_alsa_mixer(struct con_win *cwin, gchar *mixer_elem);
void set_oss_mixer(struct con_win *cwin, gchar *mixer_elem);
void set_soft_mixer(struct con_win *cwin);
void soft_volume_apply(gchar *buffer, gint buflen, struct con_win *cwin);
gint open_audio_device(gint samplerate, gint channels,
		       gboolean resume, struct con_win *cwin);

/* Systray functions */

void show_osd(struct con_win *cwin);
void status_icon_activate(GtkStatusIcon *status_icon, struct con_win *cwin);
void status_icon_tooltip_update(struct con_win *cwin);
void unset_status_icon_tooltip(struct con_win *cwin);
void status_icon_popup_menu(GtkStatusIcon *status_icon,
			    guint button,
			    guint activate_time,
			    struct con_win *cwin);
void systray_play(GtkAction *action, struct con_win *cwin);
void systray_stop(GtkAction *action, struct con_win *cwin);
void systray_pause(GtkAction *action, struct con_win *cwin);
void systray_prev(GtkAction *action, struct con_win *cwin);
void systray_next(GtkAction *action, struct con_win *cwin);
void systray_quit(GtkAction *action, struct con_win *cwin);

/* Command line functions */

gboolean cmd_version(const gchar *opt_name, const gchar *val,
		     struct con_win *cwin, GError **error);
gboolean cmd_play(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_stop(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_pause(const gchar *opt_name, const gchar *val,
		   struct con_win *cwin, GError **error);
gboolean cmd_prev(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_next(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_inc_volume(const gchar *opt_name, const gchar *val,
			struct con_win *cwin, GError **error);
gboolean cmd_dec_volume(const gchar *opt_name, const gchar *val,
			struct con_win *cwin, GError **error);
gboolean cmd_show_osd(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error);
gboolean cmd_current_state(const gchar *opt_name, const gchar *val,
			   struct con_win *cwin, GError **error);
gboolean cmd_add_file(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error);

/* D-BUS functions */

DBusHandlerResult dbus_filter_handler(DBusConnection *conn,
				      DBusMessage *msg,
				      gpointer data);
void dbus_send_signal(const gchar *signal, struct con_win *cwin);

/* Utilities */

gboolean is_hidden_file(const gchar *file);
gboolean is_playable_file(const gchar *file);
gboolean is_base_dir_and_accessible(gchar *file, struct con_win *cwin);
gboolean is_dir_and_accessible(gchar *dir, struct con_win *cwin);
gint dir_file_count(gchar *dir_name, gint call_recur);
gchar* sanitize_string_sqlite3(gchar *str);
gint get_file_type(gchar *file);
gboolean is_image_file(gchar *file);
gchar* convert_length_str(gint length);
gboolean is_present_str_list(const gchar *str, GSList *list);
GSList* delete_from_str_list(const gchar *str, GSList *list);
gchar* get_containing_folder(gchar *path);
void free_str_list(GSList *list);
gint compare_utf8_str(gchar *str1, gchar *str2);
gboolean validate_album_art_pattern(const gchar *pattern);
gboolean is_incompatible_upgrade(struct con_win *cwin);

/* GUI */

GtkUIManager* create_menu(struct con_win *cwin);
GtkWidget* create_main_region(struct con_win *cwin);
GtkWidget* create_panel(struct con_win *cwin);
GtkWidget* create_status_bar(struct con_win *cwin);
GtkWidget* create_search_bar(struct con_win *cwin);
void create_status_icon(struct con_win *cwin);
gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin);

/* Init */

gint init_dbus(struct con_win *cwin);
gint init_dbus_handlers(struct con_win *cwin);
gint init_options(struct con_win *cwin, int argc, char **argv);
gint init_config(struct con_win *cwin);
gint init_musicdbase(struct con_win *cwin);
gint init_audio(struct con_win *cwin);
gint init_threads(struct con_win *cwin);
gint init_notify(struct con_win *cwin);
void init_state(struct con_win *cwin);
void init_tag_completion(struct con_win *cwin);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* Others */

void exit_consonance(GtkWidget *widget, struct con_win *cwin);

#endif /* CONSONANCE_H */
