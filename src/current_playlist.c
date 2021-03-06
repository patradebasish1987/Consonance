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

#include <string.h>
#include "consonance.h"

/*********************/
/* General functions */
/*********************/

static gint get_total_playtime(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint total_playtime = 0;
	struct musicobject *mobj = NULL;
	gboolean ret;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj)
			total_playtime += mobj->tags->length;
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return total_playtime;
}

/* Update status bar */

static void update_status_bar(struct con_win *cwin)
{
	gint total_playtime = 0;
	gchar *str, *tot_str;

	total_playtime = get_total_playtime(cwin);
	tot_str = convert_length_str(total_playtime);
	str = g_strdup_printf("Playtime : %s", tot_str);

	CDEBUG("(%s): Updating status bar with new playtime: %s", __func__, tot_str);

	gtk_label_set_text(GTK_LABEL(cwin->status_bar), str);

	g_free(tot_str);
	g_free(str);
}

/* Clear current seq ref */

static void clear_curr_seq_ref(struct con_win *cwin)
{
	if (!cwin->cstate->curr_seq_ref)
		return;

	gtk_tree_row_reference_free(cwin->cstate->curr_seq_ref);
	cwin->cstate->curr_seq_ref = NULL;
}

/* Clear cstate->curr_seq_ref if it happens to contain the given path */

static void test_clear_curr_seq_ref(GtkTreePath *path, struct con_win *cwin)
{
	GtkTreePath *lpath;

	if (!cwin->cstate->curr_seq_ref)
		return;

	lpath = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);
	if (!gtk_tree_path_compare(path, lpath)) {
		gtk_tree_row_reference_free(cwin->cstate->curr_seq_ref);
		cwin->cstate->curr_seq_ref = NULL;
	}
	gtk_tree_path_free(lpath);
}

/* Check if given ref is the current rand reference */

static gboolean is_current_rand_ref(GtkTreeRowReference *ref, struct con_win *cwin)
{
	if (ref == cwin->cstate->curr_rand_ref)
		return TRUE;
	else
		return FALSE;
}

/* Print title of track ref */

static void print_track_ref(GtkTreeRowReference *ref, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	struct musicobject *mobj = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	path = gtk_tree_row_reference_get_path(ref);

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	if (mobj)
		g_print("Track title from ref : %s at %p\n", mobj->tags->title, ref);

	gtk_tree_path_free(path);
}

/* Print title of all nodes in cstate->rand_track_refs */

static void print_all_rand_track_refs(struct con_win *cwin)
{
	GList *list;

	if (cwin->cstate->rand_track_refs) {
		list = cwin->cstate->rand_track_refs;
		while (list) {
			print_track_ref(list->data, cwin);
			list = list->next;
		}
	}
}

/* Delete the ref corresponding to the given path */

void delete_rand_track_refs(GtkTreePath *path, struct con_win *cwin)
{
	GList *list;
	GtkTreeRowReference *ref;
	GtkTreePath *lpath;

	if (cwin->cstate->rand_track_refs) {
		list = cwin->cstate->rand_track_refs;
		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);
			if (!gtk_tree_path_compare(path, lpath)) {
				if (is_current_rand_ref(ref, cwin))
					cwin->cstate->curr_rand_ref = NULL;
				gtk_tree_row_reference_free(ref);
				cwin->cstate->rand_track_refs =
					g_list_remove(cwin->cstate->rand_track_refs,
						      ref);
				gtk_tree_path_free(lpath);
				break;
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
	}
}

/* Return the next node after the given ref */

static GtkTreeRowReference* get_rand_ref_next(GtkTreeRowReference *ref,
					      struct con_win *cwin)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!cwin->cstate->rand_track_refs)
		return NULL;

	list = cwin->cstate->rand_track_refs;
	while (list) {
		if (ref == list->data) {
			ret_ref = list->next->data;
			break;
		}
		list = list->next;
	}
	return ret_ref;
}

/* Return the prev node of the given ref */

static GtkTreeRowReference* get_rand_ref_prev(GtkTreeRowReference *ref,
					      struct con_win *cwin)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!cwin->cstate->rand_track_refs)
		return NULL;

	list = cwin->cstate->rand_track_refs;
	while (list) {
		if (ref == list->data) {
			ret_ref = list->prev->data;
			break;
		}
		list = list->next;
	}
	return ret_ref;
}

/* Return path of track at nth position in current playlist */

static GtkTreePath* current_playlist_nth_track(gint n, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint pos = 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	do {
		if (pos == n) {
			path = gtk_tree_model_get_path(model, &iter);
			break;
		}
		pos++;
	}
	while (gtk_tree_model_iter_next(model, &iter));

	return path;
}

/* Return path of next unique random track */

static GtkTreePath* get_next_unplayed_random_track(struct con_win *cwin)
{
	gint rnd;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gboolean played = TRUE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	while (played && cwin->cstate->unplayed_tracks) {
		rnd = g_rand_int_range(cwin->cstate->rand,
				       0,
				       cwin->cstate->tracks_curr_playlist);
		path = current_playlist_nth_track(rnd, cwin);
		if (!path) {
			g_printerr("No track at position : %d\n", rnd);
			return NULL;
		}

		if (gtk_tree_model_get_iter(model, &iter, path))
			gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);

		if (played) {
			gtk_tree_path_free(path);
			path = NULL;
		}
	}
	return path;
}

/* Return path of next random track,
   this is called after exhausting all unique tracks */

