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

gulong switch_cb_id;

gchar *main_menu_xml = "<ui>					\
	<menubar name=\"Menubar\">				\
		<menu action=\"FileMenu\">			\
			<menuitem action=\"Open File\"/>	\
			<separator/>				\
			<menuitem action=\"Quit\"/>		\
		</menu>						\
		<menu action=\"EditMenu\">			\
			<menuitem action=\"Expand All\"/>	\
			<menuitem action=\"Collapse All\"/>	\
			<menuitem action=\"Search Library\"/>	\
			<menuitem action=\"Search Playlist\"/>	\
			<separator/>				\
			<menuitem action=\"Preferences\"/>	\
		</menu>						\
		<menu action=\"ToolsMenu\">			\
			<menuitem action=\"Rescan Library\"/>	\
			<menuitem action=\"Update Library\"/>	\
			<menuitem action=\"Add All\"/>		\
			<menuitem action=\"Statistics\"/>	\
		</menu>						\
		<menu action=\"HelpMenu\">			\
			<menuitem action=\"About\"/>		\
		</menu>						\
	</menubar>						\
</ui>";

gchar *cp_context_menu_xml = "<ui>		    		\
	<popup>					    		\
	<menuitem action=\"Remove\"/>		    		\
	<menuitem action=\"Crop\"/>		    		\
	<menuitem action=\"Edit tags\"/>			\
	<menuitem action=\"Properties\"/>	    		\
	<separator/>				    		\
	<menuitem action=\"Save Selected as Playlist\"/>	\
	<menuitem action=\"Save Complete Playlist\"/>		\
	<menuitem action=\"Clear Playlist\"/>	    		\
	<separator/>				    		\
	<menuitem action=\"Clear Sort\"/>	    		\
	</popup>				    		\
	</ui>";

gchar *playlist_tree_context_menu_xml = "<ui>	\
	<popup>					\
	<menuitem action=\"Play\"/>		\
	<menuitem action=\"Enqueue\"/>		\
	<menuitem action=\"Delete\"/>		\
	</popup>				\
	</ui>";

gchar *library_tree_context_menu_xml = "<ui>		\
	<popup>						\
	<menuitem action=\"Play\"/>			\
	<menuitem action=\"Enqueue\"/>			\
	<menuitem action=\"Edit\"/>			\
	</popup>					\
	</ui>";

gchar *file_tree_dir_context_menu_xml = "<ui>		\
	<popup>						\
	<menuitem action=\"REnqueue\"/>			\
	<menuitem action=\"NEnqueue\"/>			\
	</popup>					\
	</ui>";

gchar *file_tree_file_context_menu_xml = "<ui>		\
	<popup>						\
	<menuitem action=\"Play\"/>			\
	<menuitem action=\"Enqueue\"/>			\
	</popup>					\
	</ui>";

gchar *library_page_context_menu_xml = "<ui>		\
	<popup>						\
	<menuitem action=\"folder_file\"/>		\
	<separator/>					\
	<menuitem action=\"artist_track\"/>		\
	<menuitem action=\"album_track\"/>		\
	<menuitem action=\"genre_track\"/>		\
	<separator/>					\
	<menuitem action=\"artist_album_track\"/>	\
	<menuitem action=\"genre_artist_track\"/>	\
	<menuitem action=\"genre_album_track\"/>	\
	<separator/>					\
	<menuitem action=\"genre_artist_album_track\"/>	\
	</popup>					\
	</ui>";

gchar *systray_menu_xml = "<ui>		\
	<popup>				\
	<menuitem action=\"Play\"/>	\
	<menuitem action=\"Stop\"/>	\
	<menuitem action=\"Prev\"/>	\
	<menuitem action=\"Next\"/>	\
	<menuitem action=\"Pause\"/>	\
	<separator/>			\
	<menuitem action=\"Quit\"/>	\
	</popup>			\
	</ui>";

GtkActionEntry main_aentries[] = {
	{"FileMenu", NULL, "_File"},
	{"EditMenu", NULL,"_Edit"},
	{"ToolsMenu", NULL, "_Tools"},
	{"HelpMenu", NULL, "_Help"},
	{"Open File", GTK_STOCK_OPEN, "_Open File",
	 "<Control>O", "Open a media file", G_CALLBACK(open_file_action)},
	{"Quit", GTK_STOCK_QUIT, "_Quit",
	 "<Control>Q", "Quit consonance", G_CALLBACK(quit_action)},
	{"Expand All", GTK_STOCK_GOTO_LAST, "_Expand All in Library",
	 NULL, "Expand All in Library", G_CALLBACK(expand_all_action)},
	{"Collapse All", GTK_STOCK_GOTO_FIRST, "_Collapse All in Library",
	 NULL, "Collapse All in Library", G_CALLBACK(collapse_all_action)},
	{"Search Library", GTK_STOCK_FIND, "Search Library",
	 NULL, "Search Library", G_CALLBACK(search_library_action)},
	{"Search Playlist", GTK_STOCK_FIND, "Search Playlist",
	 "<Control><Shift>F", "Search Playlist", G_CALLBACK(search_playlist_action)},
	{"Preferences", GTK_STOCK_PREFERENCES, "_Preferences",
	 "<Control>P", "Set preferences", G_CALLBACK(pref_action)},
	{"Rescan Library", GTK_STOCK_EXECUTE, "_Rescan Library",
	 NULL, "Rescan Library", G_CALLBACK(rescan_library_action)},
	{"Update Library", GTK_STOCK_EXECUTE, "_Update Library",
	 NULL, "Update Library", G_CALLBACK(update_library_action)},
	{"Add All", GTK_STOCK_SELECT_ALL, "_Add All to Current Playlist",
	 NULL, "Add All", G_CALLBACK(add_all_action)},
	{"Statistics", GTK_STOCK_INFO, "_Statistics",
	 NULL, "Statistics", G_CALLBACK(statistics_action)},
	{"About", GTK_STOCK_ABOUT, "About",
	 NULL, "About Consonance", G_CALLBACK(about_action)}
};

