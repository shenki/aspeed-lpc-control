AC_PREREQ([2.69])
AC_INIT([aspeed-lpc-ctrl],
        [m4_esyscmd_s([git describe --always --dirty --long 2>/dev/null || echo vUnknown])])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall foreign dist-xz])
AM_SILENT_RULES([yes])

AC_PROG_CC
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