static GtkTreePath* get_next_random_track(struct con_win *cwin)
{
	gint rnd;
	GtkTreePath *path = NULL, *rpath;

	rpath = gtk_tree_row_reference_get_path(cwin->cstate->curr_rand_ref);
	do {
		rnd = g_rand_int_range(cwin->cstate->rand,
				       0,
				       cwin->cstate->tracks_curr_playlist);
		path = current_playlist_nth_track(rnd, cwin);
	} while (!gtk_tree_path_compare(rpath, path) &&
		 (cwin->cstate->tracks_curr_playlist > 1));

	gtk_tree_path_free(rpath);

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next sequential track */

static GtkTreePath* get_next_sequential_track(struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	gboolean ret = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	ret = gtk_tree_model_get_iter_first(model, &iter);

	/* If no tracks, return NULL.
	   If current track has been removed from the playlist,
	   return the first track. */

	if (!cwin->cstate->curr_seq_ref && !ret)
		return NULL;
	else if (!cwin->cstate->curr_seq_ref && ret) {
		path = gtk_tree_model_get_path(model, &iter);
		return path;
	}

	path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);
	gtk_tree_model_get_iter(model, &iter, path);

	if (!gtk_tree_model_iter_next(model, &iter)) {
		gtk_tree_path_free(path);
		path = NULL;
	}
	else {
		gtk_tree_path_free(path);
		path = gtk_tree_model_get_path(model, &iter);
	}

	return path;
}

/* Return path of next track in the list cstate->rand_track_refs */
/* This is called when the user clicks 'next' after one/more 'prev(s)' */

static GtkTreePath* get_next_random_ref_track(struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	i = g_list_find(cwin->cstate->rand_track_refs, cwin->cstate->curr_rand_ref);
	if (i) {
		j = g_list_next(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}
	return path;
}

/* Return path of the node previous to the current track from
   cstate->rand_track_refs */

static GtkTreePath* get_prev_random_track(struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	if (!cwin->cstate->rand_track_refs)
		return NULL;

	i = g_list_find(cwin->cstate->rand_track_refs, cwin->cstate->curr_rand_ref);
	if (i) {
		j = g_list_previous(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}

	return path;
}

/* Return path of the previous sequential track */

static GtkTreePath* get_prev_sequential_track(struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;

	if (!cwin->cstate->curr_seq_ref)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);
	gtk_tree_model_get_iter(model, &iter, path);

	if (!gtk_tree_path_prev(path)) {
		gtk_tree_path_free(path);
		path = NULL;
	}

	return path;
}

/* Add a new ref to cstate->rand_track_refs */

static void append_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin)
{
	cwin->cstate->rand_track_refs = g_list_append(cwin->cstate->rand_track_refs,
						      ref);
}

/* Remove all nodes and free the list */

static void clear_rand_track_refs(struct con_win *cwin)
{
	GList *list;

	if (cwin->cstate->rand_track_refs) {
		list = cwin->cstate->rand_track_refs;
		while (list) {
			gtk_tree_row_reference_free(list->data);
			list = list->next;
		}
		g_list_free(cwin->cstate->rand_track_refs);
		cwin->cstate->rand_track_refs = NULL;
	}

	cwin->cstate->curr_rand_ref = NULL;
}

/* Mark a track in current playlist as dirty */

static void current_playlist_set_dirty_track(GtkTreePath *path, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, TRUE, -1);
}

/* Comparison function for column names */

static gint compare_playlist_column_name(gconstpointer a, gconstpointer b)
{
	const gchar *e1 = a;
	const gchar *e2 = b;

	return g_ascii_strcasecmp(e1, e2);
}

/* Function to add/delete columns from preferences */

static void modify_current_playlist_columns(struct con_win *cwin,
					    const gchar *col_name,
					    gboolean state)
{
	gboolean pref_present;
	GSList *element;

	if (!col_name) {
		g_warning("(%s): Invalid column name", __func__);
		return;
	}

	pref_present = is_present_str_list(col_name, cwin->cpref->playlist_columns);

	/* Already in preferences */

	if (pref_present && state) {
		return;
	}

	/* Remove from preferences */

	else if (pref_present && !state) {
		element = g_slist_find_custom(cwin->cpref->playlist_columns,
				      col_name, compare_playlist_column_name);
		if (element) {
			g_free(element->data);
			cwin->cpref->playlist_columns =
				g_slist_delete_link(cwin->cpref->playlist_columns,
						    element);
		}
		else
			g_warning("(%s): Column : %s not found in preferences",
				  __func__, col_name);
	}

	/* Add to preferences */
 
	else if (!pref_present && state) { 
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(col_name));
	}
}

/* Build a dialog to get a new playlist name */

static gchar* get_playlist_dialog(gint *choice, struct con_win *cwin)
{
	gchar **playlists, *playlist = NULL;
	gint result, i=0;
	GtkWidget *dialog, *radio_new, *radio_add;
	GtkWidget *hbox, *vbox1, *vbox2;
	GtkWidget *entry;
	GtkWidget *combo_add;

	/* Retrieve list of playlist names from DB */

	playlists = get_playlist_names_db(cwin);

	/* Create dialog window */

	hbox = gtk_hbox_new(TRUE, 2);
	vbox1 = gtk_vbox_new(TRUE, 2);
	vbox2 = gtk_vbox_new(TRUE, 2);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
	combo_add = gtk_combo_box_new_text();
	radio_new = gtk_radio_button_new_with_label(NULL, "Save as a new playlist");
	radio_add = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(radio_new), "Append to an existing playlist");
	if (playlists) {
		while (playlists[i]) {
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo_add),
						  playlists[i]);
			i++;
		}
	}
	else {
		gtk_widget_set_sensitive(combo_add, FALSE);
		gtk_widget_set_sensitive(radio_add, FALSE);
	}
	dialog = gtk_dialog_new_with_buttons("Save playlist",
			     GTK_WINDOW(cwin->mainwindow),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     NULL);
	gtk_box_pack_start(GTK_BOX(vbox1), radio_new, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), radio_add, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox2), entry, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox2), combo_add, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 2);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:

		/* Get playlist name */
		/* Store a copy because the dialog box is destroyed on return */

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_new))) {
			playlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
			*choice = NEW_PLAYLIST;
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_add))) {
			playlist = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_add));
			*choice = APPEND_PLAYLIST;
		}

		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}

	/* Cleanup and exit */

	gtk_widget_destroy(dialog);
	if (playlists)
		g_strfreev(playlists);

	return playlist;
}

/* Get a new playlist name that is not reserved */