GtkActionEntry cp_context_aentries[] = {
	{"Remove", GTK_STOCK_REMOVE, "Remove",
	 "Delete", "Delete this entry", G_CALLBACK(remove_current_playlist)},
	{"Crop", GTK_STOCK_CUT, "Crop",
	 "<Control>C", "Crop the playlist", G_CALLBACK(crop_current_playlist)},
	{"Edit tags", GTK_STOCK_EDIT, "Edit tags",
	 "<Control>E", "Edit tags for this track", G_CALLBACK(edit_tags_current_playlist)},
	{"Properties", GTK_STOCK_PROPERTIES, "Properties",
	 NULL, "Track Properties", G_CALLBACK(track_properties_current_playlist)},
	{"Save Selected as Playlist", GTK_STOCK_SAVE, "Save Selected as Playlist",
	 NULL, "Save selected tracks as playlist", G_CALLBACK(save_selected_playlist)},
	{"Save Complete Playlist", GTK_STOCK_SAVE, "Save Complete Playlist",
	 NULL, "Save the complete playlist", G_CALLBACK(save_current_playlist)},
	{"Clear Playlist", GTK_STOCK_CLEAR, "Clear Playlist",
	 "<Control>L", "Clear the playlist", G_CALLBACK(clear_current_playlist)},
	{"Clear Sort", GTK_STOCK_REFRESH, "Clear Sort",
	 "<Control>S", "Clear Sort", G_CALLBACK(clear_sort_current_playlist)}
};

GtkActionEntry playlist_tree_context_aentries[] = {
	{"Play", GTK_STOCK_MEDIA_PLAY, "Play",
	 NULL, "Play", G_CALLBACK(playlist_tree_play)},
	{"Enqueue", GTK_STOCK_COPY, "Enqueue",
	 NULL, "Enqueue", G_CALLBACK(playlist_tree_enqueue)},
	{"Delete", GTK_STOCK_REMOVE, "Delete",
	 NULL, "Delete", G_CALLBACK(playlist_tree_delete)}
};

GtkActionEntry library_tree_context_aentries[] = {
	{"Play", GTK_STOCK_MEDIA_PLAY, "Play",
	 NULL, "Play", G_CALLBACK(library_tree_play)},
	{"Enqueue", GTK_STOCK_COPY, "Enqueue",
	 NULL, "Enqueue", G_CALLBACK(library_tree_enqueue)},
	{"Edit", GTK_STOCK_EDIT, "Edit tags",
	 NULL, "Edit tags", G_CALLBACK(library_tree_edit_tags)}
};

GtkActionEntry file_tree_dir_context_aentries[] = {
	{"REnqueue", GTK_STOCK_COPY, "Enqueue (Recursive)",
	 NULL, "Enqueue (Recursive)", G_CALLBACK(file_tree_enqueue_recur)},
	{"NEnqueue", GTK_STOCK_COPY, "Enqueue (Non-Recursive)",
	 NULL, "Enqueue (Non Recursive)", G_CALLBACK(file_tree_enqueue_non_recur)}
};

GtkActionEntry file_tree_file_context_aentries[] = {
	{"Play", GTK_STOCK_MEDIA_PLAY, "Play",
	 NULL, "Play", G_CALLBACK(file_tree_play)},
	{"Enqueue", GTK_STOCK_COPY, "Enqueue",
	 NULL, "Enqueue", G_CALLBACK(file_tree_enqueue)}
};

GtkActionEntry library_page_context_aentries[] = {
	{"folder_file", GTK_STOCK_REFRESH, "Folder / File",
	 NULL, "Folder / File", G_CALLBACK(folder_file_library_tree)},
	{"artist_track", GTK_STOCK_REFRESH, "Artist / Track",
	 NULL, "Artist / Track", G_CALLBACK(artist_track_library_tree)},
	{"album_track", GTK_STOCK_REFRESH, "Album / Track",
	 NULL, "Album / Track", G_CALLBACK(album_track_library_tree)},
	{"genre_track", GTK_STOCK_REFRESH, "Genre / Track",
	 NULL, "Genre / Track", G_CALLBACK(genre_track_library_tree)},
	{"artist_album_track", GTK_STOCK_REFRESH, "Artist / Album / Track",
	 NULL, "Artist / Album / Track", G_CALLBACK(artist_album_track_library_tree)},
	{"genre_album_track", GTK_STOCK_REFRESH, "Genre / Album / Track",
	 NULL, "Genre / Album / Track", G_CALLBACK(genre_album_track_library_tree)},
	{"genre_artist_track", GTK_STOCK_REFRESH, "Genre / Artist / Track",
	 NULL, "Genre / Artist / Track", G_CALLBACK(genre_artist_track_library_tree)},
	{"genre_artist_album_track", GTK_STOCK_REFRESH, "Genre / Artist / Album / Track",
	 NULL, "Genre / Artist / Album / Track", G_CALLBACK(genre_artist_album_track_library_tree)}
};

