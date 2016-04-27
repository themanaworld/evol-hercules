AC_INIT(foobar, 1.0, me@mail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src)
AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_SRCDIR(src/foobar.cpp)

AM_INIT_AUTOMAKE([foreign])

#AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
LT_INIT
AC_PROG_LIBTOOL
AC_ENABLE_SHARED
AC_DISABLE_STATIC

AC_ARG_ENABLE(
    [sanitize],
    AC_HELP_STRING(
        [--enable-sanitize],
        [
            Enables or Disables Sanitize Address Checking (SANITIZE is disabled by default)
        ]
    ),
    [
        enable_sanitize="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-sanitize=$disableval... stopping]]);;
        esac
    ],
    [enable_sanitize="no"]
)
AM_CONDITIONAL(ENABLE_SANITIZE, test x$enable_sanitize = xyes)

AC_ARG_ENABLE(
    [static],
    AC_HELP_STRING(
        [--enable-static],
        [
            Enables or Disables Statick Linking (STATIC is disabled by default)
        ]
    ),
    [
        enable_static="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-static=$disableval... stopping]]);;
        esac
    ],
    [enable_static="no"]
)
AM_CONDITIONAL(ENABLE_STATIC, test x$enable_static = xyes)

AC_ARG_ENABLE(
    [gprof],
    AC_HELP_STRING(
        [--enable-gprof],
        [
            Enables or Disables Gprof (disabled by default)
        ]
    ),
    [
        enable_gprof="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-gprof=$disableval... stopping]]);;
        esac
    ],
    [enable_gprof="no"]
)
AM_CONDITIONAL(ENABLE_GPROF, test x$enable_gprof = xyes)

AC_ARG_ENABLE(
    [werror],
    AC_HELP_STRING(
        [--enable-werror],
        [
            Fail build if warning present]
        ]
    ),
    [
        enable_werror="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-werror=$disableval... stopping]]);;
        esac
    ],
    [enable_werror="no"]
)
AM_CONDITIONAL(ENABLE_WERROR, test x$enable_werror = xyes)

AC_DEFUN([AC_CHECK_COMPILER_WFLAG],
    [
        AC_MSG_CHECKING([whether $CC supports -W$1])
        OLD_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS -Werror -W$1"
        AC_COMPILE_IFELSE(
            [AC_LANG_SOURCE([int foo;])],
            [
                AC_MSG_RESULT([yes])
                CFLAGS="$OLD_CFLAGS -W$1"
                # Optionally, run a test
                if test "x$2" != "x"; then
                    AC_MSG_CHECKING([whether $CC can actually use -W$1])
                    CFLAGS="$OLD_CFLAGS -Werror -W$1"
                    AC_COMPILE_IFELSE(
                        [AC_LANG_SOURCE([$2])],
                        [
                            AC_MSG_RESULT([no])
                            CFLAGS="$OLD_CFLAGS"
                        ]
                        [
                            AC_MSG_RESULT([yes])
                            CFLAGS="$OLD_CFLAGS -W$1"
                        ],
                    )
                fi
            ],
            [
                AC_MSG_RESULT([no])
                CFLAGS="$OLD_CFLAGS"
            ]
        )
    ]
)

AC_CHECK_COMPILER_WFLAG(empty-body)
AC_CHECK_COMPILER_WFLAG(newline-eof)
AC_CHECK_COMPILER_WFLAG(int-conversion)
AC_CHECK_COMPILER_WFLAG(init-self)
AC_CHECK_COMPILER_WFLAG(pointer-arith)
AC_CHECK_COMPILER_WFLAG(switch-bool)
AC_CHECK_COMPILER_WFLAG(enum-conversion)
AC_CHECK_COMPILER_WFLAG(shorten-64-to-32)
AC_CHECK_COMPILER_WFLAG(constant-conversion)
AC_CHECK_COMPILER_WFLAG(bool-conversion)
AC_CHECK_COMPILER_WFLAG(format-security)
AC_CHECK_COMPILER_WFLAG(format)
AC_CHECK_COMPILER_WFLAG(format-signedness)
AC_CHECK_COMPILER_WFLAG(format-y2k)
AC_CHECK_COMPILER_WFLAG(missing-include-dirs)
AC_CHECK_COMPILER_WFLAG(suggest-attribute=noreturn)
AC_CHECK_COMPILER_WFLAG(undef)
# disabled for now
#AC_CHECK_COMPILER_WFLAG(cast-qual)

AC_CHECK_FUNC([strnlen],[CFLAGS="$CFLAGS -DHAVE_STRNLEN"])
AC_CHECK_FUNC([setrlimit],[CLAGS="$CFLAGS -DHAVE_SETRLIMIT"])

AC_MSG_CHECKING([PCRE library])
AC_CHECK_HEADER([pcre.h], [], [AC_MSG_ERROR([PCRE header not found])])
AC_SEARCH_LIBS([pcre_study], [pcre], [], AC_MSG_ERROR([PCRE not found or incompatible]))

AC_OUTPUT(Makefile src/Makefile)
