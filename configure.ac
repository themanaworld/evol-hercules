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
    [static-build],
    AC_HELP_STRING(
        [--enable-static-build],
        [
            Enables or Disables Statick Linking (STATIC is disabled by default)
        ]
    ),
    [
        enable_static_build="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-static-build=$disableval... stopping]]);;
        esac
    ],
    [enable_static_build="no"]
)
AM_CONDITIONAL(ENABLE_STATIC_BUILD, test x$enable_static_build = xyes)

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

AC_ARG_ENABLE(
    [lto],
        AC_HELP_STRING(
        [--enable-lto],
        [
            Enables or Disables Linktime Code Optimization (LTO is disabled by default)
        ]
    ),
    [
        enable_lto="$enableval"
        case $enableval in
            "no");;
            "yes");;
            *) AC_MSG_ERROR([[invalid argument --enable-lto=$enableval... stopping]]);;
        esac
    ],
    [enable_lto="no"]
)

OLD_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS -rdynamic"
AC_MSG_CHECKING([whether $CC accepts -rdynamic])
AC_LINK_IFELSE(
    [AC_LANG_SOURCE([
        int main(int argc, char **argv)
        {
            return 0;
        }
    ])],
    [
        AC_MSG_RESULT([yes])
    ],
    [
        AC_MSG_RESULT([no])
        LDFLAGS="$OLD_LDFLAGS"
    ],
    [
        AC_MSG_RESULT([guessing no])
    ]
)

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

if test "$enable_lto" != "no" ; then
    OLD_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -flto -ffat-lto-objects"

    OLD_LDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS -flto -ffat-lto-objects"

    AC_MSG_CHECKING([whether $CC supports -flto])
    AC_RUN_IFELSE(
        [AC_LANG_SOURCE([
            int main(int argc, char **argv){
                return 0;
            }
        ])],
        [
            AC_MSG_RESULT([yes])
        ],
        [
            AC_MSG_RESULT([no])
            CFLAGS="$OLD_CFLAGS"
            LDFLAGS="$OLD_LDFLAGS"
        ],
        [
            AC_MSG_RESULT([guessing no])
        ]
    )
fi

AC_DEFUN([AC_CHECK_SANITIZER_FLAG],
    [
        AC_MSG_CHECKING([whether $CC supports -fsanitize=$1])
        OLD_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS -fsanitize=$1"
        OLD_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS -fsanitize=$1"
        AC_COMPILE_IFELSE(
            [AC_LANG_SOURCE([int foo;])],
            [
                AC_MSG_RESULT([yes])
            ],
            [
                AC_MSG_RESULT([no])

                AC_MSG_CHECKING([whether $CC requires -fsanitize-undefined-trap-on-error for -fsanitize=$1])
                CFLAGS="$CFLAGS -fsanitize-undefined-trap-on-error"
                AC_COMPILE_IFELSE(
                    [AC_LANG_SOURCE([int foo;])],
                    [
                        AC_MSG_RESULT([yes])
                    ],
                    [
                        AC_MSG_RESULT([no])
                        CFLAGS="$OLD_CFLAGS"
                        LDFLAGS="$OLD_LDFLAGS"
                    ]
                )
            ],
            [
                AC_MSG_RESULT([guessing no])
                CFLAGS="$OLD_CFLAGS"
                LDFLAGS="$OLD_LDFLAGS"
            ]
        )
    ]
)

if test "$enable_sanitize" != "no" ; then
    AC_CHECK_SANITIZER_FLAG(address)
    # skipped because server have multiple issues: -fsanitize=alignment
    AC_CHECK_SANITIZER_FLAG(shift)
    AC_CHECK_SANITIZER_FLAG(integer-divide-by-zero)
    AC_CHECK_SANITIZER_FLAG(unreachable)
    AC_CHECK_SANITIZER_FLAG(vla-bound)
    AC_CHECK_SANITIZER_FLAG(null)
    AC_CHECK_SANITIZER_FLAG(return)
    AC_CHECK_SANITIZER_FLAG(signed-integer-overflow)
    AC_CHECK_SANITIZER_FLAG(bounds)
    AC_CHECK_SANITIZER_FLAG(bounds-strict)
    AC_CHECK_SANITIZER_FLAG(object-size)
    AC_CHECK_SANITIZER_FLAG(float-divide-by-zero)
    AC_CHECK_SANITIZER_FLAG(float-cast-overflow)
    AC_CHECK_SANITIZER_FLAG(nonnull-attribute)
    AC_CHECK_SANITIZER_FLAG(returns-nonnull-attribute)
    AC_CHECK_SANITIZER_FLAG(bool)
    AC_CHECK_SANITIZER_FLAG(enum)
    AC_CHECK_SANITIZER_FLAG(vptr)
fi

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
AC_CHECK_COMPILER_WFLAG(logical-op)
AC_CHECK_COMPILER_WFLAG(nested-externs)
AC_CHECK_COMPILER_WFLAG(old-style-definition)
AC_CHECK_COMPILER_WFLAG(overlength-strings)
AC_CHECK_COMPILER_WFLAG(redundant-decls)
#AC_CHECK_COMPILER_WFLAG(unused)
AC_CHECK_COMPILER_WFLAG(unused-but-set-parameter)
AC_CHECK_COMPILER_WFLAG(unused-but-set-variable)
AC_CHECK_COMPILER_WFLAG(unreachable-code)
AC_CHECK_COMPILER_WFLAG(unused-const-variable=2)
AC_CHECK_COMPILER_WFLAG(unused-function)
AC_CHECK_COMPILER_WFLAG(unused-local-typedefs)
AC_CHECK_COMPILER_WFLAG(unused-macros)
AC_CHECK_COMPILER_WFLAG(unused-parameter)
AC_CHECK_COMPILER_WFLAG(unused-result)
AC_CHECK_COMPILER_WFLAG(unused-value)
AC_CHECK_COMPILER_WFLAG(unused-variable)
#AC_CHECK_COMPILER_WNOFLAG(format-nonliteral)
#AC_CHECK_COMPILER_WNOFLAG(switch)
#AC_CHECK_COMPILER_WNOFLAG(missing-field-initializers)

AC_CHECK_FUNC([strnlen],[CFLAGS="$CFLAGS -DHAVE_STRNLEN"])
AC_CHECK_FUNC([setrlimit],[CLAGS="$CFLAGS -DHAVE_SETRLIMIT"])

AC_MSG_CHECKING([PCRE library])
AC_CHECK_HEADER([pcre.h], [], [AC_MSG_ERROR([PCRE header not found])])
AC_SEARCH_LIBS([pcre_study], [pcre], [], AC_MSG_ERROR([PCRE not found or incompatible]))

AC_OUTPUT(Makefile src/Makefile)