static gchar* get_playlist_name(struct con_win *cwin, gint *choice)
{
	gchar *playlist = NULL;
	gint sel = 0;

	do {
		playlist = get_playlist_dialog(&sel, cwin);
		if (playlist && !g_ascii_strcasecmp(playlist, SAVE_PLAYLIST_STATE)) {
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				"<b>con_playlist</b> is a reserved playlist name");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			g_free(playlist);
			continue;
		} else {
			break;
		}
	} while (1);

	*choice = sel;
	return playlist;
}

/*************************/
/* General playlist mgmt */
/*************************/

/* Update the state on 'Next', 'Prev' or selecting a new track
   from current playlist */
	
void update_current_state(GThread *thread, GtkTreePath *path,
			  gint action, struct con_win *cwin)
{
	GtkTreeRowReference *rand_ref;
	GtkTreeModel *model;
	GtkTreeSelection *selection;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));

	CDEBUG("(%s): Updating c_thread with new thread: %p", __func__, thread);

	cwin->cstate->c_thread = thread;

	/* Update view */

	gtk_tree_selection_unselect_all(selection);
	gtk_tree_selection_select_path(selection, path);
	if ((action != PLAYLIST_CURR) && cwin->cpref->shuffle)
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cwin->current_playlist),
					     path, NULL, TRUE, 0.5, 0);
	else
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cwin->current_playlist),
					     path, NULL, FALSE, 0, 0);

	/* Update status icon tooltip */

	status_icon_tooltip_update(cwin);

	/* Update track progress bar */

	__update_current_song_info(cwin, 0);

	/* Update album art */

	update_album_art(cwin->cstate->curr_mobj, cwin);

	/* Show OSD */

	show_osd(cwin);

	/* Store reference */

	if (!cwin->cpref->shuffle) {
		gtk_tree_row_reference_free(cwin->cstate->curr_seq_ref);
		cwin->cstate->curr_seq_ref = gtk_tree_row_reference_new(model, path);
	}

	/* Append the new reference to the list of played track references
	   to retrace the sequence */

	if (cwin->cpref->shuffle) {
		switch (action) {

			/* If 'Prev', get the previous node from the track references */

		case PLAYLIST_PREV:
			if (cwin->cstate->curr_rand_ref) {
				cwin->cstate->curr_rand_ref =
					get_rand_ref_prev(cwin->cstate->curr_rand_ref,
							  cwin);
			}
			break;

			/* If 'Next', get the next node from the track references */
			/* Do this only if the current track and the
			   last node don't match */

		case PLAYLIST_NEXT:
			if (cwin->cstate->curr_rand_ref) {
				if (cwin->cstate->curr_rand_ref !=
				    (g_list_last(cwin->cstate->rand_track_refs)->data)) {
					cwin->cstate->curr_rand_ref =
						get_rand_ref_next(cwin->cstate->curr_rand_ref,
								  cwin);
					break;
				}
			}

			/* Append a new ref of the track to the track references */

		case PLAYLIST_CURR:
			rand_ref = gtk_tree_row_reference_new(model, path);
			cwin->cstate->rand_track_refs =
				g_list_append(cwin->cstate->rand_track_refs,
					      rand_ref);
			cwin->cstate->curr_rand_ref = rand_ref;
			break;
		default:
			break;
		}
	}

	/* Mark the track as dirty */

	current_playlist_set_dirty_track(path, cwin);
	if (cwin->cstate->unplayed_tracks)
		cwin->cstate->unplayed_tracks--;
}

/* Return musicobject of the given path */

struct musicobject* current_playlist_mobj_at_path(GtkTreePath *path,
						  struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	return mobj;
}

/* Reset random rand_refs and appends given ref */

void reset_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin)
{
	GtkTreePath *path;

	current_playlist_clear_dirty_all(cwin);
	clear_rand_track_refs(cwin);
	append_rand_track_refs(ref, cwin);
	cwin->cstate->curr_rand_ref = ref;
	path = gtk_tree_row_reference_get_path(ref);
	current_playlist_set_dirty_track(path, cwin);
	gtk_tree_path_free(path);
}

/* Mark all tracks in current playlist as clean */

void current_playlist_clear_dirty_all(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean ret;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	ret = gtk_tree_model_get_iter_first(model, &iter);
	if (!ret)
		return;

	while(ret) {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, FALSE, -1);
		ret = gtk_tree_model_iter_next(model, &iter);
	}
}

/* Return the path of the selected track */

GtkTreePath* current_playlist_get_selection(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path = NULL;
	gint cnt_selected = 0;
	GList *list;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	cnt_selected = gtk_tree_selection_count_selected_rows(selection);

	if (!cnt_selected)
		return NULL;

	else if (cnt_selected == 1) {
		list = gtk_tree_selection_get_selected_rows(selection, NULL);
		if (list) {
			path = list->data;
			g_list_free(list);
		}
	}
	else if (cnt_selected > 1)
		g_message("(%s): Selected multiple", __func__);

	return path;
}

/* Return the path of the next track to be played */

GtkTreePath* current_playlist_get_next(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GList *last;
	GtkTreeIter iter;
	gboolean rand_unplayed = FALSE, seq_last = FALSE;

	/* Are we playing right now ? */

	if (cwin->cstate->state == ST_STOPPED)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	/* Check if tree is empty */

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	switch (cwin->cpref->shuffle) {
	case TRUE:
		last = g_list_last(cwin->cstate->rand_track_refs);
		if ((!cwin->cstate->curr_rand_ref) ||
		    (last && (cwin->cstate->curr_rand_ref == last->data))) {
			path = get_next_unplayed_random_track(cwin);
			if (!path) rand_unplayed = TRUE;
		}
		else {
			path = get_next_random_ref_track(cwin);
		}
		break;
	case FALSE:
		path = get_next_sequential_track(cwin);
		if (!path) seq_last = TRUE;
		break;
	default:
		break;
	}

	if (rand_unplayed && cwin->cpref->repeat)
		path = get_next_random_track(cwin);

	if (seq_last && cwin->cpref->repeat)
		path = current_playlist_nth_track(0, cwin);

	return path;
}

