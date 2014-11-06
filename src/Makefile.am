AUTOMAKE_OPTIONS = subdir-objects

CHAR_SRC = char/init.c

LOGIN_SRC = login/init.c \
    login/parse.c \
    login/parse.h

MAP_SRC = map/dummy.c \
    map/dummy.h \
    map/init.c \
    map/script.c \
    map/script.h

SHARED_FLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare -fsanitize=address

lib_LTLIBRARIES = libevol_char.la libevol_login.la libevol_map.la
libevol_char_la_SOURCES = ${CHAR_SRC}
libevol_char_la_LDFLAGS = -avoid-version
libevol_char_la_CFLAGS = ${SHARED_FLAGS}
libevol_login_la_SOURCES = ${LOGIN_SRC}
libevol_login_la_LDFLAGS = -avoid-version
libevol_login_la_CFLAGS = ${SHARED_FLAGS}
libevol_map_la_SOURCES = ${MAP_SRC}
libevol_map_la_LDFLAGS = -avoid-version
libevol_map_la_CFLAGS = ${SHARED_FLAGS}

all-local: libevol_char.la libevol_login.la libevol_map.la
	cp .libs/*.so ../../../../plugins || cp .libs/*.so ../../../plugins
	cp .libs/*.so ../../../../../server-data/plugins || cp .libs/*.so ../../../../server-data/plugins
