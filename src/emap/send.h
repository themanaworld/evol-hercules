// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

void send_npccommand (TBL_PC *sd, int npcId, int cmd);
void send_npccommand2 (TBL_PC *sd, int npcId, int cmd, int id, int x, int y);
void send_local_message(int fd, struct block_list* bl, const char* msg);
void send_changelook(struct map_session_data* sd, struct map_session_data* sd2, int fd,
                     int id, int type, int val, int val2,
                     struct item_data *data, int n);
void send_mapmask(int fd, int mask);
void send_mapmask_brodcast(const int map, const int mask);
void send_mob_info(struct block_list* bl1, struct block_list* bl2, enum send_target target);
void send_advmoving(struct unit_data* ud, bool moving, struct block_list *tbl, enum send_target target);
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
void send_client_command(TBL_PC *sd, const char *const command);
void send_changelook2(struct map_session_data* sd, struct block_list *bl,
                     int id, int type,
                     int val, int val2, struct item_data *data, int n,
                     enum send_target target);
void send_setwall(int m,
                  int x1, int y1,
                  int x2, int y2,
                  int mask,
                  enum send_target target);
void send_setwall_single(int fd,
                         int m,
                         int x1, int y1,
                         int x2, int y2,
                         int mask);

#endif  // EVOL_MAP_PC
