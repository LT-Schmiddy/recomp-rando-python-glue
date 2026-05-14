#include "main.h"
#include "rando_glue.h"

RECOMP_EXPORT bool rando_is_connected() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_is_connected,
        "recomp_data.ctx.is_connected()",
        connected
    );
    REPY_FN_CLEANUP;
    return connected;
}

RECOMP_EXPORT bool rando_location_is_checked(u32 location_id) {
    return recomputil_u32_hashset_contains(rando_checked_locations, location_id);
}

// bool rando_location_is_checked_async(u32 location_id); // unneeded

RECOMP_EXPORT u32 rando_get_location_type(u32 location_id) { // could technically be a u8
    if (!rando_location_exists(location_id)) {
        if (DEBUG_GLUE) {
            recomp_printf("location 0x%06X does not exist in %s\n", location_id, __func__);
        }
        return 0;
    }
    
    u32 flags;
    recomputil_u32_value_hashmap_get(rando_location_flag_map, location_id, &flags);
    return flags;
}

RECOMP_EXPORT bool rando_get_location_has_local_item(u32 location_id) {
    if (!rando_location_exists(location_id)) {
        if (DEBUG_GLUE) {
            recomp_printf("location 0x%06X does not exist in %s\n", location_id, __func__);
        }
        return 0;
    }

    return rando_get_location_item_player_id(location_id) == rando_get_own_slot_id();
}

RECOMP_EXPORT u32 rando_get_item_at_location(u32 location_id) {
    if (!rando_location_exists(location_id)) {
        if (DEBUG_GLUE) {
            recomp_printf("location 0x%06X does not exist in %s\n", location_id, __func__);
        }
        return 0;
    }
    
    u32 item_id;
    recomputil_u32_value_hashmap_get(rando_location_item_map, location_id, &item_id);
    return item_id;
}

// this would likely be better as an async function, queueing the locations to send on the update loop
RECOMP_EXPORT void rando_send_location(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_send_location,
        "recomp_data.last_location_sent = location_id\n"
        "check_func = recomp_data.ctx.check_locations([location_id])\n"
        "RecompClient.run_async_task_and_wait_once(check_func)\n"
    );

    // give self local items immediately (hack fix for incorrect item counts in text boxes?)
    // this appends the local item to the "received_item_ids" array, which is what we use to determine item count
    // > this works since "received_item_ids" gets reset every time items are recieved, but we might want a better solution in the future
    if (rando_get_location_has_local_item(location_id)) {
        REPY_FN_EXEC_CACHE(
            py_rando_send_location_local_instant,
            "recomp_data.ctx.received_item_ids.append(recomp_data.ctx.locations_info[location_id].item)"
        );
    }
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_complete_goal() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_complete_goal,
        "goal_func = recomp_data.ctx.complete_goal()\n"
        "RecompClient.run_async_task_and_wait_once(goal_func)\n"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT u32 rando_has_item(u32 item_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("item_id", item_id);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_has_item, 
        "recomp_data.ctx.received_item_ids.count(item_id)",
        item_count
    );
    REPY_FN_CLEANUP;
    return item_count;
}

// u32 rando_has_item_async(u32 item_id);

// this fully assumes that the player's slot id cannot be 0
u32 own_slot_id;
RECOMP_EXPORT u32 rando_get_own_slot_id() {
    if (!own_slot_id) {
        REPY_FN_SETUP_RANDO;
        
        REPY_FN_EVAL_CACHE_U32(
            py_rando_get_own_slot_id,
            "recomp_data.ctx.slot",
            slot
        );
        
        REPY_FN_CLEANUP;

        own_slot_id = slot;
    }
    
    return own_slot_id;
}

