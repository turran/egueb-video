/* Egueb Video - Video Providers for Egueb
 * Copyright (C) 2014 Jorge Luis Zapata
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "egueb_video_private.h"
#include "egueb_video_ope_provider.h"

#if HAVE_OPE
#include "oneplay-engine.h"
#endif
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#if HAVE_OPE
typedef struct _Egueb_Video_Ope_Provider
{
	Egueb_Dom_Media_Provider *vp;
	Enesim_Renderer *image;
	OPEPlayer *player;
	OPEStream *stream;
	int width;
	int height;
} Egueb_Video_Ope_Provider;

static int _init = 0;

static gboolean
_egueb_video_ope_provider_request_render_mode_cb(OPEPlayer * player,
		gpointer event, gpointer data)
{
	OPEPlayerEventRequestRenderMode *ev = event;
	const OPEStreamInfo *info;

	ope_stream_info_get(ev->stream, &info);
	if (info->type != OPE_STREAM_TYPE_VIDEO)
		return EINA_TRUE;

	/* We dont want to render the video streams, but we want the stream
	 * synchronized */
	ev->render = FALSE;
	ev->synchronize = TRUE;
	return TRUE;
}

static gboolean
_egueb_video_ope_provider_stream_info_updated_cb(OPEPlayer * player,
		gpointer event, gpointer data)
{
	Egueb_Video_Ope_Provider *thiz = data;
	OPEPlayerEventStreamInfoUpdated *ev = event;
	const OPEStreamInfo *info;

	ope_stream_info_get(ev->stream, &info);
	if (info->type != OPE_STREAM_TYPE_VIDEO)
		return EINA_TRUE;

	thiz->stream = ope_stream_ref(ev->stream);
	thiz->width = info->data.video.width;
	thiz->height = info->data.video.height;
	return EINA_TRUE;
}

static gboolean
_egueb_video_ope_provider_render_cb(OPEPlayer * player, gpointer event,
		gpointer data)
{
	Egueb_Video_Ope_Provider *thiz = data;
	OPEPlayerEventRender *ev = event;
	Enesim_Surface *surface;
	void *sw_data;
	size_t stride;

	if (ev->stream != thiz->stream)
		return EINA_TRUE;
	if (!thiz->width || !thiz->height)
		return EINA_TRUE;

	surface = enesim_surface_new(ENESIM_FORMAT_ARGB8888,
			thiz->width, thiz->height);
	enesim_surface_sw_data_get(surface, &sw_data, &stride);
	ope_stream_frame_rgb_get(ev->stream, OPE_STREAM_VIDEO_RGB_FORMAT_B8G8R8X8,
			thiz->width, thiz->height, sw_data, stride);

	/* lock the renderer */
	enesim_renderer_lock(thiz->image);
	/* set the new surface */
	enesim_renderer_image_source_surface_set(thiz->image, surface);
	/* unlock the renderer */
	enesim_renderer_unlock(thiz->image);
	return TRUE;
}

/*----------------------------------------------------------------------------*
 *                       The Video provider interface                         *
 *----------------------------------------------------------------------------*/
static void _egueb_video_ope_provider_descriptor_destroy(void *data)
{
	Egueb_Video_Ope_Provider *thiz = data;

	if (thiz->stream)
	{
		ope_stream_unref(thiz->stream);
		thiz->stream = NULL;
	}
	ope_player_free(thiz->player);

	if (thiz->image)
	{
		enesim_renderer_unref(thiz->image);
		thiz->image = NULL;
	}
	free(thiz);
}

static void _egueb_video_ope_provider_descriptor_open(void *data, Egueb_Dom_String *uri)
{
	Egueb_Video_Ope_Provider *thiz = data;
	ope_player_open(thiz->player, egueb_dom_string_chars_get(uri));
}

static void _egueb_video_ope_provider_descriptor_close(void *data)
{
	Egueb_Video_Ope_Provider *thiz = data;
	ope_player_close(thiz->player);
}

static void _egueb_video_ope_provider_descriptor_play(void *data)
{
	Egueb_Video_Ope_Provider *thiz = data;
	ope_player_play(thiz->player);
}

static void _egueb_video_ope_provider_descriptor_pause(void *data)
{
	Egueb_Video_Ope_Provider *thiz = data;
	ope_player_pause(thiz->player);
}

static Egueb_Dom_Media_Provider_Descriptor _egueb_video_ope_provider = {
	/* .version 	= */ EGUEB_DOM_MEDIA_PROVIDER_DESCRIPTOR_VERSION,
	/* .destroy 	= */ _egueb_video_ope_provider_descriptor_destroy,
	/* .open 	= */ _egueb_video_ope_provider_descriptor_open,
	/* .close 	= */ _egueb_video_ope_provider_descriptor_close,
	/* .play 	= */ _egueb_video_ope_provider_descriptor_play,
	/* .pause 	= */ _egueb_video_ope_provider_descriptor_pause
};
#endif
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI Egueb_Dom_Media_Provider * egueb_video_ope_provider_new(
		Enesim_Renderer *image)
{
#if HAVE_OPE
	Egueb_Video_Ope_Provider *thiz;
	Egueb_Dom_Media_Provider *ret;

	/* initialize OPE */
	if (!_init++)
		ope_initialize();


	thiz = calloc(1, sizeof(Egueb_Video_Ope_Provider));
	thiz->image = enesim_renderer_ref(image);
	thiz->player = ope_player_new();
	ope_player_event_listener_add (thiz->player,
			OPE_PLAYER_EVENT_REQUEST_RENDER_MODE,
			_egueb_video_ope_provider_request_render_mode_cb, thiz);
	ope_player_event_listener_add (thiz->player, OPE_PLAYER_EVENT_RENDER,
			_egueb_video_ope_provider_render_cb, thiz);

	ope_player_event_listener_add (thiz->player, OPE_PLAYER_EVENT_STREAM_INFO_UPDATED,
			_egueb_video_ope_provider_stream_info_updated_cb, thiz);

	ret = egueb_dom_media_provider_new(&_egueb_video_ope_provider,
			image, thiz);
	thiz->vp = ret;

	return ret;
#else
	return NULL;
#endif
}

