# get rid of that stupid cache mechanism
rm -f config.cache

m4_define([v_maj], [0])
m4_define([v_min], [0])
m4_define([v_mic], [1])
m4_define([v_ver], [v_maj.v_min.v_mic])

m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)

AC_INIT([egueb-video], [v_ver], [enesim-devel@googlegroups.com])
AC_PREREQ([2.60])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([foreign 1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_USE_SYSTEM_EXTENSIONS

LT_PREREQ([2.2])
LT_INIT([win32-dll])

version_info="lt_cur:lt_rev:lt_age"
AC_SUBST([version_info])
AC_DEFINE_UNQUOTED([VERSION_MAJOR], [v_maj], [Major version])
AC_DEFINE_UNQUOTED([VERSION_MINOR], [v_min], [Minor version])
AC_DEFINE_UNQUOTED([VERSION_MICRO], [v_mic], [Micro version])

VMAJ=v_maj
AC_SUBST([VMAJ])

### Needed information

AC_CANONICAL_HOST

### Checks for programs

AC_PROG_CC

# pkg-config
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if ! $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   AC_MSG_ERROR([pkg-config >= 0.22 is required])
fi

egueb_video_requirements_cflags=""
egueb_video_requirements_libs=""
egueb_video_requirements_pc="egueb-dom enesim"
egueb_video_requirements_pc_private=""

### Check for each backend
### GStreamer
PKG_CHECK_EXISTS([gstreamer-0.10],
   [
    have_gstreamer="yes"
    egueb_video_requirements_pc="${egueb_video_requirements_pc} gstreamer-0.10"
    AC_DEFINE([HAVE_GSTREAMER], [1], [Have support for gstreamer])
   ],
   [have_gstreamer="no"])

AM_CONDITIONAL([HAVE_GSTREAMER], [test "x${have_gstreamer}" = "xyes"])

### ONEPLAY
PKG_CHECK_EXISTS([oneplay-engine],
   [
    have_ope="yes"
    egueb_video_requirements_pc="${egueb_video_requirements_pc} oneplay-engine"
    AC_DEFINE([HAVE_OPE], [1], [Have support for ope])
   ],
   [have_ope="no"])

AM_CONDITIONAL([HAVE_OPE], [test "x${have_ope}" = "xyes"])


AC_SUBST([egueb_video_requirements_pc])
AC_SUBST([egueb_video_requirements_pc_private])

### Checks for libraries
PKG_CHECK_MODULES([EGUEB_VIDEO], [${egueb_video_requirements_pc}])


GST_EGUEB_CFLAGS="${EGUEB_VIDEO_CFLAGS} ${egueb_video_requirements_cflags}"
GST_EGUEB_CFLAGS="${EGUEB_VIDEO_LIBS} ${egueb_video_requirements_libs}"

## TODO Dependencies for the modules

## Make the debug preprocessor configurable

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT

#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo "Compilation.................................: make (or gmake)"
echo "  CPPFLAGS..................................: $CPPFLAGS"
echo "  CFLAGS....................................: $CFLAGS"
echo "  LDFLAGS...................................: $LDFLAGS"
echo
echo "Backends:"
echo "  GStreamer.................................: ${have_gstreamer}"
echo "  ONEPLAY Engine............................: ${have_ope}"
echo
echo "Installation................................: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix....................................: $prefix"
echo

