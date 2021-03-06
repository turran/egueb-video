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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "egueb_video_private.h"
#include "egueb_video_gst_provider.h"

#if HAVE_GSTREAMER
#include "gst/gst.h"

#if HAVE_GST_1
#define PLAYBIN "playbin"
#else
#define PLAYBIN "playbin2"
#endif

#endif
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#if HAVE_GSTREAMER
typedef struct _Egueb_Video_Gst_Provider
{
	Egueb_Dom_Media_Provider *vp;
	GstElement *playbin;
	Enesim_Renderer *image;
} Egueb_Video_Gst_Provider;

static int _init = 0;

static void _egueb_video_gst_provider_buffer_free(void *data, void *user_data)
{
	GstBuffer *buffer = user_data;
	gst_buffer_unref(buffer);
}

static void _egueb_video_gst_provider_fakesink_handoff_cb(GstElement *object,
		GstBuffer *buf, GstPad *pad, gpointer data)
{
	Egueb_Video_Gst_Provider *thiz = data;
	GstCaps *caps;
	const GstStructure *s;
	gint width, height;
	Enesim_Surface *surface;

	caps = gst_buffer_get_caps(buf);
	s = gst_caps_get_structure(caps, 0);
	/* get the width and height */
	gst_structure_get_int(s, "width", &width);
	gst_structure_get_int(s, "height", &height);
	gst_caps_unref(caps);

	surface = enesim_surface_new_data_from(ENESIM_FORMAT_ARGB8888,
			width, height, EINA_FALSE, GST_BUFFER_DATA(buf),
			GST_ROUND_UP_4(width * 4),
			_egueb_video_gst_provider_buffer_free,
			gst_buffer_ref(buf));
	/* lock the renderer */
	enesim_renderer_lock(thiz->image);
	/* set the new surface */
	enesim_renderer_image_source_surface_set(thiz->image, surface);
	/* unlock the renderer */
	enesim_renderer_unlock(thiz->image);
}

/* We will use this to notify egueb about some changes (buffering, state, error, whatever) */
static gboolean _egueb_video_gst_provider_bus_watch(GstBus *bus, GstMessage *msg, gpointer data)
{
	Egueb_Video_Gst_Provider *thiz = data;

	if (msg->src != (gpointer) thiz->playbin)
		return TRUE;

	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_EOS:
		break;

		case GST_MESSAGE_STATE_CHANGED:
		break;

		default:
		break;
	}
	return TRUE;
}

/*----------------------------------------------------------------------------*
 *                       The Video provider interface                         *
 *----------------------------------------------------------------------------*/
static void _egueb_video_gst_provider_descriptor_destroy(void *data)
{
	Egueb_Video_Gst_Provider *thiz = data;

	gst_element_set_state(thiz->playbin, GST_STATE_NULL);
	gst_object_unref(thiz->playbin);

	if (thiz->image)
	{
		enesim_renderer_unref(thiz->image);
		thiz->image = NULL;
	}
	free(thiz);
}

static void _egueb_video_gst_provider_descriptor_open(void *data, Egueb_Dom_String *uri)
{
	Egueb_Video_Gst_Provider *thiz = data;

	/* the uri that comes from the api must be absolute */
	gst_element_set_state(thiz->playbin, GST_STATE_READY);
	g_object_set(thiz->playbin, "uri", egueb_dom_string_chars_get(uri), NULL);
}

static void _egueb_video_gst_provider_descriptor_close(void *data)
{
	Egueb_Video_Gst_Provider *thiz = data;
	gst_element_set_state(thiz->playbin, GST_STATE_READY);
}

static void _egueb_video_gst_provider_descriptor_play(void *data)
{
	Egueb_Video_Gst_Provider *thiz = data;
	gst_element_set_state(thiz->playbin, GST_STATE_PLAYING);
}

static void _egueb_video_gst_provider_descriptor_pause(void *data)
{
	Egueb_Video_Gst_Provider *thiz = data;
	gst_element_set_state(thiz->playbin, GST_STATE_PAUSED);
}

static Egueb_Dom_Media_Provider_Descriptor _egueb_video_gst_provider = {
	/* .version 	= */ EGUEB_DOM_MEDIA_PROVIDER_DESCRIPTOR_VERSION,
	/* .destroy 	= */ _egueb_video_gst_provider_descriptor_destroy,
	/* .open 	= */ _egueb_video_gst_provider_descriptor_open,
	/* .close 	= */ _egueb_video_gst_provider_descriptor_close,
	/* .play 	= */ _egueb_video_gst_provider_descriptor_play,
	/* .pause 	= */ _egueb_video_gst_provider_descriptor_pause
};
#endif
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI Egueb_Dom_Media_Provider * egueb_video_gst_provider_new(
		Enesim_Renderer *image)
{
#if HAVE_GSTREAMER
	Egueb_Video_Gst_Provider *thiz;
	Egueb_Dom_Media_Provider *ret;
	GstElement *fakesink, *capsfilter, *sink;
	GstBus *bus;
	GstPad *pad, *ghost_pad;
	GstCaps *caps;

	/* initialize GStreamer */
	if (!_init++)
		gst_init(NULL, NULL);

	thiz = calloc(1, sizeof(Egueb_Video_Gst_Provider));
	thiz->image = enesim_renderer_ref(image);

	sink = gst_bin_new(NULL);

	/* force a rgb32 bpp */
	capsfilter = gst_element_factory_make("capsfilter", NULL);
	caps = gst_caps_new_simple (
#if HAVE_GST_1
			"video/x-raw",
			"format", G_TYPE_STRING, "BGRx",
#else
                        "video/x-raw-rgb",
			"depth", G_TYPE_INT, 24, "bpp", G_TYPE_INT, 32,
			"endianness", G_TYPE_INT, G_BIG_ENDIAN,
			"red_mask", G_TYPE_INT, 0x0000ff00,
			"green_mask", G_TYPE_INT, 0x00ff0000,
			"blue_mask", G_TYPE_INT, 0xff000000,
#endif
			NULL);
	g_object_set(capsfilter, "caps", caps, NULL);

	/* define a new sink based on fakesink to catch up every buffer */
	fakesink = gst_element_factory_make("fakesink", NULL);
	g_object_set(fakesink, "sync", TRUE, "signal-handoffs", TRUE, NULL);
	g_signal_connect(G_OBJECT(fakesink), "handoff",
			G_CALLBACK(_egueb_video_gst_provider_fakesink_handoff_cb),
			thiz); 

	gst_bin_add_many(GST_BIN(sink), capsfilter, fakesink, NULL);
	gst_element_link(capsfilter, fakesink);
	/* Create ghost pad and link it to the capsfilter */
	pad = gst_element_get_static_pad (capsfilter, "sink");
	ghost_pad = gst_ghost_pad_new ("sink", pad);
	gst_pad_set_active (ghost_pad, TRUE);
	gst_element_add_pad (GST_ELEMENT(sink), ghost_pad);
	gst_object_unref (pad);

	thiz->playbin = gst_element_factory_make(PLAYBIN, NULL);
	/* we add a message handler */
	bus = gst_pipeline_get_bus (GST_PIPELINE (thiz->playbin));
	gst_bus_add_watch (bus, _egueb_video_gst_provider_bus_watch, thiz);
	gst_object_unref (bus);

	/* finally set the sink */
	g_object_set(thiz->playbin, "video-sink", sink, NULL);

	ret = egueb_dom_media_provider_new(&_egueb_video_gst_provider,
			image, thiz);
	thiz->vp = ret;

	return ret;
#else
	return NULL;
#endif
}
