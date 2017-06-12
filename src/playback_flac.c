/*************************************************************************/
/* Copyright (C) 2007,2008 sacamano <m.sacamano@gmail.com>		 */
/* Copyright (C) 2007,2008 Jared Casper <jaredcasper@gmail.com>		 */
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

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder,
						     const FLAC__Frame *frame,
						     const FLAC__int32 * const buffer[],
						     void *client_data)
{
	struct con_flac_decoder *fdec = (struct con_flac_decoder*)client_data;
	guint samp, chan, out_pos = 0;
	guint16 *out_buf16 = (guint16*)fdec->out_buf;
	guint8 *out_buf8 = (guint8*)fdec->out_buf;

	if (fdec->bits_per_sample == 8) {
		for (samp = out_pos = 0; samp < frame->header.blocksize; samp++) {
			for(chan = 0; chan < frame->header.channels; chan++) {
				out_buf8[out_pos++] = (guint8)buffer[chan][samp];
			}
		}
	} else {
		for (samp = out_pos = 0; samp < frame->header.blocksize; samp++) {
			for(chan = 0; chan < frame->header.channels; chan++) {
				out_buf16[out_pos++] = (guint16)buffer[chan][samp];
			}
		}
	}

	fdec->out_buf_len = frame->header.blocksize
		* frame->header.channels
		* (fdec->bits_per_sample / 8);
	fdec->current_sample += frame->header.blocksize;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void metadata_callback(const FLAC__StreamDecoder *decoder,
			      const FLAC__StreamMetadata *metadata,
			      void *client_data)
{
	struct con_flac_decoder* fdec = (struct con_flac_decoder*)client_data;
	const FLAC__StreamMetadata_StreamInfo *si = &(metadata->data.stream_info);

	switch (metadata->type) {
	case FLAC__METADATA_TYPE_STREAMINFO:
		fdec->bits_per_sample = si->bits_per_sample;
		fdec->sample_rate = si->sample_rate;
		fdec->channels = si->channels;
		fdec->total_samples = si->total_samples;
		break;
	default:
		break;
	}
}

static void error_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__StreamDecoderErrorStatus status,
			   void *client_data)
{
	switch (status) {
	case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
		g_critical("(%s) FLAC decoder: lost sync", __func__);
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
		g_critical("(%s) FLAC decoder: bad header", __func__);
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
		g_critical("(%s) FLAC decoder: crc mismatch", __func__);
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
		g_critical("(%s) FLAC decoder: unparseable stream", __func__);
		break;
	default:
		g_critical("(%s) FLAC decoder: unknown error", __func__);
	}
}

static void update_gui(struct con_flac_decoder *fdec, struct con_win *cwin)
{
	gint newsec = fdec->current_sample / fdec->sample_rate;

	if ((newsec != fdec->displayed_seconds) &&
	    (newsec <= cwin->cstate->curr_mobj->tags->length)) {
		cwin->cstate->newsec = newsec;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);
		fdec->displayed_seconds = newsec;
	}
}

static gint flac_init(struct con_flac_decoder* fdec, struct con_win *cwin)
{
	FLAC__StreamDecoderInitStatus init_status;

	fdec->decoder = FLAC__stream_decoder_new();
	if (fdec->decoder == NULL) {
		g_critical("(%s): Unable to allocate decoder", __func__);
		return -1;
	}

	init_status = FLAC__stream_decoder_init_file(fdec->decoder,
						     cwin->cstate->curr_mobj->file,
						     write_callback,
						     metadata_callback,
						     error_callback,
						     fdec);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		g_critical("(%s): Unable to initialize decoder: %s",
			   __func__, FLAC__StreamDecoderInitStatusString[init_status]);
		return -1;
	}

	return 0;
}

static gint flac_seek(struct con_flac_decoder *fdec, struct con_win *cwin)
{
	gdouble seek_fraction;

	g_mutex_lock(cwin->cstate->c_mutex);
	seek_fraction = cwin->cstate->seek_fraction;
	g_mutex_unlock(cwin->cstate->c_mutex);

	fdec->current_sample = fdec->total_samples * seek_fraction;

	FLAC__stream_decoder_seek_absolute(fdec->decoder, fdec->current_sample);
	update_gui(fdec, cwin);

	return 0;
}

static gint flac_decode(struct con_flac_decoder *fdec, struct con_win *cwin)
{
	gint cmd = 0;

	if (flac_init(fdec, cwin) < 0)
		goto exit;

	/* Decode loop */

	do {
		/* Get and process command */

		cmd = process_thread_command(cwin);
		switch(cmd) {
		case CMD_PLAYBACK_STOP:
			goto exit;
		case CMD_PLAYBACK_SEEK:
			if (flac_seek(fdec, cwin) < 0)
				g_critical("(%s): Unable to seek", __func__);
			break;
		default:
			break;
		}

		/* Reset command */

		if (cmd) {
			g_mutex_lock(cwin->cstate->c_mutex);
			cwin->cstate->cmd = 0;
			g_mutex_unlock(cwin->cstate->c_mutex);
		}

		/* Decode */

		if (!FLAC__stream_decoder_process_single(fdec->decoder))
			break;

		if (FLAC__stream_decoder_get_state(fdec->decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
			break;

		if (cwin->cpref->software_mixer)
			soft_volume_apply((gchar *)fdec->out_buf,
					  fdec->out_buf_len, cwin);

		ao_play(cwin->clibao->ao_dev, (gchar*)fdec->out_buf, fdec->out_buf_len);

		update_gui(fdec, cwin);

	} while (1);
exit:
	if (fdec->decoder)
		FLAC__stream_decoder_delete(fdec->decoder);

	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_flac(struct con_win *cwin)
{
	gint ret = 0;
	struct con_flac_decoder *fdec;

	if (!cwin->cstate->curr_mobj->file)
		return;

	/* Open audio device */

	if (open_audio_device(cwin->cstate->curr_mobj->tags->samplerate,
			      cwin->cstate->curr_mobj->tags->channels,
			      FALSE,
			      cwin) == -1) {
		g_warning("(%s): Unable to play file: %s",
			  __func__, cwin->cstate->curr_mobj->file);
		goto exit;
	}

	CDEBUG("(%s): Playing : %s", __func__, cwin->cstate->curr_mobj->file);

	fdec = (struct con_flac_decoder*)g_slice_new0(struct con_flac_decoder);

	/* Decode */

	ret = flac_decode(fdec, cwin);

	/* Close and cleanup */

	g_slice_free(struct con_flac_decoder, fdec);
exit:
	/* Include this block of code at the end of all playback threads */

	if (!ret) {
		cwin->cstate->advance_track = TRUE;
		g_idle_add(advance_playback, cwin);
	}
	else if (ret == -1)
		cwin->cstate->advance_track = FALSE;
}
