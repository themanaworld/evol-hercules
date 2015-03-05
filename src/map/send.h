// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

void send_npccommand (TBL_PC *sd, int npcId, int cmd);
void send_npccommand2 (TBL_PC *sd, int npcId, int cmd, int id, int x, int y);
void send_local_message(int fd, struct block_list* bl, const char* msg);
void send_changelook(int fd, int id, int type, int val);
void send_mapmask(int fd, int mask);
void send_mapmask_brodcast(const int map, const int mask);
void send_mob_info(struct block_list* bl1, struct block_list* bl2, enum send_target target);
void send_advmoving(struct unit_data* ud, struct block_list *tbl, enum send_target target);
void send_changemusic_brodcast(const int map, const char *music);
void send_changenpc_title (TBL_PC *sd, const int npcId, const char *name);
void send_join_ack(int fd, const char *const name, int flag);
void send_pc_info(struct block_list* bl1,
                  struct block_list* bl2,
                  enum send_target target);
void send_npc_info(struct block_list* bl1,
                   struct block_list* bl2,
                   enum send_target target);
void send_slave_say(TBL_PC *sd,
                    struct block_list *bl,
                    const char *const name,
                    const char *const message);
void send_online_list(int fd, const char *buf, unsigned size);

#endif  // EVOL_MAP_PC
