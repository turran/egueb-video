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
#include "egueb_video_gst_provider.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
static int _init = 0;
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
int egueb_video_log = -1;
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI void egueb_video_init(void)
{
	if (!_init++)
	{
		eina_init();
		egueb_video_log = eina_log_domain_register("egueb-video", EGUEB_VIDEO_LOG_COLOR_DEFAULT);
		egueb_dom_init();
	}
}

EAPI void egueb_video_shutdown(void)
{
	if (_init == 1)
	{
		egueb_dom_shutdown();
		eina_log_domain_unregister(egueb_video_log);
		eina_shutdown();
	}
	_init--;
}

EAPI Egueb_Dom_Video_Provider * egueb_video_provider_new(
		const Egueb_Dom_Video_Provider_Notifier *notifier,
		Enesim_Renderer *image, Egueb_Dom_Node *n)
{
	Egueb_Dom_Video_Provider *vp = NULL;

	vp = egueb_video_ope_provider_new(notifier, image, n);
	if (vp) return vp;

	vp = egueb_video_gst_provider_new(notifier, image, n);
	return vp;
}