// might want a new system for this in the actual randomizers
RECOMP_EXPORT u32 rando_get_items_size() {
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
RECOMP_EXPORT u32 rando_get_item(u32 items_index) {
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
RECOMP_EXPORT s32 rando_get_item_location(u32 items_index) {
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
RECOMP_EXPORT u32 rando_get_sending_player(u32 items_index) {
    if (items_index > rando_get_items_size()) {
        return 0;
    }
    
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("index", items_index);
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_sending_player, 
        "recomp_data.ctx.items_received[index].player",
        sending_player
    );
    REPY_FN_CLEANUP;
    return sending_player;
}

// make this require a max string length
RECOMP_EXPORT void rando_get_item_name_from_id(u32 item_id, char** out_str) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("item_id", item_id);
    REPY_FN_EXEC_CACHE(
        py_rando_get_item_name_from_id,
        "item_name = recomp_data.ctx.item_names.lookup_in_game(item_id)\n"
        "item_name_len = len(item_name)\n" // temp?
    );
    (*out_str) = REPY_FN_GET_STR("item_name");
    REPY_FN_CLEANUP;
}

// make this require a max string length (also don't like how this is)
RECOMP_EXPORT void rando_get_sending_player_name(u32 items_index, char** out_str) {
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

// blank string helper function
void rando_set_blank_str(char** out_str) {
    // Allocating a buffer for the string on the heap.
    // Empty string, so the buffer only needs to be one byte.
    (*out_str) = recomp_alloc(sizeof(char) * 1);
    // Setting the only character in the string to be the null-terminator.
    (*out_str)[0] = 0;
}

// make this require a max string length
RECOMP_EXPORT void rando_get_location_item_player(u32 location_id, char** out_str) {
    if (!rando_location_exists(location_id)) {
        if (DEBUG_GLUE) {
            recomp_printf("location 0x%06X does not exist in %s\n", location_id, __func__);
        }
        rando_set_blank_str(out_str);
        return;
    }
    
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

RECOMP_EXPORT u32 rando_get_location_item_player_id(u32 location_id) {
    if (!rando_location_exists(location_id)) {
        if (DEBUG_GLUE) {
            recomp_printf("location 0x%06X does not exist in %s\n", location_id, __func__);
        }
        return 0;
    }

    u32 player_slot;
    recomputil_u32_value_hashmap_get(rando_location_player_map, location_id, &player_slot);
    return player_slot;
}

// make this require a max string length
RECOMP_EXPORT void rando_get_location_item_name(u32 location_id, char** out_str) {
    if (!rando_location_exists(location_id)) {
        if (DEBUG_GLUE) {
            recomp_printf("location 0x%06X does not exist in %s\n", location_id, __func__);
        }
        rando_set_blank_str(out_str);
        return;
    }
    
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location_id", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_get_location_item_name,
        "item_id = recomp_data.ctx.locations_info[location_id].item\n"
        "slot = recomp_data.ctx.locations_info[location_id].player\n"
        "item_name = recomp_data.ctx.item_names.lookup_in_slot(item_id, slot)"
    );
    (*out_str) = REPY_FN_GET_STR("item_name");
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT u32 rando_get_last_location_sent() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_U32(
        py_rando_get_last_location_sent,
        "recomp_data.last_location_sent\n",
        location_id
    );
    REPY_FN_CLEANUP;
    return location_id;
}

RECOMP_EXPORT u32 rando_get_seed_name(char** seed_name_out, u32 buffer_size) {
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
RECOMP_EXPORT void rando_get_own_slot_name(char** out_str) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_location_item_name,
        "name = recomp_data.ctx.player_names[recomp_data.ctx.slot]"
    );
    (*out_str) = REPY_FN_GET_STR("name");
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_get_saved_apconnect(u8* save_dir, char** address, char** player_name, char** password) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_saved_apconnect,
        "connection_info = rando_saves.get_ap_connect()\n"
        "address = connection_info[0]\n"
        "player_name = connection_info[1]\n"
        "password = connection_info[2]\n"
    );
    (*address) = REPY_FN_GET_STR("address");
    (*player_name) = REPY_FN_GET_STR("player_name");
    (*password) = REPY_FN_GET_STR("password");
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_set_saved_apconnect(u8* save_dir, char* address, char* player_name, char* password) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("address", address);
    REPY_FN_SET_STR("player_name", player_name);
    REPY_FN_SET_STR("password", password);
    REPY_FN_EXEC_CACHE(
        py_rando_set_saved_apconnect,
        "rando_saves.save_ap_connect(address, player_name, password)\n"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT bool rando_location_exists(u32 location_id) {
    return recomputil_u32_value_hashmap_contains(rando_location_item_map, location_id);
}

// SCOUTS

RECOMP_EXPORT void rando_queue_scout(u32 location) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location", location);
    REPY_FN_EXEC_CACHE(
        py_rando_queue_scout,
        "recomp_data.queued_scouts.add(location)"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_queue_scouts_all() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_queue_scouts_all,
        "recomp_data.queued_scouts = recomp_data.ctx.server_locations"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_remove_queued_scout(u32 location) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location", location);
    REPY_FN_EXEC_CACHE(
        py_rando_remove_queued_scout,
        "recomp_data.queued_scouts.discard(location)"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_send_queued_scouts(int hint) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("hint", hint);
    REPY_FN_EXEC_CACHE(
        py_rando_send_queued_scouts,
        "recomp_data.ctx.locations_scouted = recomp_data.queued_scouts\n"
        "msg_func = recomp_data.ctx.send_msgs([{\"cmd\": \"LocationScouts\",\n"
        "                                       \"locations\": list(recomp_data.queued_scouts),\n"
        "                                       \"create_as_hint\": hint}])\n"
        "RecompClient.run_async_task_and_wait_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_broadcast_location_hint(u32 location_id) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_U32("location", location_id);
    REPY_FN_EXEC_CACHE(
        py_rando_broadcast_location_hint,
        "msg_func = recomp_data.ctx.send_msgs([{\"cmd\": \"LocationScouts\",\n"
        "                                       \"locations\": [location],\n"
        "                                       \"create_as_hint\": True}])\n"
        "RecompClient.run_async_task_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

// SLOTDATA

RECOMP_EXPORT u32 rando_get_slotdata_u32(char* key) {
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

// DATASTORAGE

// datastorage functions will automatically add a prefix of "game_team_slot_value"

// copied from DKR, which copied from BT - i don't know why the values are turned into hex strings
RECOMP_EXPORT void rando_datastorage_replace_u32(char* key, u32 value) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("key", key);
    REPY_FN_SET_U32("value", value);
    REPY_FN_EXEC_CACHE(
        py_rando_datastorage_replace_u32,
        "game = recomp_data.ctx.game.replace(' ', '_')\n"
        "slot = recomp_data.ctx.slot\n"
        "team = recomp_data.ctx.team\n"
        "msg_func = recomp_data.ctx.send_msgs([{'cmd': 'Set',\n"
        "                                       'key': f'{game}_{team}_{slot}_{key}',\n"
        "                                       'default': hex(0),\n"
        "                                       'want_reply': False,\n"
        "                                       'operations': [{\n"
        "                                           'operation': 'replace',\n"
        "                                           'value': hex(value)}]\n"
        "                                       }])\n"
        "RecompClient.run_async_task_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_datastorage_replace_str(char* key, char* value) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("key", key);
    REPY_FN_SET_STR("value", value);
    REPY_FN_EXEC_CACHE(
        py_rando_datastorage_replace_str,
        "game = recomp_data.ctx.game.replace(' ', '_')\n"
        "slot = recomp_data.ctx.slot\n"
        "team = recomp_data.ctx.team\n"
        "msg_func = recomp_data.ctx.send_msgs([{'cmd': 'Set',\n"
        "                                       'key': f'{game}_{team}_{slot}_{key}',\n"
        "                                       'default': '',\n"
        "                                       'want_reply': False,\n"
        "                                       'operations': [{\n"
        "                                           'operation': 'replace',\n"
        "                                           'value': value}]\n"
        "                                       }])\n"
        "RecompClient.run_async_task_once(msg_func)\n"
    );
    REPY_FN_CLEANUP;
}

// DEATHLINK

RECOMP_EXPORT void rando_send_death_link() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_send_death_link,
        "deathlink_func = recomp_data.ctx.send_death()\n"
        "RecompClient.run_async_task_once(deathlink_func)\n"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT void rando_send_death_link_msg(char* death_msg) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_STR("death_msg", death_msg);
    REPY_FN_EXEC_CACHE(
        py_rando_send_death_link_msg,
        "deathlink_func = recomp_data.ctx.send_death(death_msg)\n"
        "RecompClient.run_async_task_once(deathlink_func)\n"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT bool rando_get_death_link_pending() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_get_death_link_pending,
        "recomp_data.ctx.deathlink_pending",
        pending
    );
    REPY_FN_CLEANUP;
    return pending;
}

RECOMP_EXPORT void rando_reset_death_link_pending() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_reset_death_link_pending,
        "recomp_data.ctx.deathlink_pending = False"
    );
    REPY_FN_CLEANUP;
}

RECOMP_EXPORT bool rando_get_death_link_enabled() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_get_death_link_pending,
        "'DeathLink' in recomp_data.ctx.tags",
        enabled
    );
    REPY_FN_CLEANUP;
    return enabled;
}

RECOMP_EXPORT void rando_toggle_death_link(bool toggle) {
    REPY_FN_SETUP_RANDO;
    REPY_FN_SET_BOOL("toggle", toggle);
    REPY_FN_EXEC_CACHE(
        py_rando_toggle_death_link,
        "deathlink_func = recomp_data.ctx.update_death_link(toggle)\n"
        "RecompClient.run_async_task_once(deathlink_func)\n"
    );
    REPY_FN_CLEANUP;
}

// this should hash the seed name + slot number into a random seed either the randomizer or other mods can use
RECOMP_EXPORT u32 rando_get_random_seed() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EXEC_CACHE(
        py_rando_get_random_seed,
        "import hashlib\n"
        "ctx = recomp_data.ctx\n"
        "hashed_seed = int(hashlib.sha256((f'{ctx.seed_name}{ctx.slot}').encode('utf-8')).hexdigest(), 16) % 0xFFFFFFFF\n"
    );
    u32 seed = REPY_FN_GET_U32("hashed_seed");
    REPY_FN_CLEANUP;
    return seed;
}

// everything below is mm specific slotdata stuff that will need to be altered in the mm repo
// bool rando_get_game_is_oot(u32 player_id);
// bool rando_get_game_is_ww(u32 player_id);