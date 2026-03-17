#include "main.h"

bool rando_location_is_checked(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_location_is_checked,
        "location_id in recomp_data.ctx.locations_checked", // checked_locations is server checked while locations_checked is in the client
        checked
    );
    REPY_FN_CLEANUP;
    return checked;
}

// bool rando_location_is_checked_async(u32 location_id); // unneeded

u32 rando_get_location_type(u32 location_id) { // could technically be a u8
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_location_type,
        "recomp_data.ctx.locations_info[location_id].flags",
        type
    );
    REPY_FN_CLEANUP;
    return type;
}

bool rando_get_location_has_local_item(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_get_location_has_local_item,
        "recomp_data.ctx.locations_info[location_id].player == recomp_data.ctx.slot",
        is_local
    );
    REPY_FN_CLEANUP;
    return is_local;
}

u32 rando_get_item_at_location(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_item_at_location,
        "recomp_data.ctx.locations_info[location_id].item",
        item_id
    );
    REPY_FN_CLEANUP;
    return item_id;
}

// this would likely be better as an async function, queueing the locations to send on the update loop
void rando_send_location(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_send_location,
        "recomp_data.last_location_sent = location_id\n"
        "check_func = recomp_data.ctx.check_locations([location_id])\n"
        "RecompClient.run_async_task_once(check_func)\n"
    );
    REPY_FN_CLEANUP;
}

void rando_complete_goal(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_complete_goal,
        "import RecompClient\n"
        "msg_func = recomp_data.ctx.send_msgs([{'cmd': 'StatusUpdate', 'status': ClientStatus.CLIENT_GOAL}])\n"
        "RecompClient.run_async_task_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

u32 rando_has_item(u32 item_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("item_id", item_id);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_has_item, 
        "recomp_data.ctx.recieved_item_ids.count(item_id)",
        item_count
    );
    REPY_FN_CLEANUP;
    return item_count;
}

// u32 rando_has_item_async(u32 item_id);

u32 rando_get_own_slot_id() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_own_slot_id,
        "recomp_data.ctx.slot",
        slot
    );
    REPY_FN_CLEANUP;
    return slot;
}

// might want a new system for this in the actual randomizers
u32 rando_get_items_size() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_items_size,
        "len(recomp_data.ctx.items_received)",
        items_size
    );
    REPY_FN_CLEANUP;
    return items_size;
}

// same as above, this is dumb lol
u32 rando_get_item(u32 items_index) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("index", items_index);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_item,
        "recomp_data.ctx.items_received[index].item",
        item
    );
    REPY_FN_CLEANUP;
    return item;
}

// okay i'm getting lost at this point
// also i'm ripping this out in mm for something better because man.
s32 rando_get_item_location(u32 items_index) {
    if (items_index > rando_get_items_size()) {
        return 0;
    }

    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("index", items_index);
    REPY_FN_EVAL_CACHE_S32(
        py_rando_get_item_location,
        "recomp_data.ctx.items_received[index].location",
        item_location
    );
    REPY_FN_CLEANUP;
    return item_location;
}

// same as above, but this is actually used by other randos
s32 rando_get_sending_player(u32 items_index) {
    if (items_index > rando_get_items_size()) {
        return 0;
    }
    
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("index", items_index);
    REPY_FN_EVAL_CACHE_S32(
        py_rando_get_sending_player, 
        "recomp_data.ctx.items_received[index].player",
        sending_player
    );
    REPY_FN_CLEANUP;
    return sending_player;
}

// make this require a max string length
void rando_get_item_name_from_id(u32 item_id, char** out_str) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("item_id", item_id);
    REPY_FN_EXEC_CACHE(
        py_rando_get_item_name_from_id,
        "item_name = recomp_data.ctx.item_names[item_id]\n"
        "item_name_len = len(item_name)\n" // temp?
    );
    (*out_str) = REPY_FN_GET_STR("item_name");
    REPY_FN_CLEANUP;
}

