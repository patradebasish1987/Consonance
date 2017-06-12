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

struct musicobject* new_musicobject_from_file(gchar *file)
{
	gint type;
	struct musicobject *mobj;

	CDEBUG("(%s): Creating new musicobject from file: %s", __func__, file);

	type = get_file_type(file);
	if (type == -1)
		return NULL;

	mobj = g_slice_new0(struct musicobject);
	mobj->tags = g_slice_new0(struct tags);
	mobj->file = g_strdup(file);

	switch(type) {
	case FILE_WAV:
		if (get_wav_info(file, mobj->tags))
			mobj->file_type = FILE_WAV;
		else {
			g_critical("(%s): WAV Info failed", __func__);
			goto bad;
		}
		break;
	case FILE_MP3:
		if (get_mp3_info(file, mobj->tags))
			mobj->file_type = FILE_MP3;
		else {
			g_critical("(%s): MP3 Info failed", __func__);
			goto bad;
		}
		break;
	case FILE_FLAC:
		if (get_flac_info(file, mobj->tags))
			mobj->file_type = FILE_FLAC;
		else {
			g_critical("(%s): FLAC Info failed", __func__);
			goto bad;
		}
		break;
	case FILE_OGGVORBIS:
		if (get_ogg_info(file, mobj->tags))
			mobj->file_type = FILE_OGGVORBIS;
		else {
			g_critical("(%s): OGG Info failed", __func__);
			goto bad;
		}
		break;
#ifdef ENABLE_MODPLUG
	case FILE_MODPLUG:
		if (get_mod_info(file, mobj->tags))
			mobj->file_type = FILE_MODPLUG;
		else {
			g_critical("(%s): MOD Info failed", __func__);
			goto bad;
		}
		break;
#endif
	default:
		break;
	}

	return mobj;

bad:
	g_free(mobj->file);
	g_slice_free(struct tags, mobj->tags);
	g_slice_free(struct musicobject, mobj);
	mobj = NULL;
	return NULL;
}

struct musicobject* new_musicobject_from_db(gint location_id, struct con_win *cwin)
{
	gchar *query;
	struct db_result result;
	struct musicobject *mobj = NULL;
	gint i = 0;

	CDEBUG("(%s): Creating new musicobject with location id: %d",
	       __func__, location_id);

	query = g_strdup_printf("SELECT \
TRACK.file_type, \
TRACK.samplerate, \
TRACK.channels, \
TRACK.length, \
TRACK.bitrate, \
YEAR.year, \
TRACK.track_no, \
GENRE.name, \
ALBUM.name, \
ARTIST.name, \
TRACK.title, \
LOCATION.name \
FROM TRACK, YEAR, GENRE, ALBUM, ARTIST, LOCATION \
WHERE TRACK.location = \"%d\" \
AND YEAR.id = TRACK.year \
AND GENRE.id = TRACK.genre \
AND ALBUM.id = TRACK.album \
AND ARTIST.id = TRACK.artist \
AND LOCATION.id = \"%d\";", location_id, location_id);
	if (!exec_sqlite_query(query, cwin, &result)) {
		g_critical("(%s): Track with location id : %d not found in DB",
			   __func__, location_id);
		return NULL;
	}
	else {
		i = result.no_columns;
		mobj = g_slice_new0(struct musicobject);
		mobj->tags = g_slice_new0(struct tags);

		mobj->file = g_strdup(result.resultp[i+11]);
		mobj->tags->title = g_strdup(result.resultp[i+10]);
		mobj->tags->artist = g_strdup(result.resultp[i+9]);
		mobj->tags->album = g_strdup(result.resultp[i+8]);
		mobj->tags->genre = g_strdup(result.resultp[i+7]);
		mobj->tags->track_no = atoi(result.resultp[i+6]);
		mobj->tags->year = atoi(result.resultp[i+5]);
		mobj->tags->bitrate = atoi(result.resultp[i+4]);
		mobj->tags->length = atoi(result.resultp[i+3]);
		mobj->tags->channels = atoi(result.resultp[i+2]);
		mobj->tags->samplerate = atoi(result.resultp[i+1]);
		mobj->file_type = atoi(result.resultp[i]);

		sqlite3_free_table(result.resultp);

		return mobj;
	}
}

void delete_musicobject(struct musicobject *mobj)
{
	CDEBUG("(%s): Freeing musicobject: %s", __func__, mobj->file);

	g_free(mobj->tags->title);
	g_free(mobj->tags->artist);
	g_free(mobj->tags->album);
	g_free(mobj->tags->genre);
	g_free(mobj->file);
	g_slice_free(struct tags, mobj->tags);
	g_slice_free(struct musicobject, mobj);
}

void test_delete_musicobject(struct musicobject *mobj, struct con_win *cwin)
{
	if (!mobj)
		return;

	CDEBUG("(%s): Test freeing musicobject: %s", __func__, mobj->file);

	if (mobj == cwin->cstate->curr_mobj)
		cwin->cstate->curr_mobj_clear = TRUE;
	else
		delete_musicobject(mobj);
}
