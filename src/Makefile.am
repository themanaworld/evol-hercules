AUTOMAKE_OPTIONS = subdir-objects

CHAR_SRC = echar/char.c \
    echar/char.h \
    echar/config.c \
    echar/config.h \
    echar/init.c \
    ecommon/config.c \
    ecommon/config.h \
    ecommon/enum/gender.h \
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
    emap/battle.c \
    emap/battle.h \
    emap/battleground.c \
    emap/battleground.h \
    emap/chrif.c \
    emap/chrif.h \
    emap/clan.c \
    emap/clan.h \
    emap/clif.c \
    emap/clif.h \
    emap/config.c \
    emap/config.h \
    emap/console.c \
    emap/console.h \
    emap/craft.c \
    emap/craft.h \
    emap/craftconf.c \
    emap/craftconf.h \
    emap/effects.h \
    emap/hashtable.c \
    emap/hashtable.h \
    emap/homunculus.c \
    emap/homunculus.h \
    emap/horse.c \
    emap/horse.h \
    emap/init.c \
    emap/inter.c \
    emap/inter.h \
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
    emap/skill_const.c \
    emap/skill_const.h \
    emap/skill_ground.c \
    emap/skill_ground.h \
    emap/skill_targeted.h \
    emap/skill_targeted.c \
    emap/status.c \
    emap/status.h \
    emap/unit.c \
    emap/unit.h \
    emap/const/craft.h \
    emap/const/skilldmaxeffects.h \
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
    emap/data/skilld.c \
    emap/data/skilld.h \
    emap/enum/beingflag.h \
    emap/enum/esctype.h \
    emap/enum/esitype.h \
    emap/struct/bgdext.h \
    emap/struct/craft.h \
    emap/struct/itemdext.h \
    emap/struct/mapdext.h \
    emap/struct/mobdext.h \
    emap/struct/npcdext.h \
    emap/struct/sessionext.h \
    emap/struct/skilldext.h \
    emap/utils/formatutils.c \
    emap/utils/formatutils.h \
    ecommon/config.c \
    ecommon/config.h \
    ecommon/enum/gender.h \
    ecommon/init.c \
    ecommon/init.h \
    ecommon/serverversion.h \
    ecommon/struct/strutildata.h \
    ecommon/utils/strutil.c \
    ecommon/utils/strutil.h

# need remove -Wno-unused

SHARED_CFLAGS = ${CFLAGS} -O3 -pipe -ffast-math -std=c11 -Wall -Wextra -Wno-sign-compare -Wno-unused -fno-omit-frame-pointer \
    -DPCRE_SUPPORT -I../../.. -I../../../../3rdparty \
    -DHERCULES_CORE_HPMI_SKIP \
    -DPACKETVER=20170517 \
    -DevolPacketOffset=0x100
SHARED_LDFLAGS = -lm -avoid-version -Wl,--no-undefined

if ENABLE_STATIC_BUILD
SHARED_CFLAGS += "-static-libgcc"
endif

if ENABLE_SANITIZE
SHARED_LDFLAGS += -lasan -lubsan
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