// make this require a max string length (also don't like how this is)
void rando_get_sending_player_name(u32 items_index, char** out_str) {
    if (items_index > rando_get_items_size()) {
        return;
    }

    s32 player_slot = rando_get_sending_player(items_index);
    
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_S32("player_slot", player_slot);
    REPY_FN_EXEC_CACHE(
        py_rando_get_sending_player_name,
        "player_name = recomp_data.ctx.player_names[player_slot]"
    );
    (*out_str) = REPY_FN_GET_STR("player_name");
    REPY_FN_CLEANUP;
}

// make this require a max string length
void rando_get_location_item_player(u32 location_id, char** out_str) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_get_location_item_player,
        "player_slot = recomp_data.ctx.locations_info[location_id].player\n"
        "player_name = recomp_data.ctx.player_names[player_slot]\n"
    );
    (*out_str) = REPY_FN_GET_STR("player_name");
    REPY_FN_CLEANUP;
}

u32 rando_get_location_item_player_id(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_location_item_player,
        "recomp_data.ctx.locations_info[location_id].player\n",
        player_slot
    );
    REPY_FN_CLEANUP;
    return player_slot;
}

// make this require a max string length
void rando_get_location_item_name(u32 location_id, char** out_str) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_get_location_item_name,
        "item_id = recomp_data.ctx.locations_info[location_id].item\n"
        "item_name = recomp_data.ctx.item_names[item_id]"
    );
    (*out_str) = REPY_FN_GET_STR("item_name");
    REPY_FN_CLEANUP;
}

u32 rando_get_last_location_sent() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_last_location_sent,
        "recomp_data.last_location_sent\n",
        location_id
    );
    REPY_FN_CLEANUP;
    return location_id;
}

u32 rando_get_seed_name(char** seed_name_out, u32 buffer_size) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_seed_name,
        "seed_name = recomp_data.ctx.seed_name"
    );
    (*seed_name_out) = REPY_FN_GET_STR("seed_name");
    REPY_FN_CLEANUP;
    return 0; // temp
}

// make this require a max string length
void rando_get_own_slot_name(char** out_str) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_location_item_name,
        "name = recomp_data.ctx.player_names[recomp_data.ctx.slot]"
    );
    (*out_str) = REPY_FN_GET_STR("name");
    REPY_FN_CLEANUP;
}

void rando_get_saved_apconnect(u8* save_dir, char** address, char** player_name, char** password) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_saved_apconnect,
        "import RecompClient\n"
        "connection_info = RecompClient.get_ap_connect()\n"
        "address = connection_info[0]\n"
        "player_name = connection_info[1]\n"
        "password = connection_info[2]\n"
    );
    (*address) = REPY_FN_GET_STR("address");
    (*player_name) = REPY_FN_GET_STR("player_name");
    (*password) = REPY_FN_GET_STR("password");
    REPY_FN_CLEANUP;
}

void rando_set_saved_apconnect(u8* save_dir, char* address, char* player_name, char* password) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("address", address);
    REPY_FN_SET_STR("player_name", player_name);
    REPY_FN_SET_STR("password", password);
    REPY_FN_EXEC_CACHE(
        py_rando_set_saved_apconnect,
        "import RecompClient\n"
        "RecompClient.save_ap_connect(address, player_name, password)\n"
    );
    REPY_FN_CLEANUP;
}

bool rando_location_exists(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_location_exists,
        "location_id in recomp_data.ctx.locations_info", // would server_locations make more sense?
        exists
    );
    REPY_FN_CLEANUP;
    return exists;
}

// SCOUTS

void rando_queue_scout(u32 location) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location", location);
    REPY_FN_EXEC_CACHE(
        py_rando_queue_scout,
        "recomp_data.queued_scouts.add(location)"
    );
    REPY_FN_CLEANUP;
}

void rando_queue_scouts_all() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_queue_scouts_all,
        "recomp_data.queued_scouts = recomp_data.ctx.server_locations"
    );
    REPY_FN_CLEANUP;
}

void rando_remove_queued_scout(u32 location) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location", location);
    REPY_FN_EXEC_CACHE(
        py_rando_remove_queued_scout,
        "recomp_data.queued_scouts.discard(location)"
    );
    REPY_FN_CLEANUP;
}