GtkActionEntry systray_menu_aentries[] = {
	{"Play", GTK_STOCK_MEDIA_PLAY, "Play",
	 NULL, "Play", G_CALLBACK(systray_play)},
	{"Stop", GTK_STOCK_MEDIA_STOP, "Stop",
	 NULL, "Stop", G_CALLBACK(systray_stop)},
	{"Prev", GTK_STOCK_MEDIA_PREVIOUS, "Prev",
	 NULL, "Prev", G_CALLBACK(systray_prev)},
	{"Next", GTK_STOCK_MEDIA_NEXT, "Next",
	 NULL, "Next", G_CALLBACK(systray_next)},
	{"Pause", GTK_STOCK_MEDIA_PAUSE, "Pause [T]",
	 NULL, "Pause", G_CALLBACK(systray_pause)},
	{"Quit", GTK_STOCK_QUIT, "Quit",
	 NULL, "Quit", G_CALLBACK(systray_quit)}
};

GtkTargetEntry tentries[] = {
	{"LOCATION_ID", GTK_TARGET_SAME_APP, TARGET_LOCATION_ID},
	{"FILENAME", GTK_TARGET_SAME_APP, TARGET_FILENAME},
	{"PLAYLIST", GTK_TARGET_SAME_APP, TARGET_PLAYLIST}
};

/****************/
/* Library tree */
/****************/

static GtkUIManager* create_library_tree_context_menu(GtkWidget *library_tree,
						      struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Library Tree Context Actions");
	context_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       library_tree_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create library tree context menu, err : %s",
			   __func__, error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     library_tree_context_aentries,
				     G_N_ELEMENTS(library_tree_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkWidget* create_library_tree(struct con_win *cwin)
{
	GError *error = NULL;
	GtkWidget *library_tree;
	GtkTreeModel *library_filter_tree;
	GtkTreeStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	gint width, height;

	/* Create the tree store */

	store = gtk_tree_store_new(N_L_COLUMNS,
				   GDK_TYPE_PIXBUF, /* Pixbuf */
				   G_TYPE_STRING,   /* Node */
				   G_TYPE_INT,      /* Node type : Artist / Album / Track */
				   G_TYPE_INT,      /* Location id (valid only for Track) */
				   G_TYPE_BOOLEAN); /* Row visibility */


	/* Create the filter model */

	library_filter_tree = gtk_tree_model_filter_new(GTK_TREE_MODEL(store), NULL);
	gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(library_filter_tree),
						 L_VISIBILE);
	/* Create the tree view */

	library_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(library_filter_tree));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(library_tree), FALSE);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(library_tree), TRUE);

	/* Selection mode is multiple */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(library_tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Load pixbufs */

	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);

	cwin->pixbuf->pixbuf_artist = gdk_pixbuf_new_from_file_at_scale(SHAREDIR
									"/data/artist.png",
									width,
									height,
									TRUE,
									&error);
	if (!cwin->pixbuf->pixbuf_artist) {
		g_warning("(%s): Unable to load artist png : %s", __func__, error->message);
		g_error_free(error);
		error = NULL;
	}
	cwin->pixbuf->pixbuf_album = gdk_pixbuf_new_from_file_at_scale(SHAREDIR
								       "/data/album.png",
								       width,
								       height,
								       TRUE,
								       &error);
	if (!cwin->pixbuf->pixbuf_album) {
		g_warning("(%s): Unable to load album png : %s", __func__, error->message);
		g_error_free(error);
		error = NULL;
	}
	cwin->pixbuf->pixbuf_track = gdk_pixbuf_new_from_file_at_scale(SHAREDIR
								       "/data/track.png",
								       width,
								       height,
								       TRUE,
								       &error);
	if (!cwin->pixbuf->pixbuf_track) {
		g_warning("(%s): Unable to load track png : %s", __func__, error->message);
		g_error_free(error);
		error = NULL;
	}
	cwin->pixbuf->pixbuf_genre = gdk_pixbuf_new_from_file_at_scale(SHAREDIR
								       "/data/genre.png",
								       width,
								       height,
								       TRUE,
								       &error);
	if (!cwin->pixbuf->pixbuf_genre) {
		g_warning("(%s): Unable to load genre png : %s", __func__, error->message);
		g_error_free(error);
		error = NULL;
	}

	/* Create column and cell renderers */

	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "pixbuf", L_PIXBUF,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "text", L_NODE_DATA,
					    NULL);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(library_tree), column);

	cwin->library_store = store;
	cwin->library_tree = library_tree;
	g_signal_connect(G_OBJECT(library_tree), "row-activated",
			 G_CALLBACK(library_tree_row_activated_cb), cwin);

	/* Create right click popup menu */

	cwin->library_tree_context_menu = create_library_tree_context_menu(library_tree,
									   cwin);

	/* Signal handler for right-clicking */

	g_signal_connect(G_OBJECT(GTK_WIDGET(library_tree)), "button-press-event",
			 G_CALLBACK(library_tree_right_click_cb), cwin);

	g_object_unref(library_filter_tree);
	
	return library_tree;
}

/*************/
/* File Tree */
/*************/

