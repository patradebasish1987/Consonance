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

static void add_entry_playlist(gchar *playlist,
			       GtkTreeIter *root,
			       GtkTreeModel *model,
			       struct con_win *cwin)
{
	GtkTreeIter iter;

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      root);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   P_PIXBUF, cwin->pixbuf->pixbuf_file,
			   P_PLAYLIST, playlist,
			   -1);
}

/* Add all the tracks under the given path to the current playlist */
/* NB: Optimization */

static void add_row_current_playlist(GtkTreePath *path, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *playlist;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));
	if (gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_model_get(model, &iter, P_PLAYLIST, &playlist, -1);
		add_playlist_current_playlist(playlist, cwin);
		g_free(playlist);
	}
}

static gboolean overwrite_existing_playlist(const gchar *playlist, struct con_win *cwin)
{
	gboolean choice = FALSE;
	gint ret;
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"Do you want to overwrite the playlist: %s ?",
					playlist);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));

	switch(ret) {
	case GTK_RESPONSE_YES:
		choice = TRUE;
		break;
	case GTK_RESPONSE_NO:
		choice = FALSE;
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);
	return choice;
}

/**********************/
/* External functions */
/**********************/

/* Append the given playlist to the current playlist */

void add_playlist_current_playlist(gchar *playlist, struct con_win *cwin)
{
	gchar *query, *file;
	gint playlist_id, location_id, i = 0;
	struct db_result result;
	struct musicobject *mobj;

	playlist_id = find_playlist_db(playlist, cwin);
	query = g_strdup_printf("SELECT FILE FROM PLAYLIST_TRACKS WHERE PLAYLIST=%d",
				playlist_id);
	exec_sqlite_query(query, cwin, &result);
	for_each_result_row(result, i) {
		file = sanitize_string_sqlite3(result.resultp[i]);

		if ((location_id = find_location_db(file, cwin)))
			mobj = new_musicobject_from_db(location_id, cwin);
		else
			mobj = new_musicobject_from_file(result.resultp[i]);

		append_current_playlist(mobj, cwin);
		g_free(file);
	}
	sqlite3_free_table(result.resultp);
}

void playlist_tree_row_activated_cb(GtkTreeView *playlist_tree,
				    GtkTreePath *path,
				    GtkTreeViewColumn *column,
				    struct con_win *cwin)
{
	GtkTreeIter r_iter;
	GtkTreePath *r_path;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));

	gtk_tree_model_get_iter_first(model, &r_iter);
	r_path = gtk_tree_model_get_path(model, &r_iter);

	if (!gtk_tree_path_compare(path, r_path)) {
		if (!gtk_tree_view_row_expanded(GTK_TREE_VIEW(cwin->playlist_tree),
						path))
			gtk_tree_view_expand_row(GTK_TREE_VIEW(cwin->playlist_tree),
						 path, FALSE);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(cwin->playlist_tree),
						   path);
	}
	else {
		add_row_current_playlist(path, cwin);
	}

	gtk_tree_path_free(r_path);
}

gboolean playlist_tree_right_click_cb(GtkWidget *widget,
				      GdkEventButton *event,
				      struct con_win *cwin)
{
	GtkWidget *popup_menu;
	GtkTreeSelection *selection;
	gboolean ret = FALSE;

	switch(event->button) {
	case 3:
		popup_menu = gtk_ui_manager_get_widget(cwin->playlist_tree_context_menu,
						       "/popup");
		gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
			       event->button, event->time);

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->playlist_tree));

		/* If more than one track is selected, don't propagate event */

		if (gtk_tree_selection_count_selected_rows(selection) > 1)
			ret = TRUE;
		else
			ret = FALSE;
		break;
	default:
		ret = FALSE;
		break;
	}

	return ret;
}

void playlist_tree_play(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->playlist_tree));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	/* Clear current playlist first */

	clear_current_playlist(NULL, cwin);

	if (list) {

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			add_row_current_playlist(path, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */

			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE)) {
					g_list_free(list);
					return;
				}
			}
		}
		
		g_list_free(list);
	}

	/* Play the first track */

	play_first_current_playlist(cwin);
}

void playlist_tree_enqueue(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->playlist_tree));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	if (list) {

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			add_row_current_playlist(path, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */

			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE)) {
					g_list_free(list);
					return;
				}
			}
		}
		g_list_free(list);
	}
}

void playlist_tree_delete(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	gchar *playlist;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->playlist_tree));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	if (list) {

		/* Delete selected playlists */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get(model, &iter, P_PLAYLIST, &playlist, -1);
			delete_playlist_db(playlist, cwin);
			gtk_tree_path_free(path);
			g_free(playlist);
		}
		g_list_free(list);
	}
	init_playlist_view(cwin);
}

/* Callback for DnD signal 'drag-data-get' */

