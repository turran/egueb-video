lib_LTLIBRARIES = src/lib/libegueb_video.la

installed_headersdir = $(pkgincludedir)-$(VMAJ)
dist_installed_headers_DATA = \
src/lib/Egueb_Video.h \
src/lib/egueb_video_gst_provider.h \
src/lib/egueb_video_ope_provider.h

src_lib_libegueb_video_la_SOURCES = \
src/lib/egueb_video.c \
src/lib/egueb_video_gst_provider.c \
src/lib/egueb_video_ope_provider.c

src_lib_libegueb_video_la_CPPFLAGS = $(EGUEB_VIDEO_CFLAGS)
src_lib_libegueb_video_la_LIBADD = $(EGUEB_VIDEO_LIBS)