static GtkUIManager* create_file_tree_dir_context_menu(GtkWidget *file_tree,
						       struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("File Tree Dir Context Actions");
	context_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       file_tree_dir_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create file tree dir context menu, err : %s",
			   __func__,
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     file_tree_dir_context_aentries,
				     G_N_ELEMENTS(file_tree_dir_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkUIManager* create_file_tree_file_context_menu(GtkWidget *file_tree,
							struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("File Tree File Context Actions");
	context_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       file_tree_file_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create file tree dir context menu, err : %s",
			   __func__,
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     file_tree_file_context_aentries,
				     G_N_ELEMENTS(file_tree_file_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkWidget* create_file_tree(struct con_win *cwin)
{
	GtkWidget *file_tree;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeSortable *sortable;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GError *error = NULL;

	/* Create the tree store */

	store = gtk_list_store_new(N_F_COLUMNS,
				   GDK_TYPE_PIXBUF,
				   G_TYPE_STRING,
				   G_TYPE_INT);

	/* Create the tree view */

	file_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(file_tree));
	sortable = GTK_TREE_SORTABLE(model);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(file_tree), FALSE);
	gtk_tree_sortable_set_sort_column_id(sortable, F_NAME, GTK_SORT_ASCENDING);
	gtk_tree_sortable_set_sort_func(sortable,
					F_NAME,
					file_tree_sort_func,
					(gpointer)cwin,
					NULL);

	/* Create pixbufs */

	cwin->pixbuf->pixbuf_dir = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
							    "gtk-directory",
							    16, 0, &error);
	if( error != NULL )
		g_warning("(%s): Unable to load gtk-directory icon, err : %s",
			  __func__,
			  error->message);
	cwin->pixbuf->pixbuf_file = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
							     "gtk-file",
							     16, 0, &error);
	if( error != NULL )
		g_warning("(%s): Unable to load gtk-file icon, err : %s",
			  __func__, error->message);

	/* Create the columns and cell renderers */

	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "pixbuf", F_PIXBUF,
					    NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "text", F_NAME,
					    NULL);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(file_tree), column);

	cwin->file_tree = file_tree;

	/* Setup row activation cb handling */

	g_signal_connect(G_OBJECT(file_tree), "row-activated",
			 G_CALLBACK(file_tree_row_activated_cb), cwin);

	/* Create right click popup menu */

	cwin->file_tree_dir_context_menu = create_file_tree_dir_context_menu(file_tree,
									     cwin);
	cwin->file_tree_file_context_menu = create_file_tree_file_context_menu(file_tree,
									       cwin);

	/* Signal handler for right-clicking */

	g_signal_connect(G_OBJECT(GTK_WIDGET(file_tree)), "button-release-event",
			 G_CALLBACK(file_tree_right_click_cb), cwin);

	g_object_unref(store);
	return file_tree;
}

/*****************/
/* Playlist Tree */
/*****************/

static GtkUIManager* create_playlist_tree_context_menu(GtkWidget *playlist_tree,
						       struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Playlist Tree Context Actions");
	context_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       playlist_tree_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create playlist tree context menu, err : %s",
			   __func__, error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     playlist_tree_context_aentries,
				     G_N_ELEMENTS(playlist_tree_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkWidget* create_playlist_tree(struct con_win *cwin)
{
	GtkWidget *playlist_tree;
	GtkTreeStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	/* Create the tree store */

	store = gtk_tree_store_new(N_PL_COLUMNS,
				   GDK_TYPE_PIXBUF, /* Pixbuf */
				   G_TYPE_STRING);  /* Playlist name */

	/* Create the tree view */

	playlist_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(playlist_tree), FALSE);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(playlist_tree), TRUE);

	/* Selection mode is multiple */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(playlist_tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create column and cell renderers */

	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "pixbuf", P_PIXBUF,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "text", P_PLAYLIST,
					    NULL);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(playlist_tree), column);

	cwin->playlist_tree = playlist_tree;

	g_signal_connect(G_OBJECT(playlist_tree), "row-activated",
			 G_CALLBACK(playlist_tree_row_activated_cb), cwin);

	/* Create right click popup menu */

	cwin->playlist_tree_context_menu = create_playlist_tree_context_menu(playlist_tree,
									     cwin);

	/* Signal handler for right-clicking */

	g_signal_connect(G_OBJECT(GTK_WIDGET(playlist_tree)), "button-press-event",
			 G_CALLBACK(playlist_tree_right_click_cb), cwin);

	g_object_unref(store);

	return playlist_tree;
}

/***************************/
/* Left pane (Browse mode) */
/***************************/

static GtkUIManager* create_library_page_context_menu(GtkWidget *library_page,
						      struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Library Page Context Actions");
	context_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       library_page_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create library page context menu, err : %s",
			   __func__,
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     library_page_context_aentries,
				     G_N_ELEMENTS(library_page_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkWidget* create_browse_mode_view(struct con_win *cwin)
{
	GtkWidget *browse_mode;
	GtkWidget *vbox_lib;
	GtkWidget *label_lib, *label_file;
	GtkWidget *file_tree, *library_tree, *playlist_tree;
	GtkWidget *file_tree_scroll, *library_tree_scroll;
	GtkWidget *sep;

	browse_mode = gtk_notebook_new();
	vbox_lib = gtk_vbox_new(FALSE, 0);
	sep = gtk_hseparator_new();

	/* The scrollbar window widgets */

	file_tree_scroll = gtk_scrolled_window_new(NULL, NULL);
	library_tree_scroll = gtk_scrolled_window_new(NULL, NULL);

	/* Set scrollbar policies */

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_tree_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_tree_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	/* Labels for the the notebook pages : Library / File / Playlist */

	label_lib = gtk_label_new("Library");
	label_file = gtk_label_new("File");

	/* Set angle to 90 degrees to display the labels vertically */

	gtk_label_set_angle(GTK_LABEL(label_lib), 90);
	gtk_label_set_angle(GTK_LABEL(label_file), 90);

	/* The actual notebook page widgets */

	file_tree = create_file_tree(cwin);
	library_tree = create_library_tree(cwin);
	playlist_tree = create_playlist_tree(cwin);

	/* Store playlist/library tree in hbox */

	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   playlist_tree,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   sep,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   library_tree,
			   TRUE,
			   TRUE,
			   0);

	/* Store them in the scrollbar widgets */

	gtk_container_add(GTK_CONTAINER(file_tree_scroll), file_tree);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(library_tree_scroll),
					      vbox_lib);

	/* Create library page context menu */

	cwin->library_page_context_menu = create_library_page_context_menu(label_lib,
									   cwin);

	/* Append the notebook page widgets */

	gtk_notebook_append_page(GTK_NOTEBOOK(browse_mode),
				 GTK_WIDGET(library_tree_scroll),
				 GTK_WIDGET(label_lib));
	gtk_notebook_append_page(GTK_NOTEBOOK(browse_mode),
				 GTK_WIDGET(file_tree_scroll),
				 GTK_WIDGET(label_file));

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(browse_mode), GTK_POS_LEFT);

	cwin->browse_mode = browse_mode;

	/* Setup focus-tab handling */

	switch_cb_id = g_signal_connect(G_OBJECT(browse_mode), "switch-page",
					G_CALLBACK(browse_mode_switch_page_cb), cwin);

	/* Signal handler for right-clicking on a page */

	g_signal_connect(G_OBJECT(GTK_NOTEBOOK(browse_mode)), "button-press-event",
			 G_CALLBACK(library_page_right_click_cb), cwin);

	gtk_notebook_popup_disable(GTK_NOTEBOOK(browse_mode));

	return browse_mode;
}

