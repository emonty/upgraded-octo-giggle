# Copyright (C) 2009 Sun Microsystems, Inc.
# Copyright (C) 2019 Red Hat, Inc
# This file is free software; The Autoconf Macro copyright holders
# give unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([OPENDEV_CANONICAL_TARGET],[

  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CXX])

  # We need to prevent canonical target
  # from injecting -O2 into CFLAGS - but we won't modify anything if we have
  # set CFLAGS on the command line, since that should take ultimate precedence
  AS_IF([test "x${ac_cv_env_CFLAGS_set}" = "x"],
        [CFLAGS=""])
  AS_IF([test "x${ac_cv_env_CXXFLAGS_set}" = "x"],
        [CXXFLAGS=""])

  AX_IS_RELEASE(always)
  AM_SILENT_RULES([yes])

  AX_CXX_COMPILE_STDCXX([14],[],[mandatory])
  AM_PROG_CC_C_O
  AC_PROG_CC_STDC
  gl_VISIBILITY

  OPENDEV_OPTIMIZE
  OPENDEV_WARNINGS

  AC_ARG_WITH([comment],
    [AS_HELP_STRING([--with-comment],
      [Comment about compilation environment. @<:@default=off@:>@])],
    [with_comment=$withval],
    [with_comment=no])
  AS_IF([test "$with_comment" != "no"],[
    COMPILATION_COMMENT=$with_comment
    ],[
    COMPILATION_COMMENT="Source distribution (${PANDORA_RELEASE_COMMENT})"
    ])
  AC_DEFINE_UNQUOTED([COMPILATION_COMMENT],["$COMPILATION_COMMENT"],
    [Comment about compilation environment])

  AX_PTHREAD([
    AM_CXXFLAGS="${PTHREAD_CFLAGS} ${AM_CXXFLAGS}"
    AM_LDFLAGS="${PTHREAD_LIBS} ${AM_LDFLAGS}"
    LIBS="${PTHREAD_LIBS} ${LIBS}"
    ], [AC_MSG_ERROR([${PACKAGE} requires pthreads])])

  AM_CFLAGS="${AM_CFLAGS} ${CC_WARNINGS} ${CC_PROFILING} ${CC_COVERAGE}"
  AM_CXXFLAGS="${AM_CXXFLAGS} ${CXX_WARNINGS} ${CC_PROFILING} ${CC_COVERAGE}"

  AC_SUBST([AM_CFLAGS])
  AC_SUBST([AM_CXXFLAGS])
  AC_SUBST([AM_CPPFLAGS])
  AC_SUBST([AM_LDFLAGS])

])