void dnd_playlist_tree_get(GtkWidget *widget,
			   GdkDragContext *context,
			   GtkSelectionData *data,
			   guint info,
			   guint time,
			   struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter, r_iter;
	GtkTreePath *r_path;
	GList *list = NULL, *l;
	GArray *playlist_arr;
	gchar *playlist;

	switch(info) {
	case TARGET_PLAYLIST:
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->playlist_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		/* No selections */

		if (!list) {
			gtk_selection_data_set(data, data->type, 8, NULL, 0);
			break;
		}

		gtk_tree_model_get_iter_first(model, &r_iter);
		r_path = gtk_tree_model_get_path(model, &r_iter);

		/* Form an array of playlist names */

		playlist_arr = g_array_new(TRUE, TRUE, sizeof(gchar *));
		l = list;

		while(l) {
			if (gtk_tree_path_compare(r_path, l->data)) {
				gtk_tree_model_get_iter(model, &iter, l->data);
				gtk_tree_model_get(model, &iter, P_PLAYLIST, &playlist, -1);
				g_array_append_val(playlist_arr, playlist);
			}
			gtk_tree_path_free(l->data);
			l = l->next;
		}

		gtk_selection_data_set(data,
				       data->type,
				       8,
				       (guchar *)&playlist_arr,
				       sizeof(GArray *));

		CDEBUG("(%s): Fill DnD data, selection: %p, playlist_arr: %p",
		       __func__, data->data, playlist_arr);

		/* Cleanup */

		gtk_tree_path_free(r_path);
		g_list_free(list);

		break;
	default:
		g_warning("(%s): Unknown DND type", __func__);
	}
}

/* Save tracks to a playlist using the given type */

void save_playlist(gint playlist_id, gint type, struct con_win *cwin)
{
	gchar *file;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	GList *list, *i;
	gchar *query;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	switch(type) {
	case SAVE_COMPLETE:
		query = g_strdup_printf("BEGIN;");
		exec_sqlite_query(query, cwin, NULL);
		gtk_tree_model_get_iter_first(model, &iter);
		do {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			if (mobj) {
				file = sanitize_string_sqlite3(mobj->file);
				add_track_playlist_db(file, playlist_id, cwin);
				g_free(file);
			}
		} while(gtk_tree_model_iter_next(model, &iter));
		query = g_strdup_printf("END;");
		exec_sqlite_query(query, cwin, NULL);
		break;
	case SAVE_SELECTED:
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							cwin->current_playlist));
		list = gtk_tree_selection_get_selected_rows(selection, NULL);

		if (list) {

			/* Add all the rows to the playlist */

			query = g_strdup_printf("BEGIN;");
			exec_sqlite_query(query, cwin, NULL);

			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter,
						   P_MOBJ_PTR, &mobj, -1);

				if (mobj) {
					file = sanitize_string_sqlite3(mobj->file);
					add_track_playlist_db(file, playlist_id,
							      cwin);
					g_free(file);
				}

				gtk_tree_path_free(path);
			}
			g_list_free(list);

			query = g_strdup_printf("END;");
			exec_sqlite_query(query, cwin, NULL);
		}
		break;
	default:
		break;
	}
}

void new_playlist(const gchar *playlist, gint type, struct con_win *cwin)
{
	gint playlist_id = 0;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (!playlist || !strlen(playlist)) {
		g_warning("(%s): Playlist name is NULL", __func__);
		return;
	}

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("(%s): Current playlist is empty", __func__);
		return;
	}

	if ((playlist_id = find_playlist_db(playlist, cwin))) {
		if (overwrite_existing_playlist(playlist, cwin))
			delete_playlist_db((gchar *)playlist, cwin);
		else
			return;
	}

	playlist_id = add_new_playlist_db(playlist, cwin);
	save_playlist(playlist_id, type, cwin);
}

void append_playlist(const gchar *playlist, gint type, struct con_win *cwin)
{
	gint playlist_id;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (!playlist || !strlen(playlist)) {
		g_warning("(%s): Playlist name is NULL", __func__);
		return;
	}

	playlist_id = find_playlist_db(playlist, cwin);

	if (!playlist_id) {
		g_warning("(%s): Playlist doesn't exist\n", __func__);
		return;
	}

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("(%s): Current playlist is empty", __func__);
		return;
	}

	save_playlist(playlist_id, type, cwin);
}

void init_playlist_view(struct con_win *cwin)
{
	gint i = 0;
	gchar *query;
	struct db_result result;
	GtkTreeIter iter, r_iter;
	GtkTreePath *r_path = NULL;
	GtkTreeModel *model;
	gboolean expanded = FALSE;

	cwin->cstate->view_change = TRUE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->playlist_tree));
	if (gtk_tree_model_get_iter_first(model, &r_iter)) {
		r_path = gtk_tree_model_get_path(model, &r_iter);
		expanded = gtk_tree_view_row_expanded(GTK_TREE_VIEW(cwin->playlist_tree),
						      r_path);
	}
	gtk_tree_store_clear(GTK_TREE_STORE(model));

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   P_PIXBUF, cwin->pixbuf->pixbuf_dir,
			   P_PLAYLIST, "Playlists",
			   -1);

	/* Query and insert entries */

	query = g_strdup_printf("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\";",
				SAVE_PLAYLIST_STATE);

	exec_sqlite_query(query, cwin, &result);

	for_each_result_row(result, i) {
		add_entry_playlist(result.resultp[i],
				   &iter, model, cwin);

		/* Have to give control to GTK periodically ... */
		/* If gtk_main_quit has been called, return -
		   since main loop is no more. */

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE)) {
				if (r_path)
					gtk_tree_path_free(r_path);
				sqlite3_free_table(result.resultp);
				return;
			}
		}
	}

	if (r_path) {
		if (expanded)
			gtk_tree_view_expand_row(GTK_TREE_VIEW(cwin->playlist_tree),
						 r_path, FALSE);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(cwin->playlist_tree),
						   r_path);

		gtk_tree_path_free(r_path);
	}
	sqlite3_free_table(result.resultp);
	cwin->cstate->view_change = FALSE;
}