/*********************************/
/* Right pane (Current playlist) */
/*********************************/

static GtkWidget* create_header_context_menu(struct con_win *cwin)
{
	GtkWidget *menu;
	GtkWidget *toggle_track,
		*toggle_title,
		*toggle_artist,
		*toggle_album,
		*toggle_genre,
		*toggle_bitrate,
		*toggle_year,
		*toggle_length,
		*toggle_filename;

	menu = gtk_menu_new();

	/* Create the checkmenu items */

	toggle_track = gtk_check_menu_item_new_with_label(P_TRACK_NO_STR);
	toggle_title = gtk_check_menu_item_new_with_label(P_TITLE_STR);
	toggle_artist = gtk_check_menu_item_new_with_label(P_ARTIST_STR);
	toggle_album = gtk_check_menu_item_new_with_label(P_ALBUM_STR);
	toggle_genre = gtk_check_menu_item_new_with_label(P_GENRE_STR);
	toggle_bitrate = gtk_check_menu_item_new_with_label(P_BITRATE_STR);
	toggle_year = gtk_check_menu_item_new_with_label(P_YEAR_STR);
	toggle_length = gtk_check_menu_item_new_with_label(P_LENGTH_STR);
	toggle_filename = gtk_check_menu_item_new_with_label(P_FILENAME_STR);

	/* Add the items to the menu */

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_track);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_title);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_artist);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_album);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_genre);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_bitrate);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_year);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_length);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_filename);

	/* Initialize the state of the items */

	if (is_present_str_list(P_TRACK_NO_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_track), TRUE);
	if (is_present_str_list(P_TITLE_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_title), TRUE);
	if (is_present_str_list(P_ARTIST_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_artist), TRUE);
	if (is_present_str_list(P_ALBUM_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_album), TRUE);
	if (is_present_str_list(P_GENRE_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_genre), TRUE);
	if (is_present_str_list(P_BITRATE_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_bitrate), TRUE);
	if (is_present_str_list(P_YEAR_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_year), TRUE);
	if (is_present_str_list(P_LENGTH_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_length), TRUE);
	if (is_present_str_list(P_FILENAME_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_filename), TRUE);

	/* Setup the individual signal handlers */

	g_signal_connect(G_OBJECT(toggle_track), "toggled",
			 G_CALLBACK(playlist_track_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_title), "toggled",
			 G_CALLBACK(playlist_title_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_artist), "toggled",
			 G_CALLBACK(playlist_artist_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_album), "toggled",
			 G_CALLBACK(playlist_album_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_genre), "toggled",
			 G_CALLBACK(playlist_genre_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_bitrate), "toggled",
			 G_CALLBACK(playlist_bitrate_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_year), "toggled",
			 G_CALLBACK(playlist_year_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_length), "toggled",
			 G_CALLBACK(playlist_length_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_filename), "toggled",
			 G_CALLBACK(playlist_filename_column_change_cb), cwin);

	gtk_widget_show_all(menu);

	return menu;
}