/* Return the path of the next(prev) track to be played */

GtkTreePath* current_playlist_get_prev(struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	gboolean seq_first = FALSE;

	switch (cwin->cpref->shuffle) {
	case TRUE:
		path = get_prev_random_track(cwin);
		break;
	case FALSE:
		path = get_prev_sequential_track(cwin);
		if (!path) seq_first = TRUE;
		break;
	default:
		break;
	}

	if (seq_first && cwin->cpref->repeat)
		path = current_playlist_nth_track((cwin->cstate->tracks_curr_playlist-1),
						  cwin);

	return path;
}

/* Remove selected rows from current playlist */

void remove_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	struct musicobject *mobj = NULL;
	gboolean played = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	if (list) {

		/* Get references from the paths and store them in the 'data'
		   portion of the list elements.
		   This idea was inspired by code from 'claws-mail' */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			ref = gtk_tree_row_reference_new(model, path);
			i->data = ref;
			gtk_tree_path_free(path);
		}
		
		/* Now build iterators from the references and delete
		   them from the store */

		for (i=list; i != NULL; i = i->next) {
			ref = i->data;
			path = gtk_tree_row_reference_get_path(ref);
			delete_rand_track_refs(path, cwin);
			test_clear_curr_seq_ref(path, cwin);

			if (gtk_tree_model_get_iter(model, &iter, path)) {
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
				if (mobj)
					test_delete_musicobject(mobj, cwin);
				gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
				gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
				cwin->cstate->tracks_curr_playlist--;
				if (!played)
					cwin->cstate->unplayed_tracks--;
			}
			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
		}
		g_list_free(list);
	}
	update_status_bar(cwin);
}

/* Crop selected rows from current playlist */

void crop_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	gboolean ret, played = FALSE;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path;
	GList *to_delete = NULL, *i = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));

	/* At least one row must be selected */

	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	ret = gtk_tree_model_get_iter_first(model, &iter);

	/* Get a reference to all the nodes that are _not_ selected */

	while (ret) {
		if (!gtk_tree_selection_iter_is_selected(selection, &iter)) {
			path = gtk_tree_model_get_path(model, &iter);
			ref = gtk_tree_row_reference_new(model, path);
			to_delete = g_list_append(to_delete, ref);
			gtk_tree_path_free(path);
		}
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	/* Delete the referenced nodes */

	for (i=to_delete; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);
		delete_rand_track_refs(path, cwin);
		test_clear_curr_seq_ref(path, cwin);

		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			if (mobj)
				test_delete_musicobject(mobj, cwin);
			gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
			cwin->cstate->tracks_curr_playlist--;
			if (!played)
				cwin->cstate->unplayed_tracks--;
		}
		gtk_tree_path_free(path);
		gtk_tree_row_reference_free(ref);
	}
	g_list_free(to_delete);
	update_status_bar(cwin);
}

/* Edit tags for selected track(s) */

void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin)
{
	struct tags otag, ntag;
	struct musicobject *mobj = NULL;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	GArray *loc_arr = NULL, *file_arr = NULL;
	gint sel = 0, location_id, changed = 0, j = 0;
	gchar *sfile = NULL, *tfile;

	memset(&otag, 0, sizeof(struct tags));
	memset(&ntag, 0, sizeof(struct tags));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	sel = gtk_tree_selection_count_selected_rows(selection);
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (!sel)
		return;

	/* Setup initial entries */

	if (sel == 1) {
		path = list->data;

		if (!gtk_tree_model_get_iter(model, &iter, path))
			goto exit;

		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (!mobj) {
			g_warning("(%s): Invalid mobj pointer\n", __func__);
			goto exit;
		}

		memcpy(&otag, mobj->tags, sizeof(struct tags));
	}

	/* Prompt the user for tag changes */

	changed = tag_edit_dialog(&otag, &ntag, cwin);
	if (!changed)
		goto exit;

	loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
	file_arr = g_array_new(TRUE, TRUE, sizeof(gchar *));

	/* Store the new tags */

	for (i = list; i != NULL; i = i->next) {
		path = i->data;
		mobj = NULL;
		if (!gtk_tree_model_get_iter(model, &iter, path))
			continue;
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (!mobj) {
			g_warning("(%s): Invalid mobj pointer\n", __func__);
			continue;
		}
		sfile = sanitize_string_sqlite3(mobj->file);
		location_id = find_location_db(sfile, cwin);
		if (location_id) {
			g_array_append_val(loc_arr, location_id);
			g_free(sfile);
			continue;
		}
		tfile = g_strdup(mobj->file);
		file_arr = g_array_append_val(file_arr, tfile);
		g_free(sfile);
	}

	tag_update(loc_arr, file_arr, changed, &ntag, cwin);

	/* NB: Optimize this to just refresh the changed tracks */

	if (changed && (loc_arr || file_arr))
		remove_current_playlist(NULL, cwin);

	if (changed && loc_arr) {
		gint elem = 0;
		for (j = 0; j < loc_arr->len; j++) {
			elem = g_array_index(loc_arr, gint, j);
			if (elem) {
				mobj = new_musicobject_from_db(elem, cwin);
				if (!mobj)
					g_critical("(%s): Invalid location ID",
						   __func__);
				else
					append_current_playlist(mobj, cwin);
			}
		}
	}

	if (changed && file_arr) {
		gchar *elem = NULL;
		for (j = 0; j < file_arr->len; j++) {
			elem = g_array_index(file_arr, gchar *, j);
			if (elem) {
				mobj = new_musicobject_from_file(elem);
				if (!mobj)
					g_critical("(%s): Invalid File",
						   __func__);
				else
					append_current_playlist(mobj, cwin);
			}
		}
	}

	if (changed && (loc_arr || file_arr))
		init_library_view(cwin);
exit:
	/* Cleanup */

	for (i=list; i != NULL; i = i->next) {
		path = i->data;
		gtk_tree_path_free(path);
	}

	if (loc_arr)
		g_array_free(loc_arr, TRUE);
	if (file_arr) {
		gchar *elem = NULL;
		for (j = 0; j < file_arr->len; j++) {
			elem = g_array_index(file_arr, gchar *, j);
			g_free(elem);
		}
		g_array_free(file_arr, TRUE);
	}

	g_free(ntag.title);
	g_free(ntag.artist);
	g_free(ntag.album);
	g_free(ntag.genre);
		
	g_list_free(list);
}

