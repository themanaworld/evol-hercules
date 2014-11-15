AUTOMAKE_OPTIONS = subdir-objects

CHAR_SRC = char/init.c

LOGIN_SRC = login/config.c \
    login/config.h \
    login/init.c \
    login/parse.c \
    login/parse.h \
    login/send.c \
    login/send.h

MAP_SRC = map/dummy.c \
    map/dummy.h \
    map/init.c \
    map/parse.c \
    map/parse.h \
    map/pc.c \
    map/pc.h \
    map/script.c \
    map/script.h \
    map/scriptdefines.h \
    map/send.c \
    map/send.h \
    map/session.c \
    map/session.h \
    map/sessionext.h

SHARED_CFLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare
SHARED_LDFLAGS = -avoid-version -Wl,--no-undefined

if ENABLE_SANITIZE
SHARED_CFLAGS += -fsanitize=address
SHARED_LDFLAGS += -lasan
endif

if ENABLE_STATIC
SHARED_CFLAGS += "-static-libgcc"
endif

lib_LTLIBRARIES = libevol_char.la libevol_login.la libevol_map.la
libevol_char_la_SOURCES = ${CHAR_SRC}
libevol_char_la_LDFLAGS = ${SHARED_LDFLAGS}
libevol_char_la_CFLAGS = ${SHARED_CFLAGS}
libevol_login_la_SOURCES = ${LOGIN_SRC}
libevol_login_la_LDFLAGS = ${SHARED_LDFLAGS}
libevol_login_la_CFLAGS = ${SHARED_CFLAGS}
libevol_map_la_SOURCES = ${MAP_SRC}
libevol_map_la_LDFLAGS = ${SHARED_LDFLAGS}
libevol_map_la_CFLAGS = ${SHARED_CFLAGS}

all-local: libevol_char.la libevol_login.la libevol_map.la
	cp .libs/*.so ../../../../plugins || cp .libs/*.so ../../../plugins
	cp .libs/*.so ../../../../../server-data/plugins || cp .libs/*.so ../../../../server-data/plugins