static GtkUIManager* create_cp_context_menu(GtkWidget *current_playlist,
					    struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("CP Context Actions");
	context_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       cp_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create current playlist context menu, err : %s",
			   __func__,
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     cp_context_aentries,
				     G_N_ELEMENTS(cp_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static void create_current_playlist_columns(GtkWidget *current_playlist,
					    struct con_win *cwin)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *label_track,
		*label_title,
		*label_artist,
		*label_album,
		*label_genre,
		*label_bitrate,
		*label_year,
		*label_length,
		*label_filename;
	GtkWidget *col_button;

	label_track = gtk_label_new(P_TRACK_NO_STR);
	label_title = gtk_label_new(P_TITLE_STR);
	label_artist = gtk_label_new(P_ARTIST_STR);
	label_album = gtk_label_new(P_ALBUM_STR);
	label_genre = gtk_label_new(P_GENRE_STR);
	label_bitrate = gtk_label_new(P_BITRATE_STR);
	label_year = gtk_label_new(P_YEAR_STR);
	label_length = gtk_label_new(P_LENGTH_STR);
	label_filename = gtk_label_new(P_FILENAME_STR);

	/* Column : Track No */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_TRACK_NO_STR,
							  renderer,
							  "text",
							  P_TRACK_NO,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TRACK_NO);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_track);
	gtk_widget_show(label_track);
	col_button = gtk_widget_get_ancestor(label_track, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Title */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_TITLE_STR,
							  renderer,
							  "text",
							  P_TITLE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TITLE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_title);
	gtk_widget_show(label_title);
	col_button = gtk_widget_get_ancestor(label_title, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Artist */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_ARTIST_STR,
							  renderer,
							  "text",
							  P_ARTIST,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ARTIST);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_artist);
	gtk_widget_show(label_artist);
	col_button = gtk_widget_get_ancestor(label_artist, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Album */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_ALBUM_STR,
							  renderer,
							  "text",
							  P_ALBUM,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ALBUM);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_album);
	gtk_widget_show(label_album);
	col_button = gtk_widget_get_ancestor(label_album, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Genre */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_GENRE_STR,
							  renderer,
							  "text",
							  P_GENRE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_GENRE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_genre);
	gtk_widget_show(label_genre);
	col_button = gtk_widget_get_ancestor(label_genre, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Bitrate */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_BITRATE_STR,
							  renderer,
							  "text",
							  P_BITRATE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_BITRATE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_bitrate);
	gtk_widget_show(label_bitrate);
	col_button = gtk_widget_get_ancestor(label_bitrate, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Year */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_YEAR_STR,
							  renderer,
							  "text",
							  P_YEAR,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_YEAR);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_year);
	gtk_widget_show(label_year);
	col_button = gtk_widget_get_ancestor(label_year, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Length */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_LENGTH_STR,
							  renderer,
							  "text",
							  P_LENGTH,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_LENGTH);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_length);
	gtk_widget_show(label_length);
	col_button = gtk_widget_get_ancestor(label_length, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Filename */

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(P_FILENAME_STR,
							  renderer,
							  "text",
							  P_FILENAME,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_FILENAME);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_filename);
	gtk_widget_show(label_filename);
	col_button = gtk_widget_get_ancestor(label_filename, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);
}

static GtkWidget* create_current_playlist_view(struct con_win *cwin)
{
	GtkWidget *current_playlist_scroll;
	GtkWidget *current_playlist;
	GtkUIManager *cp_context_menu;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeSortable *sortable;

	/* The scrollbar widget */

	current_playlist_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(current_playlist_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);

	/* Create the tree store */

	store = gtk_list_store_new(N_P_COLUMNS,
				   G_TYPE_POINTER,	/* Pointer to musicobject */
				   G_TYPE_STRING,	/* Tag : Track No */
				   G_TYPE_STRING,	/* Tag : Title */
				   G_TYPE_STRING,	/* Tag : Artist */
				   G_TYPE_STRING,	/* Tag : Album */
				   G_TYPE_STRING,	/* Tag : Genre */
				   G_TYPE_STRING,	/* Tag : Bitrate */
				   G_TYPE_STRING,	/* Tag : Year */
				   G_TYPE_STRING,	/* Tag : Length */
				   G_TYPE_STRING,	/* Filename */
				   G_TYPE_BOOLEAN);	/* Played flag */

	/* Create the tree view */

	current_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(current_playlist));
	sortable = GTK_TREE_SORTABLE(model);

	/* Set the search function for interactive search */

	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(current_playlist),
					    current_playlist_search_compare,
					    cwin,
					    NULL);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(current_playlist), FALSE);

	/* Set the sort functions */

	gtk_tree_sortable_set_sort_func(sortable,
					P_TRACK_NO,
					compare_track_no,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_BITRATE,
					compare_bitrate,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_YEAR,
					compare_year,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_LENGTH,
					compare_length,
					NULL,
					NULL);

	/* Set selection properties */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(current_playlist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create the columns and cell renderers */

	create_current_playlist_columns(current_playlist, cwin);

	/* Signal handler for double-clicking on a row */

	g_signal_connect(G_OBJECT(current_playlist), "row-activated",
			 G_CALLBACK(current_playlist_row_activated_cb), cwin);

	/* Create contextual menus */

	cp_context_menu = create_cp_context_menu(current_playlist, cwin);
	cwin->cp_context_menu = cp_context_menu;
	cwin->header_context_menu = create_header_context_menu(cwin);

	/* Signal handler for right-clicking */

	g_signal_connect(G_OBJECT(GTK_WIDGET(current_playlist)), "button-press-event",
			 G_CALLBACK(current_playlist_right_click_cb), cwin);

	/* Store the treeview in the scrollbar widget */

	gtk_container_add(GTK_CONTAINER(current_playlist_scroll), current_playlist);
	cwin->current_playlist = current_playlist;

	/* Set initial column visibility */

	init_current_playlist_columns(cwin);

	g_object_unref(store);

	return current_playlist_scroll;
}

static void init_dnd(struct con_win *cwin)
{
	/* Source: Library View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->library_tree),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(cwin->library_tree),
			 "drag-data-get",
			 G_CALLBACK(dnd_library_tree_get),
			 cwin);

	/* Source: File View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->file_tree),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(cwin->file_tree),
			 "drag-data-get",
			 G_CALLBACK(dnd_file_tree_get),
			 cwin);

	/* Source: Playlist View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->playlist_tree),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(cwin->playlist_tree),
			 "drag-data-get",
			 G_CALLBACK(dnd_playlist_tree_get),
			 cwin);

	/* Source/Dest: Current Playlist */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->current_playlist),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_MOVE);

	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(cwin->current_playlist),
					     tentries,
					     G_N_ELEMENTS(tentries),
					     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect(G_OBJECT(cwin->current_playlist),
			 "drag-drop",
			 G_CALLBACK(dnd_current_playlist_drop),
			 cwin);
	g_signal_connect(G_OBJECT(cwin->current_playlist),
			 "drag-data-received",
			 G_CALLBACK(dnd_current_playlist_received),
			 cwin);
}