/* Show track properties dialog */

void track_properties_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GError *error = NULL;
	GtkTreeModel *model;
	GtkWidget *dialog;
	GtkTreeSelection *selection;
	GList *list;
	GtkTreePath *path;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	GtkWidget *t_hbox, *align, *tag_box, *info_box;
	gint i=0;
	gchar tags[11][20] = {P_TNO_FULL_STR,
			      P_TITLE_STR,
			      P_ARTIST_STR,
			      P_ALBUM_STR,
			      P_GENRE_STR,
			      P_YEAR_STR,
			      P_LENGTH_STR,
			      P_BITRATE_STR,
			      "Channels",
			      "Samplerate",
			      P_FILENAME_STR};

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	if (list) {
		path = list->data;
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj) {
			gchar *tno = g_strdup_printf("%d", mobj->tags->track_no);
			gchar *year = g_strdup_printf("%d", mobj->tags->year);
			gchar *length = convert_length_str(mobj->tags->length);
			gchar *bitrate = g_strdup_printf("%d", mobj->tags->bitrate);
			gchar *channels = g_strdup_printf("%d", mobj->tags->channels);
			gchar *samplerate = g_strdup_printf("%d", mobj->tags->samplerate);

			gchar *u_file = g_filename_to_utf8(mobj->file, -1,
							   NULL, NULL, &error);
			if (!u_file) {
				g_warning("(%s): Unable to convert file to UTF-8: %s\n",
					  __func__, mobj->file);
				g_error_free(error);
				error = NULL;
			}

			gchar *tr_info[11] = {tno,
			      (strlen(mobj->tags->title)) ? mobj->tags->title : "-na-",
			      (strlen(mobj->tags->artist)) ? mobj->tags->artist : "-na-",
			      (strlen(mobj->tags->album)) ? mobj->tags->album : "-na-",
			      (strlen(mobj->tags->genre)) ? mobj->tags->genre : "-na-",
			      year,
			      length,
			      bitrate,
			      channels,
			      samplerate,
			      u_file};

			dialog = gtk_dialog_new_with_buttons("Track Information",
							     GTK_WINDOW(cwin->mainwindow),
							     GTK_DIALOG_MODAL |
							     GTK_DIALOG_DESTROY_WITH_PARENT,
							     GTK_STOCK_OK,
							     GTK_RESPONSE_ACCEPT,
							     NULL);
			tag_box = gtk_vbox_new(FALSE, 0);
			info_box = gtk_vbox_new(FALSE, 0);
			t_hbox = gtk_hbox_new(FALSE, 0);

			for (i=0; i<11; i++) {
				align = gtk_alignment_new(0, 0, 0, 0);
				gtk_container_add(GTK_CONTAINER(align),
						  GTK_WIDGET(gtk_label_new(tags[i])));
				gtk_box_pack_start(GTK_BOX(tag_box),
						   GTK_WIDGET(align),
						   FALSE,
						   FALSE,
						   0);
				align = gtk_alignment_new(0, 0, 0, 0);
				gtk_container_add(GTK_CONTAINER(align),
						  GTK_WIDGET(gtk_label_new(tr_info[i])));
				gtk_box_pack_start(GTK_BOX(info_box),
						   GTK_WIDGET(align),
						   FALSE,
						   FALSE,
						   0);
			}

			gtk_box_pack_start(GTK_BOX(t_hbox),
					   GTK_WIDGET(tag_box),
					   FALSE,
					   FALSE,
					   10);
			gtk_box_pack_start(GTK_BOX(t_hbox),
					   GTK_WIDGET(info_box),
					   FALSE,
					   FALSE,
					   10);
			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
					  GTK_WIDGET(t_hbox));
			gtk_widget_show_all(dialog);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			g_free(tno);
			g_free(year);
			g_free(length);
			g_free(bitrate);
			g_free(channels);
			g_free(samplerate);
			g_free(u_file);
		}
		else
			g_critical("(%s): Dangling music object", __func__);

		gtk_tree_path_free(path);
		g_list_free(list);
	}
}

/* Clear all rows from current playlist */

void clear_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	gboolean ret, played = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	clear_rand_track_refs(cwin);
	clear_curr_seq_ref(cwin);

	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj)
			test_delete_musicobject(mobj, cwin);
		gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
		cwin->cstate->tracks_curr_playlist--;
		if (!played)
			cwin->cstate->unplayed_tracks--;
		ret = gtk_tree_model_iter_next(model, &iter);
	}
	gtk_list_store_clear(GTK_LIST_STORE(model));
	update_status_bar(cwin);
}

/* Append a track to the current playlist */

void append_current_playlist(struct musicobject *mobj, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *ch_length, *ch_track_no, *ch_year, *ch_bitrate, *ch_filename;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (!mobj) {
		g_warning("(%s): Dangling entry in current playlist", __func__);
		return;
	}
	if (!mobj->tags) {
		g_warning("(%s): Corrupt music object, no tags found", __func__);
		return;
	}

	ch_length = convert_length_str(mobj->tags->length);
	ch_track_no = g_strdup_printf("%d", mobj->tags->track_no);
	ch_year = g_strdup_printf("%d", mobj->tags->year);
	ch_bitrate = g_strdup_printf("%d", mobj->tags->bitrate);
	ch_filename = g_path_get_basename(mobj->file);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   P_MOBJ_PTR, mobj,
			   P_TRACK_NO, ch_track_no,
			   P_TITLE, mobj->tags->title,
			   P_ARTIST, mobj->tags->artist,
			   P_ALBUM, mobj->tags->album,
			   P_GENRE, mobj->tags->genre,
			   P_BITRATE, ch_bitrate,
			   P_YEAR, ch_year,
			   P_LENGTH, ch_length,
			   P_FILENAME, ch_filename,
			   P_PLAYED, FALSE,
			   -1);

	/* Increment global count of tracks */

	cwin->cstate->tracks_curr_playlist++;
	cwin->cstate->unplayed_tracks++;
	update_status_bar(cwin);

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
				     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
				     GTK_SORT_ASCENDING);
}

