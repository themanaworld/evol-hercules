AUTOMAKE_OPTIONS = subdir-objects

CHAR_SRC = echar/char.c \
    echar/char.h \
    echar/config.c \
    echar/config.h \
    echar/init.c \
    ecommon/config.c \
    ecommon/config.h \
    ecommon/init.c \
    ecommon/init.h \
    ecommon/serverversion.h

LOGIN_SRC = elogin/config.c \
    elogin/config.h \
    elogin/init.c \
    elogin/login.c \
    elogin/login.h \
    elogin/md5calc.c \
    elogin/md5calc.h \
    elogin/mt_rand.c \
    elogin/mt_rand.h \
    elogin/parse.c \
    elogin/parse.h \
    elogin/send.c \
    elogin/send.h \
    ecommon/config.c \
    ecommon/config.h \
    ecommon/init.c \
    ecommon/init.h \
    ecommon/serverversion.h

MAP_SRC = emap/atcommand.c \
    emap/atcommand.h \
    emap/battleground.c \
    emap/battleground.h \
    emap/clif.c \
    emap/clif.h \
    emap/config.c \
    emap/config.h \
    emap/craft.c \
    emap/craft.h \
    emap/craftconf.c \
    emap/craftconf.h \
    emap/horse.c \
    emap/horse.h \
    emap/init.c \
    emap/itemdb.c \
    emap/itemdb.h \
    emap/lang.c \
    emap/lang.h \
    emap/mail.c \
    emap/mail.h \
    emap/map.c \
    emap/map.h \
    emap/mob.c \
    emap/mob.h \
    emap/npc.c \
    emap/npc.h \
    emap/packets_struct.h \
    emap/parse.c \
    emap/parse.h \
    emap/pc.c \
    emap/pc.h \
    emap/permission.c \
    emap/permission.h \
    emap/quest.c \
    emap/quest.h \
    emap/script.c \
    emap/script.h \
    emap/script_buildins.c \
    emap/script_buildins.h \
    emap/scriptdefines.h \
    emap/send.c \
    emap/send.h \
    emap/skill.c \
    emap/skill.h \
    emap/status.c \
    emap/status.h \
    emap/unit.c \
    emap/unit.h \
    emap/const/craft.h \
    emap/data/bgd.c \
    emap/data/bgd.h \
    emap/data/itemd.c \
    emap/data/itemd.h \
    emap/data/mapd.c \
    emap/data/mapd.h \
    emap/data/mobd.c \
    emap/data/mobd.h \
    emap/data/npcd.c \
    emap/data/npcd.h \
    emap/data/session.c \
    emap/data/session.h \
    emap/struct/bgdext.h \
    emap/struct/craft.h \
    emap/struct/itemdext.h \
    emap/struct/mapdext.h \
    emap/struct/mobdext.h \
    emap/struct/npcdext.h \
    emap/struct/sessionext.h \
    emap/utils/formatutils.c \
    emap/utils/formatutils.h \
    ecommon/config.c \
    ecommon/config.h \
    ecommon/init.c \
    ecommon/init.h \
    ecommon/serverversion.h \
    ecommon/struct/strutildata.h \
    ecommon/utils/strutil.c \
    ecommon/utils/strutil.h

# need remove -Wno-unused

SHARED_CFLAGS = ${CFLAGS} -pipe -ffast-math -fvisibility=hidden -rdynamic -std=c99 -Wall -Wextra -Wno-sign-compare -Wno-unused \
    -DPCRE_SUPPORT -I../../.. -I../../../../3rdparty -DPACKETVER=20150000
SHARED_LDFLAGS = -avoid-version -Wl,--no-undefined

if ENABLE_SANITIZE
#skip -fsanitize=alignment
SHARED_CFLAGS += -fsanitize=address \
-fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable \
-fsanitize=vla-bound -fsanitize=null -fsanitize=return \
-fsanitize=signed-integer-overflow -fsanitize=bounds \
-fsanitize=object-size -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow \
-fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute -fsanitize=bool \
-fsanitize=enum -fsanitize=vptr
SHARED_LDFLAGS += -lasan -lubsan -fsanitize=address \
-fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable \
-fsanitize=vla-bound -fsanitize=null -fsanitize=return \
-fsanitize=signed-integer-overflow -fsanitize=bounds \
-fsanitize=object-size -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow \
-fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute -fsanitize=bool \
-fsanitize=enum -fsanitize=vptr
endif

if ENABLE_STATIC
SHARED_CFLAGS += "-static-libgcc"
endif

if ENABLE_GPROF
SHARED_CFLAGS += "-pg"
endif

if ENABLE_WERROR
SHARED_CFLAGS += "-Werror"
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