static GtkUIManager* create_systray_menu(struct con_win *cwin)
{
	GtkUIManager *menu = NULL;
	GtkActionGroup *actions;
	GError *error = NULL;

	actions = gtk_action_group_new("Systray Actions");
	menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(menu, systray_menu_xml, -1, &error)) {
		g_critical("(%s): Unable to create systray menu, err : %s",
			   __func__, error->message);
	}

	gtk_action_group_add_actions(actions,
				     systray_menu_aentries,
				     G_N_ELEMENTS(systray_menu_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(menu));
	gtk_ui_manager_insert_action_group(menu, actions, 0);

	return menu;
}

/********************************/
/* Externally visible functions */
/********************************/

GtkUIManager* create_menu(struct con_win *cwin)
{
	GtkUIManager *main_menu = NULL;
	GtkActionGroup *main_actions;
	GError *error = NULL;

	main_actions = gtk_action_group_new("Main Actions");
	main_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(main_menu, main_menu_xml, -1, &error)) {
		g_critical("(%s): Unable to create main menu, err : %s",
			   __func__, error->message);
	}

	gtk_action_group_add_actions(main_actions,
				     main_aentries,
				     G_N_ELEMENTS(main_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(main_menu));
	gtk_ui_manager_insert_action_group(main_menu, main_actions, 0);

	return main_menu;
}

GtkWidget* create_main_region(struct con_win *cwin)
{
	GtkWidget *hpane;
	GtkWidget *browse_mode;
	GtkWidget *current_playlist;

	/* A two paned container */

	hpane = gtk_hpaned_new();

	/* Left pane contains a notebook widget holding the various views */

	browse_mode = create_browse_mode_view(cwin);

	/* Right pane contains the current playlist */

	current_playlist = create_current_playlist_view(cwin);

	/* DnD */

	init_dnd(cwin);

	/* Set initial sizes */

	gtk_widget_set_size_request(browse_mode, BROWSE_MODE_SIZE, -1);

	/* Pack everything into the hpane */

	gtk_paned_pack1 (GTK_PANED (hpane), browse_mode, FALSE, FALSE);
	gtk_paned_pack2 (GTK_PANED (hpane), current_playlist, FALSE, FALSE);

	return hpane;
}

GtkWidget* create_panel(struct con_win *cwin)
{
	GtkWidget *controls_align, *shuffle_align, *repeat_align;
	GtkWidget *hbox_panel, *hbox_controls;
	GtkWidget *play_button, *stop_button, *prev_button, *next_button, *vol_button;
	GtkWidget *album_art_frame = NULL;
	GtkWidget *track_progress_bar, *track_progress_align;
	GtkWidget *vbox_order;
	GtkWidget *shuffle_button, *repeat_button;
	GtkObject *vol_adjust;

	hbox_panel = gtk_hbox_new(FALSE, 5);
	vbox_order = gtk_vbox_new(FALSE, 1);
	hbox_controls = gtk_hbox_new(FALSE, 1);

	/* Setup album art */

	if (cwin->cpref->show_album_art) {
		album_art_frame = gtk_frame_new(NULL);
		gtk_box_pack_start(GTK_BOX(hbox_panel),
				   GTK_WIDGET(album_art_frame),
				   FALSE, FALSE, 2);
	}

	/* Setup track progress */

	track_progress_align = gtk_alignment_new(0, 0.5, 1, 0.3);
	track_progress_bar = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(track_progress_bar),
				  TRACK_PROGRESS_BAR_STOPPED);
	gtk_container_add(GTK_CONTAINER(track_progress_align), track_progress_bar);

	gtk_widget_set_events(track_progress_bar, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(G_OBJECT(track_progress_bar), "button-press-event",
			 G_CALLBACK(track_progress_change_cb), cwin);

	/* Setup play order buttons */

	shuffle_align = gtk_alignment_new(0, 1, 0, 0);
	repeat_align = gtk_alignment_new(0, 0, 0, 0);
	shuffle_button = gtk_check_button_new_with_label("Shuffle");
	repeat_button = gtk_check_button_new_with_label("Repeat");

	gtk_container_add(GTK_CONTAINER(shuffle_align), shuffle_button);
	gtk_container_add(GTK_CONTAINER(repeat_align), repeat_button);

	gtk_box_pack_start(GTK_BOX(vbox_order),
			   GTK_WIDGET(shuffle_align),
			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_order),
			   GTK_WIDGET(repeat_align),
			   TRUE, TRUE, 0);

	/* Signal handlers for play order controls */

	g_signal_connect(G_OBJECT(shuffle_button), "toggled",
			 G_CALLBACK(shuffle_button_handler), cwin);
	g_signal_connect(G_OBJECT(repeat_button), "toggled",
			 G_CALLBACK(repeat_button_handler), cwin);

	/* Images for pause and play */

	cwin->pixbuf->image_pause =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);
	cwin->pixbuf->image_play =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);

	g_object_ref(cwin->pixbuf->image_play);
	g_object_ref(cwin->pixbuf->image_pause);

	/* Setup control buttons */

	prev_button = gtk_button_new();
	play_button = gtk_button_new();
	stop_button = gtk_button_new();
	next_button = gtk_button_new();
	vol_button = gtk_volume_button_new();

	vol_adjust = gtk_adjustment_new(0, 0, 100, 1, 5, 0);
	gtk_scale_button_set_adjustment(GTK_SCALE_BUTTON(vol_button),
					GTK_ADJUSTMENT(vol_adjust));
	gtk_button_set_relief(GTK_BUTTON(vol_button), GTK_RELIEF_NORMAL);

	gtk_button_set_image(GTK_BUTTON(prev_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(stop_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(next_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(play_button),
			     cwin->pixbuf->image_play);

	gtk_box_pack_start(GTK_BOX(hbox_controls),
			   GTK_WIDGET(prev_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_controls),
			   GTK_WIDGET(play_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_controls),
			   GTK_WIDGET(stop_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_controls),
			   GTK_WIDGET(next_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_controls),
			   GTK_WIDGET(vol_button),
			   TRUE, FALSE, 0);

	controls_align = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(controls_align), hbox_controls);

	/* Signal handlers */

	g_signal_connect(G_OBJECT(prev_button), "clicked",
			 G_CALLBACK(prev_button_handler), cwin);
	g_signal_connect(G_OBJECT(play_button), "clicked",
			 G_CALLBACK(play_button_handler), cwin);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
			 G_CALLBACK(stop_button_handler), cwin);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			 G_CALLBACK(next_button_handler), cwin);
	g_signal_connect(G_OBJECT(vol_button), "value-changed",
			 G_CALLBACK(vol_button_handler), cwin);

	/* Initial state of various widgets from stored preferences */

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shuffle_button),
				     cwin->cpref->shuffle);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_button),
				     cwin->cpref->repeat);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_button),
				   SCALE_UP_VOL(cwin->cmixer->curr_vol));

	/* References to widgets */

	cwin->hbox_panel = hbox_panel;
	cwin->album_art_frame = album_art_frame;
	cwin->track_progress_bar = track_progress_bar;
	cwin->shuffle_button = shuffle_button;
	cwin->repeat_button = repeat_button;
	cwin->prev_button = prev_button;
	cwin->play_button = play_button;
	cwin->stop_button = stop_button;
	cwin->next_button = next_button;
	cwin->vol_button = vol_button;

	/* Tooltips */

	gtk_widget_set_tooltip_text(GTK_WIDGET(play_button), "Play / Pause Track");
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_button), "Previous Track");
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_button), "Next Track");
	gtk_widget_set_tooltip_text(GTK_WIDGET(stop_button), "Stop playback");

	/* Pack panel widgets into hbox_panel */

	gtk_box_pack_start(GTK_BOX(hbox_panel),
			   GTK_WIDGET(track_progress_align),
			   TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(hbox_panel),
			 GTK_WIDGET(vbox_order),
			 FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hbox_panel),
			 GTK_WIDGET(controls_align),
			 FALSE, FALSE, 0);

	return hbox_panel;
}