/* Save selected tracks as a playlist */

void save_selected_playlist(GtkAction *action, struct con_win *cwin)
{
	gchar *playlist;
	gint choice = 0;

	playlist = get_playlist_name(cwin, &choice);
	if (playlist) {
		switch(choice) {
		case NEW_PLAYLIST:
			new_playlist((const gchar *)playlist, SAVE_SELECTED, cwin);
			init_playlist_view(cwin);
			break;
		case APPEND_PLAYLIST:
			append_playlist((const gchar *)playlist, SAVE_SELECTED, cwin);
			break;
		default:
			break;

		}
		g_free(playlist);
	}
}

/* Save current playlist */

void save_current_playlist(GtkAction *action, struct con_win *cwin)
{
	gchar *playlist = NULL;
	gint choice = 0;

	playlist = get_playlist_name(cwin, &choice);
	if (playlist) {
		switch(choice) {
		case NEW_PLAYLIST:
			new_playlist((const gchar *)playlist, SAVE_COMPLETE, cwin);
			init_playlist_view(cwin);
			break;
		case APPEND_PLAYLIST:
			append_playlist((const gchar *)playlist, SAVE_COMPLETE, cwin);
			break;
		default:
			break;

		}
		g_free(playlist);
	}
}

/**********************/
/* Playback functions */
/**********************/

/* Play first track in current playlist */

void play_first_current_playlist(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	struct musicobject *mobj;
	GThread *thread;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	/* Check if tree is empty, if not play first track */

	if (gtk_tree_model_get_iter_first(model, &iter)) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		thread = start_playback(mobj, cwin);
		if (!thread)
			g_critical("(%s): Unable to create thread for playback",
				   __func__);
		else {
			clear_rand_track_refs(cwin);
			path = gtk_tree_model_get_path(model, &iter);
			update_current_state(thread, path, PLAYLIST_CURR, cwin);
			gtk_tree_path_free(path);
		}
	}
}

/* Play prev track in current playlist */

void play_prev_track(struct con_win *cwin)
{
	GtkTreePath *path;
	struct musicobject *mobj = NULL;
	GThread *thread;

	/* Get the next (prev) track to be played */

	path = current_playlist_get_prev(cwin);

	/* No more tracks */

	if (!path)
		return;

	/* Stop currently playing track */

	stop_playback(cwin);

	/* Start playing new track */

	mobj = current_playlist_mobj_at_path(path, cwin);

	thread = start_playback(mobj, cwin);
	if (!thread)
		g_critical("(%s): Unable to create playback thread", __func__);
	else
		update_current_state(thread, path, PLAYLIST_PREV, cwin);

	gtk_tree_path_free(path);
}

/* Play next track in current_playlist */

void play_next_track(struct con_win *cwin)
{
	GtkTreePath *path;
	struct musicobject *mobj = NULL;
	GThread *thread;

	/* Get the next track to be played */

	path = current_playlist_get_next(cwin);

	/* No more tracks */

	if (!path)
		return;

	/* Stop currently playing track */

	stop_playback(cwin);

	/* Start playing new track */

	mobj = current_playlist_mobj_at_path(path, cwin);

	thread = start_playback(mobj, cwin);
	if (!thread)
		g_critical("(%s): Unable to create playback thread", __func__);
	else
		update_current_state(thread, path, PLAYLIST_NEXT, cwin);

	gtk_tree_path_free(path);
}

/* Start playback of a new track, or resume playback of current track */

void play_track(struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	GThread *thread;
	GtkTreePath *path = NULL;

	/* New action is based on the current state */

	/************************************/
        /* State     Action		    */
	/* 				    */
	/* Playing   Restart playback	    */
	/* Paused    Resume playback	    */
	/* Stopped   Start playback	    */
        /************************************/

	switch (cwin->cstate->state) {
	case ST_PLAYING:
		if (cwin->cpref->shuffle && cwin->cstate->curr_rand_ref)
			path = gtk_tree_row_reference_get_path(cwin->cstate->curr_rand_ref);
		else if (!cwin->cpref->shuffle && cwin->cstate->curr_seq_ref)
			path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);

		if (path) {
			mobj = new_musicobject_from_file(cwin->cstate->curr_mobj->file);
			thread = start_playback(mobj, cwin);
			if (!thread)
				g_critical("(%s): Unable to create playback thread",
					   __func__);
			else {
				update_current_state(thread, path, PLAYLIST_CURR, cwin);
			}
			gtk_tree_path_free(path);
		}
		break;
	case ST_PAUSED:
		resume_playback(cwin);
		break;
	case ST_STOPPED:
		path = current_playlist_get_selection(cwin);
		if (!path) {
			play_first_current_playlist(cwin);
			break;
		}

		mobj = current_playlist_mobj_at_path(path, cwin);

		thread = start_playback(mobj, cwin);
		if (!thread)
			g_critical("(%s): Unable to create playback thread", __func__);
		else {
			clear_rand_track_refs(cwin);
			current_playlist_clear_dirty_all(cwin);
			cwin->cstate->unplayed_tracks =
				cwin->cstate->tracks_curr_playlist;
			update_current_state(thread, path, PLAYLIST_CURR, cwin);
		}

		gtk_tree_path_free(path);
		break;
	default:
		break;
	}
}

/* Toggle pause/resume */

void pause_resume_track(struct con_win *cwin)
{
	switch(cwin->cstate->state) {
	case ST_PAUSED:
		resume_playback(cwin);
		break;
	case ST_PLAYING:
		pause_playback(cwin);
		break;
	default:
		break;
	}
}

