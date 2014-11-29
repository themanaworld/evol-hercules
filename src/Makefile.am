AUTOMAKE_OPTIONS = subdir-objects

CHAR_SRC = char/char.c \
    char/char.h \
    char/config.c \
    char/config.h \
    char/init.c \
    common/interfaces.c \
    common/interfaces.h

LOGIN_SRC = login/config.c \
    login/config.h \
    login/init.c \
    login/parse.c \
    login/parse.h \
    login/send.c \
    login/send.h \
    common/interfaces.c \
    common/interfaces.h

MAP_SRC = map/clif.c \
    map/clif.h \
    map/init.c \
    map/lang.c \
    map/lang.h \
    map/npc.c \
    map/npc.h \
    map/parse.c \
    map/parse.h \
    map/pc.c \
    map/pc.h \
    map/quest.c \
    map/quest.h \
    map/script.c \
    map/script.h \
    map/scriptdefines.h \
    map/send.c \
    map/send.h \
    map/status.c \
    map/status.h \
    map/data/mapd.c \
    map/data/mapd.h \
    map/data/npcd.c \
    map/data/npcd.h \
    map/data/session.c \
    map/data/session.h \
    map/struct/mapdext.h \
    map/struct/npcdext.h \
    map/struct/sessionext.h \
    map/utils/formatutils.c \
    map/utils/formatutils.h \
    common/interfaces.c \
    common/interfaces.h

SHARED_CFLAGS = -pipe -ffast-math -Wall -Wextra -Wno-sign-compare -DPCRE_SUPPORT
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
libevol_map_la_CFLAGS = ${SHARED_CFLAGS} -Wno-strict-aliasing

all-local: libevol_char.la libevol_login.la libevol_map.la
	@cp .libs/*.so ../../../../plugins || cp .libs/*.so ../../../plugins || cp .libs/*.so ../../../server-code/plugins || cp .libs/*.so ../../server-code/plugins
	@cp .libs/*.so ../../../../../server-data/plugins || cp .libs/*.so ../../../../server-data/plugins || cp .libs/*.so ../../../server-data/plugins || cp .libs/*.so ../../server-data/plugins