void rando_send_queued_scouts(int hint) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("hint", hint);
    REPY_FN_EXEC_CACHE(
        py_rando_send_queued_scouts,
        "import RecompClient\n"
        "recomp_data.ctx.locations_scouted = recomp_data.queued_scouts\n"
        "msg_func = recomp_data.ctx.send_msgs([{\"cmd\": \"LocationScouts\",\n"
        "                                       \"locations\": list(recomp_data.queued_scouts),\n"
        "                                       \"create_as_hint\": hint}])\n"
        "RecompClient.run_async_task_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

void rando_broadcast_location_hint(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_broadcast_location_hint,
        "import RecompClient\n"
        "msg_func = recomp_data.ctx.send_msgs([{\"cmd\": \"LocationScouts\",\n"
        "                                       \"locations\": list(location),\n"
        "                                       \"create_as_hint\": True}])\n"
        "RecompClient.run_async_task_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

// SLOTDATA

u32 rando_get_slotdata_u32(char* key) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("key", key);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_slotdata_u32,
        "recomp_data.ctx.slot_data[key]",
        slotdata_value
    );
    REPY_FN_CLEANUP;
    return slotdata_value;
}

// not even going to pretend i know how these work well enough to implement them
// void rando_get_slotdata_raw_o32(const char* key, u32* out_handle_ptr);
// u32 rando_access_slotdata_raw_u32_o32(u32* in_handle_ptr);
// void rando_access_slotdata_raw_array_o32(u32* in_handle_ptr, u32 index, u32* out_handle_ptr);
// void rando_access_slotdata_raw_dict_o32(u32* in_handle_ptr, const char* key, u32* out_handle_ptr);

// bool rando_init(char* address, char* player_name, char* password);

// DEATHLINK

void rando_send_death_link() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_send_death_link,
        "import RecompClient\n"
        "deathlink_func = recomp_data.ctx.send_death()\n"
        "RecompClient.run_async_task_once(deathlink_func)\n"
    );
    REPY_FN_CLEANUP;
}

void rando_send_death_link_msg(char* death_msg) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("death_msg", death_msg);
    REPY_FN_EXEC_CACHE(
        py_rando_send_death_link_msg,
        "import RecompClient\n"
        "deathlink_func = recomp_data.ctx.send_death(death_msg)\n"
        "RecompClient.run_async_task_once(deathlink_func)\n"
    );
    REPY_FN_CLEANUP;
}

bool rando_get_death_link_pending() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_get_death_link_pending,
        "recomp_data.ctx.deathlink_pending",
        pending
    );
    REPY_FN_CLEANUP;
    return pending;
}

void rando_reset_death_link_pending() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_reset_death_link_pending,
        "recomp_data.ctx.deathlink_pending = False"
    );
    REPY_FN_CLEANUP;
}

bool rando_get_death_link_enabled() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_get_death_link_pending,
        "'DeathLink' in recomp_data.ctx.tags",
        enabled
    );
    REPY_FN_CLEANUP;
    return enabled;
}

void rando_toggle_death_link(bool toggle) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_BOOL("toggle", toggle);
    REPY_FN_EXEC_CACHE(
        py_rando_toggle_death_link,
        "import RecompClient\n"
        "deathlink_func = recomp_data.ctx.update_death_link(toggle)\n"
        "RecompClient.run_async_task_once(deathlink_func)\n"
    );
    REPY_FN_CLEANUP;
}

// this should hash the seed name + slot number into a random seed either the randomizer or other mods can use
u32 rando_get_random_seed() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_random_seed,
        "import hashlib\n"
        "ctx = recomp_data.ctx\n"
        "hashed_seed = int.from_bytes(hashlib.sha256((ctx.seed_name + ctx.slot).encode('utf-8')))\n"
    );
    REPY_FN_CLEANUP;
    return REPY_FN_GET_U32("hashed_seed");
}

// everything below is mm specific slotdata stuff that will need to be altered in the mm repo
// bool rando_advanced_shops_enabled(); // not used?
// u32 rando_damage_multiplier(); modified manually
// s16 rando_get_shop_price(u32 shop_item_id); // game specific

// bool rando_get_game_is_oot(u32 player_id);
// bool rando_get_game_is_ww(u32 player_id);