/*******************/
/* Event Callbacks */
/*******************/

/* Handler for row double click / kboard enter */

void current_playlist_row_activated_cb(GtkTreeView *current_playlist,
				       GtkTreePath *path,
				       GtkTreeViewColumn *column,
				       struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	struct musicobject *mobj;
	GThread *thread;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	thread = start_playback(mobj, cwin);

	if (!thread)
		g_critical("(%s): Unable to create thread for playback", __func__);
	else {
		clear_rand_track_refs(cwin);
		current_playlist_clear_dirty_all(cwin);
		cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
		update_current_state(thread, path, PLAYLIST_CURR, cwin);
	}
}

/* Handler for current playlist right click popup menu */

gboolean current_playlist_right_click_cb(GtkWidget *widget,
					 GdkEventButton *event,
					 struct con_win *cwin)
{
	GtkWidget *popup_menu, *track_prop;
	gboolean ret = FALSE;
	GtkTreeSelection *selection;
	gint n_select = 0;

	switch(event->button) {
	case 3:
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
		n_select = gtk_tree_selection_count_selected_rows(selection);

		track_prop = gtk_ui_manager_get_widget(cwin->cp_context_menu,
						       "/popup/Properties");
		if (!track_prop)
			g_critical("Unable to find prop widget");

		/* 'Properties' menuitem is shown only for a single selection */

		if (n_select != 1)
			gtk_widget_hide(GTK_WIDGET(track_prop));
		else
			gtk_widget_show(GTK_WIDGET(track_prop));

		popup_menu = gtk_ui_manager_get_widget(cwin->cp_context_menu,
						       "/popup");
		gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
			       event->button, event->time);

		/* If more than one track is selected, don't propagate event */

