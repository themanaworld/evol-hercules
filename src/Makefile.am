AUTOMAKE_OPTIONS = subdir-objects

MAP_SRC = map/dummy.c \
    map/dummy.h \
    map/init.c

lib_LTLIBRARIES = libevol_map.la
libevol_map_la_SOURCES = ${MAP_SRC}
libevol_map_la_LDFLAGS = -avoid-version
libevol_map_la_CFLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare

all-local: libevol_map.la
	cp .libs/*.so ../../../plugins