GtkWidget* create_status_bar(struct con_win *cwin)
{
	GtkWidget *status_bar;

	status_bar = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(status_bar), 0.99, 0);
	cwin->status_bar = status_bar;

	return status_bar;
}

/* Search (simple) */

GtkWidget* create_search_bar(struct con_win *cwin)
{
	GtkWidget *hbox_bar;
	GtkWidget *search_entry;
	GtkWidget *label_find;
	GtkWidget *cancel_button;

	hbox_bar = gtk_hbox_new(FALSE, 0);
	label_find = gtk_label_new("Search Library");
	search_entry = gtk_entry_new();
	cancel_button = gtk_button_new();

	gtk_button_set_image(GTK_BUTTON(cancel_button),
			     gtk_image_new_from_stock(GTK_STOCK_CANCEL,
						      GTK_ICON_SIZE_MENU));

	gtk_box_pack_start(GTK_BOX(hbox_bar),
			   cancel_button,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(hbox_bar),
			   label_find,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(hbox_bar),
			   search_entry,
			   TRUE,
			   TRUE,
			   2);

	cwin->search_bar = hbox_bar;
	cwin->search_entry = search_entry;

	/* Signal handlers */

	g_signal_connect(G_OBJECT(cancel_button),
			 "clicked",
			 G_CALLBACK(cancel_simple_library_search_handler),
			 cwin);
	g_signal_connect(G_OBJECT(search_entry),
			 "key-release-event",
			 G_CALLBACK(simple_library_search_keyrelease_handler),
			 cwin);

	return hbox_bar;
}

/* Systray */

void create_status_icon(struct con_win *cwin)
{
	GtkStatusIcon *status_icon;
	GtkUIManager *systray_menu;

	if (cwin->pixbuf->pixbuf_app)
		status_icon = gtk_status_icon_new_from_pixbuf(cwin->pixbuf->pixbuf_app);
	else
		status_icon = gtk_status_icon_new_from_stock(GTK_STOCK_NEW);

	gtk_status_icon_set_tooltip(GTK_STATUS_ICON(status_icon), PACKAGE_STRING);

	g_signal_connect(G_OBJECT(status_icon), "activate",
			 G_CALLBACK(status_icon_activate), cwin);
	g_signal_connect(G_OBJECT(status_icon), "popup-menu",
			 G_CALLBACK(status_icon_popup_menu), cwin);

	/* Systray right click menu */

	systray_menu = create_systray_menu(cwin);

	/* Store reference */

	cwin->status_icon = status_icon;
	cwin->systray_menu = systray_menu;
}

gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin)
{
	if(gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon))) {
		gtk_widget_hide(GTK_WIDGET(cwin->mainwindow));
		return TRUE;
	}

	return FALSE;
}