		if (n_select > 1)
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

/* Handler for column header right click popup menu */

gboolean header_right_click_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       struct con_win *cwin)
{
	gboolean ret = FALSE;

	switch(event->button) {
	case 3: {
		gtk_menu_popup(GTK_MENU(cwin->header_context_menu),
			       NULL, NULL, NULL, NULL, event->button, event->time);
		ret = TRUE;
	}
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

/*******/
/* DnD */
/*******/

gboolean dnd_current_playlist_drop(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   guint time,
				   struct con_win *cwin)
{
	GdkAtom target;

	if (gtk_drag_get_source_widget(context) == cwin->file_tree) {
		CDEBUG("(%s): file_tree", __func__);
		target = GDK_POINTER_TO_ATOM(g_list_nth_data(context->targets,
							     TARGET_FILENAME));
		gtk_drag_get_data(widget,
				  context,
				  target,
				  time);
		return TRUE;
	}
	else if (gtk_drag_get_source_widget(context) == cwin->library_tree) {
		CDEBUG("(%s): library_tree", __func__);
		target = GDK_POINTER_TO_ATOM(g_list_nth_data(context->targets,
							     TARGET_LOCATION_ID));
		gtk_drag_get_data(widget,
				  context,
				  target,
				  time);
		return TRUE;
	}
	else if (gtk_drag_get_source_widget(context) == cwin->playlist_tree) {
		CDEBUG("(%s): playlist_tree", __func__);
		target = GDK_POINTER_TO_ATOM(g_list_nth_data(context->targets,
							     TARGET_PLAYLIST));
		gtk_drag_get_data(widget,
				  context,
				  target,
				  time);
		return TRUE;
	}

	return FALSE;
}

/* Callback for DnD signal 'drag-data-received' */

void dnd_current_playlist_received(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   GtkSelectionData *data,
				   guint info,
				   guint time,
				   struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeRowReference *ref;
	GtkTreePath *dest_path, *path;
	GtkTreeIter dest_iter, iter;
	GtkTreeViewDropPosition pos = 0;
	GList *list = NULL, *l;
	struct musicobject *mobj = NULL;
	GArray *loc_arr, *file_arr, *playlist_arr;
	gint i = 0, elem = 0;
	gchar *name = NULL;
	gboolean ret;

	/* Reorder within current playlist */

	if (gtk_drag_get_source_widget(context) == cwin->current_playlist) {
		CDEBUG("(%s): Reorder", __func__);
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		/* Clear sort */

		clear_sort_current_playlist(NULL, cwin);

		/* No selections */

		if (!list)
			goto exit;

		/* Store references to the selected paths */

		l = list;
		while(l) {
			path = l->data;
			ref = gtk_tree_row_reference_new(model, path);
			l->data = ref;
			gtk_tree_path_free(path);
			l = l->next;
		}

		/* Move them to the new place */

		ret = gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(cwin->current_playlist),
							x, y,
							&dest_path,
							&pos);
		if (ret)
			gtk_tree_model_get_iter(model, &dest_iter, dest_path);

		for (l=list; l != NULL; l = l->next) {
			ref = l->data;
			path = gtk_tree_row_reference_get_path(ref);
			gtk_tree_model_get_iter(model, &iter, path);

			if (pos == GTK_TREE_VIEW_DROP_BEFORE)
				gtk_list_store_move_before(GTK_LIST_STORE(model), &iter,
							   (ret) ? &dest_iter : NULL);
			else if (pos == GTK_TREE_VIEW_DROP_AFTER)
				gtk_list_store_move_after(GTK_LIST_STORE(model),
							  &iter, &dest_iter);

			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
		}

		/* Cleanup and finish. */

		gtk_tree_path_free(dest_path);
		g_list_free(list);

		goto exit;
	}

	/* Append new tracks to playlist */

	switch(info) {
	case TARGET_LOCATION_ID:
		loc_arr = *(GArray **)data->data;
		if (!loc_arr)
			g_warning("(%s): No selections to process in DnD", __func__);

		CDEBUG("(%s): Target: LOCATION_ID, selection: %p, loc_arr: %p",
		       __func__, data->data, loc_arr);

		do {
			elem = g_array_index(loc_arr, gint, i);
			if (elem) {
				mobj = new_musicobject_from_db(elem, cwin);
				if (!mobj)
					g_critical("(%s): Invalid location ID",
						   __func__);
				else
					append_current_playlist(mobj, cwin);
			}
			i++;
		} while (elem != 0);

		g_array_free(loc_arr, TRUE);

		break;
	case TARGET_FILENAME:
		file_arr = *(GArray **)data->data;
		if (!file_arr)
			g_warning("(%s): No selections to process in DnD", __func__);

		CDEBUG("(%s): Target: FILENAME, selection: %p, file_arr: %p",
		       __func__, data->data, file_arr);

		while(1) {
			name = g_array_index(file_arr, gchar*, i);
			if (name) {
				mobj = new_musicobject_from_file(name);
				if (!mobj)
					g_critical("(%s): Invalid Filename: %s",
						   __func__, name);
				else
					append_current_playlist(mobj, cwin);
				g_free(name);
				i++;
			}
			else
				break;
		};

		g_array_free(file_arr, TRUE);

		break;
	case TARGET_PLAYLIST:
		playlist_arr = *(GArray **)data->data;
		if (!playlist_arr)
			g_warning("(%s): No selections to process in DnD", __func__);

		CDEBUG("(%s): Target: PLAYLIST, selection: %p, playlist_arr: %p",
		       __func__, data->data, playlist_arr);

		while(1) {
			name = g_array_index(playlist_arr, gchar*, i);
			if (name) {
				add_playlist_current_playlist(name, cwin);
				g_free(name);
				i++;
			}
			else
				break;
		};

		g_array_free(playlist_arr, TRUE);

		break;
	default:
		g_warning("(%s): Unknown DND type", __func__);
		break;
	}
exit:
	gtk_drag_finish(context, TRUE, FALSE, time);
}

/* Search through title, artist, album and filename */

gboolean current_playlist_search_compare(GtkTreeModel *model,
					 gint column,
					 const gchar *key,
					 GtkTreeIter *iter,
					 gpointer data)
{
	gchar *basename;
	struct musicobject *mobj;

	gtk_tree_model_get(model, iter, P_MOBJ_PTR, &mobj, -1);
	if(!mobj)
		return FALSE;

	if (!g_strncasecmp(key, mobj->tags->title, strlen(key)))
		return FALSE;
	if (!g_strncasecmp(key, mobj->tags->artist, strlen(key)))
		return FALSE;
	if (!g_strncasecmp(key, mobj->tags->album, strlen(key)))
		return FALSE;

	basename = g_path_get_basename(mobj->file);
	if (!g_strncasecmp(key, basename, strlen(key))) {
		g_free(basename);
		return FALSE;
	}
	g_free(basename);

	return TRUE;
}

/* Save current playlist state on exit */

void save_current_playlist_state(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint playlist_id = 0;

	playlist_id = find_playlist_db(SAVE_PLAYLIST_STATE, cwin);
	if (!playlist_id)
		playlist_id = add_new_playlist_db(SAVE_PLAYLIST_STATE,
						   cwin);
	else
		flush_playlist_db(playlist_id, cwin);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	save_playlist(playlist_id, SAVE_COMPLETE, cwin);
}

/* Init current playlist on application bringup,
   restore stored playlist */

void init_current_playlist_view(struct con_win *cwin)
{
	add_playlist_current_playlist(SAVE_PLAYLIST_STATE, cwin);
}

/* Initialize columns of current playlist */

void init_current_playlist_columns(struct con_win *cwin)
{
	const gchar *col_name;
	GtkTreeViewColumn *col;
	GList *list = NULL, *i;
	GSList *j;
	gint k = 0;

	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(cwin->current_playlist));

	/* Mark only the columns that are present in
	   cwin->cpref->playlist_columns as visible.
	   And set their sizes */

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			col = i->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name, cwin->cpref->playlist_columns)) {
				j = g_slist_nth(cwin->cpref->playlist_column_widths, k++);
				gtk_tree_view_column_set_visible(col, TRUE);
				gtk_tree_view_column_set_sizing(col,
							GTK_TREE_VIEW_COLUMN_FIXED);
				if (GPOINTER_TO_INT(j->data) > COL_WIDTH_THRESH)
					gtk_tree_view_column_set_fixed_width(col,
						     GPOINTER_TO_INT(j->data));
				else
					gtk_tree_view_column_set_fixed_width(col,
						     DEFAULT_PLAYLIST_COL_WIDTH);
			}
			else
				gtk_tree_view_column_set_visible(col, FALSE);
		}
		g_list_free(list);
	}
	else
		g_warning("(%s): No columns in playlist view", __func__);
}

/* Callback for adding/deleting track_no column */

void playlist_track_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_TRACK_NO - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting title column */

void playlist_title_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_TITLE - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting artist column */

void playlist_artist_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_ARTIST - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting album column */

void playlist_album_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_ALBUM - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting genre column */

void playlist_genre_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_GENRE - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting bitrate column */

void playlist_bitrate_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_BITRATE - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting year column */

void playlist_year_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_YEAR - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting length column */

void playlist_length_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_LENGTH - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting filename column */

void playlist_filename_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_FILENAME - 1);

	if (!col) {
		g_warning("(%s): Invalid column number", __func__);
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Comparison function for track numbers */

gint compare_track_no(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->track_no < mobj_b->tags->track_no)
		return -1;
	else if (mobj_a->tags->track_no > mobj_b->tags->track_no)
		return 1;
	else
		return 0;
}

/* Comparison function for bitrates */

gint compare_bitrate(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->bitrate < mobj_b->tags->bitrate)
		return -1;
	else if (mobj_a->tags->bitrate > mobj_b->tags->bitrate)
		return 1;
	else
		return 0;
}

/* Comparison function for years */

gint compare_year(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->year < mobj_b->tags->year)
		return -1;
	else if (mobj_a->tags->year > mobj_b->tags->year)
		return 1;
	else
		return 0;
}

/* Comparison function for lengths */

gint compare_length(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->length < mobj_b->tags->length)
		return -1;
	else if (mobj_a->tags->length > mobj_b->tags->length)
		return 1;
	else
		return 0;
}
