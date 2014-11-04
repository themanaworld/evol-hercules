AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libevol_map.la
libevol_map_la_SOURCES = map/dummy.c
libevol_map_la_LDFLAGS = -avoid-version
libevol_map_la_CFLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare

all-local: libevol_map.la
	cp .libs/*.so ../../../plugins
