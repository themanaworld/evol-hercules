AUTOMAKE_OPTIONS = subdir-objects

MAP_SRC = map/dummy.c \
    map/dummy.h \
    map/init.c \
    map/script.c \
    map/script.h

LOGIN_SRC = login/init.c

lib_LTLIBRARIES = libevol_login.la libevol_map.la
libevol_map_la_SOURCES = ${MAP_SRC}
libevol_map_la_LDFLAGS = -avoid-version
libevol_map_la_CFLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare

libevol_login_la_SOURCES = ${LOGIN_SRC}
libevol_login_la_LDFLAGS = -avoid-version
libevol_login_la_CFLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare

all-local: libevol_login.la libevol_map.la
	cp .libs/*.so ../../../plugins
	cp .libs/*.so ../../../../server-data/plugins
