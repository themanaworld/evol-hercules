/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Hercules Renewal: Phase Two http://herc.ws/board/topic/383-hercules-renewal-phase-two/ */

#ifndef EVOL_MAP_PACKETS_STRUCT_H
#define EVOL_MAP_PACKETS_STRUCT_H

#include "common/cbasetypes.h"
#include "common/mmo.h"

struct packet_idle_unit_old {
    short PacketType;
#if PACKETVER >= 20091103
    short PacketLength;
    unsigned char objecttype;
#endif
//#if PACKETVER >= 20131223
//    unsigned int AID;
//#endif
    unsigned int GID;
    short speed;
    short bodyState;
    short healthState;
#if PACKETVER < 20080102
    short effectState;
#else
    int effectState;
#endif
    short job;
    short head;
#if PACKETVER < 7
    short weapon;
#else
    int weapon;
#endif
    short accessory;
#if PACKETVER < 7
    short shield;
#endif
    short accessory2;
    short accessory3;
    short headpalette;
    short bodypalette;
    short headDir;
#if PACKETVER >= 20101124
    short robe;
#endif
    unsigned int GUID;
    short GEmblemVer;
    short honor;
#if PACKETVER > 7
    int virtue;
#else
    short virtue;
#endif
    uint8 isPKModeON;
    unsigned char sex;
    unsigned char PosDir[3];
    unsigned char xSize;
    unsigned char ySize;
    unsigned char state;
    short clevel;
#if PACKETVER >= 20080102
    short font;
#endif
#if PACKETVER >= 20120221
    int maxHP;
    int HP;
    unsigned char isBoss;
#endif
#if PACKETVER >= 20150513
    short body;
#endif
} __attribute__((packed));

struct packet_spawn_unit_old {
    short PacketType;
#if PACKETVER >= 20091103
    short PacketLength;
    unsigned char objecttype;
#endif
//#if PACKETVER >= 20131223
//    unsigned int AID;
//#endif
    unsigned int GID;
    short speed;
    short bodyState;
    short healthState;
#if PACKETVER < 20080102
    short effectState;
#else
    int effectState;
#endif
    short job;
    short head;
#if PACKETVER < 7
    short weapon;
#else
    int weapon;
#endif
    short accessory;
#if PACKETVER < 7
    short shield;
#endif
    short accessory2;
    short accessory3;
    short headpalette;
    short bodypalette;
    short headDir;
#if PACKETVER >= 20101124
    short robe;
#endif
    unsigned int GUID;
    short GEmblemVer;
    short honor;
#if PACKETVER > 7
    int virtue;
#else
    short virtue;
#endif
    uint8 isPKModeON;
    unsigned char sex;
    unsigned char PosDir[3];
    unsigned char xSize;
    unsigned char ySize;
    short clevel;
#if PACKETVER >= 20080102
    short font;
#endif
#if PACKETVER >= 20120221
    int maxHP;
    int HP;
    unsigned char isBoss;
#endif
#if PACKETVER >= 20150513
    short body;
#endif
} __attribute__((packed));

struct packet_unit_walking_old {
    short PacketType;
#if PACKETVER >= 20091103
    short PacketLength;
#endif
#if PACKETVER > 20071106
    unsigned char objecttype;
#endif
//#if PACKETVER >= 20131223
//    unsigned int AID;
//#endif
    unsigned int GID;
    short speed;
    short bodyState;
    short healthState;
#if PACKETVER < 7
    short effectState;
#else
    int effectState;
#endif
    short job;
    short head;
#if PACKETVER < 7
    short weapon;
#else
    int weapon;
#endif
    short accessory;
    unsigned int moveStartTime;
#if PACKETVER < 7
    short shield;
#endif
    short accessory2;
    short accessory3;
    short headpalette;
    short bodypalette;
    short headDir;
#if PACKETVER >= 20101124
    short robe;
#endif
    unsigned int GUID;
    short GEmblemVer;
    short honor;
#if PACKETVER > 7
    int virtue;
#else
    short virtue;
#endif
    uint8 isPKModeON;
    unsigned char sex;
    unsigned char MoveData[6];
    unsigned char xSize;
    unsigned char ySize;
    short clevel;
#if PACKETVER >= 20080102
    short font;
#endif
#if PACKETVER >= 20120221
    int maxHP;
    int HP;
    unsigned char isBoss;
#endif
#if PACKETVER >= 20150513
    short body;
#endif
} __attribute__((packed));

struct packet_damage_old {
    short PacketType;
    unsigned int GID;
    unsigned int targetGID;
    unsigned int startTime;
    int attackMT;
    int attackedMT;
#if PACKETVER < 20071113
    short damage;
#else
    int damage;
#endif
//#if PACKETVER >= 20131223
//    unsigned char is_sp_damaged;
//#endif
    short count;
    unsigned char action;
#if PACKETVER < 20071113
    short leftDamage;
#else
    int leftDamage;
#endif
} __attribute__((packed));

#endif /* EVOL_MAP_PACKETS_STRUCT_H */
