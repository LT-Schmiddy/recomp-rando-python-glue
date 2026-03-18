#include "repy_api.h"
#include "recompdata.h"

#include "PR/ultratypes.h"
#define bool    u32

REPY_EXTERN_SUBINTERPRETER(rando_interp);

extern U32ValueHashmapHandle rando_location_item_map;
extern U32ValueHashmapHandle rando_location_player_map;
extern U32ValueHashmapHandle rando_location_flag_map;
extern U32HashsetHandle rando_checked_locations;

void RandoGlue_Init(char* mod_id, char* ap_game_name);
bool rando_init(char* address, char* player_name, char* password);
void rando_populate_locations();
void rando_update_cache();

bool rando_location_is_checked(u32 location_id);
// bool rando_location_is_checked_async(u32 location_id);

u32 rando_get_location_type(u32 location_id);
u32 rando_get_location_has_local_item(u32 location_id);
u32 rando_get_item_at_location(u32 location_id);
void rando_broadcast_location_hint(u32 location_id);
void rando_send_location(u32 location_id);

void rando_complete_goal();

u32 rando_has_item(u32 item_id);
u32 rando_get_own_slot_id();

u32 rando_get_items_size(); // might want a new system for this
u32 rando_get_item(u32 items_i);
s32 rando_get_item_location(u32 items_i);
u32 rando_get_sending_player(u32 items_i);

void rando_get_item_name_from_id(u32 item_id, char** out_str);
void rando_get_sending_player_name(u32 items_i, char** out_str);
void rando_get_location_item_player(u32 location_id, char** out_str);
u32 rando_get_location_item_player_id(u32 location_id);
void rando_get_location_item_name(u32 location_id, char** out_str);
u32 rando_get_last_location_sent();
u32 rando_get_seed_name(char** seed_name_out, u32 buffer_size);
void rando_get_own_slot_name(char** out_str);
void rando_get_saved_apconnect(u8* save_dir, char** address, char** player_name, char** password);
void rando_set_saved_apconnect(u8* save_dir, char* address, char* player_name, char* password);
bool rando_location_exists(u32 location_id);

void rando_queue_scout(u32 location);
void rando_queue_scouts_all();
void rando_remove_queued_scout(u32 location);
void rando_send_queued_scouts(int hint);

u32 rando_get_slotdata_u32(char* key);
// void rando_get_slotdata_raw_o32(const char* key, u32* out_handle_ptr);
// u32 rando_access_slotdata_raw_u32_o32(u32* in_handle_ptr);
// void rando_access_slotdata_raw_array_o32(u32* in_handle_ptr, u32 index, u32* out_handle_ptr);
// void rando_access_slotdata_raw_dict_o32(u32* in_handle_ptr, const char* key, u32* out_handle_ptr);

// bool rando_init(char* address, char* player_name, char* password);

// deathlink
void rando_send_death_link();
void rando_send_death_link_msg(char* death_msg);
bool rando_get_death_link_pending();
void rando_reset_death_link_pending();
bool rando_get_death_link_enabled();
void rando_toggle_death_link(bool toggle);

// solo
void rando_scan_solo_seeds(const unsigned char* save_filename);
u32 rando_solo_count();
u32 rando_solo_get_seed_name(u32 seed_index, char* out, u32 max_length); // Returns the actual string length
u32 rando_solo_get_generation_date(u32 seed_index, char* out, u32 max_length); // Returns the actual string length
u32 rando_init_solo(u32 seed_index);

void rando_yaml_init();
void rando_yaml_puts(const char* text, u32 size);
void rando_yaml_finalize(const unsigned char* save_path);
bool rando_solo_generate();