#include "main.h"

U32ValueHashmapHandle rando_location_item_map;
U32ValueHashmapHandle rando_location_player_map;
U32ValueHashmapHandle rando_location_flag_map;
U32HashsetHandle rando_checked_locations;

void RandoGlue_Init(char* mod_id, char* ap_game_name) {
    // REPY_SetInterpreterAutoDisarm(rando_interp, true); // A hack fix until we have a proper shutdown event.
    
    REPY_FN_SETUP_INTERP(rando_interp);

    REPY_AddNrmToSysPath();

    // create a `recomp_data` module to store variables that can be used in other python code
    REPY_ConstructModuleFromCStr(
        "recomp_data",
        "ctx = None\n"
        "mod_data_path = None\n"
        "queued_scouts = set()\n"
        "queued_locations = set()\n" // unsure if this will actually get used
        "last_location_sent = 0\n",
        true
    );
    REPY_Handle data_module = REPY_ImportModule("recomp_data");
    
    // setup the `mod_data` path variable
    REPY_FN_SET_STR("mods_folder", (char*) recomp_get_mod_folder_path());
    REPY_FN_SET_STR("recomp_mod_id", mod_id);
    REPY_FN_SET_STR("ap_game_str", ap_game_name);

    REPY_FN_EXEC_CACHE( // maybe move this to a separate python file?
        rando_setup_filepath,
        "from pathlib import Path\n"
        "mod_data_path = Path(mods_folder).joinpath('..', 'mod_data', recomp_mod_id).resolve()\n"
        "mod_data_path.mkdir(parents=True, exist_ok=True)\n"
        "mod_data_path.joinpath('Archipelago', 'local', 'Players').mkdir(parents=True, exist_ok=True)\n" // need to make directories beforehand
    );

    // REPY_SetAttrCStr(data_module, "mod_data_path", REPY_MakeSUH(REPY_FN_GET("mod_data_path")));
    REPY_SetAttrCStr(data_module, "mod_data_path", REPY_MakeSUH(REPY_DictGetCStr(REPY_FN_LOCAL_SCOPE, "mod_data_path")));

    REPY_FN_EXEC_CACHE(
        rando_setup,
        "import RecompClient\n"
        "import Utils\n"
        "Utils.init_logging('RecompClient', exception_logger='Client')\n" // add condition for this to only appear for debugging?
        "RecompClient.run_async_task_once(RecompClient.setup_ctx(ap_game_str))\n"
    );

    // REPY_FN_EXEC_CACHE(
    //     rando_connect,
    //     "import RecompClient\n"
    //     "import recomp_data\n"
    //     "ctx = recomp_data.ctx\n"
    //     "ctx.server_address = 'archipelago://' + 'localhost:38281'\n"
    //     "ctx.auth = 'Hyped'\n"
    //     "ctx.password = ''\n"
    //     "RecompClient.connect_client()\n"
    // );

    rando_location_item_map = recomputil_create_u32_value_hashmap();
    rando_location_player_map = recomputil_create_u32_value_hashmap();
    rando_location_flag_map = recomputil_create_u32_value_hashmap();
    rando_checked_locations = recomputil_create_u32_hashset();
    
    REPY_FN_CLEANUP;
}

bool rando_init(char* address, char* player_name, char* password, char** error_msg) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_STR("ap_address", address);
    REPY_FN_SET_STR("ap_player_name", player_name);
    REPY_FN_SET_STR("ap_password", password);
    
    REPY_FN_EXEC_CACHE(
        rando_connect,
        "ctx = recomp_data.ctx\n"
        "ctx.server_address = 'archipelago://' + ap_address\n"
        "ctx.username = ap_player_name\n"
        "ctx.password = ap_password\n"
        // "RecompClient.save_ap_connect(ap_address, ap_player_name, ap_password)\n"
        "RecompClient.connect_client()\n" // crashes on failed connection due to archipelago raising an error
        "connected = RecompClient.wait_for_connection(5, 0.25)\n"
        "fail_msg = recomp_data.ctx.failed_reason\n" // only if connection failed
    );

    // TODO: write to fail_msg if its empty based on error as certain exceptions raise outside of where we currently check

    bool connected = REPY_FN_GET_BOOL("connected");

    if (!connected) {
        (*error_msg) = REPY_FN_GET_STR("fail_msg");
    }

    REPY_FN_CLEANUP;

    return connected;
}

bool randoDataInitialized;

// this needs to run after scouting is completed to have the info to store
void rando_populate_locations() {
    if (randoDataInitialized) return;

    REPY_FN_SETUP_RANDO;

    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_locations_ready,
        "recomp_data.ctx.locations_info != {}",
        is_ready
    );

    if (!is_ready) {
        REPY_FN_CLEANUP;
        return;
    }

    // note, an error occurs when the location is bigger than 32 bits
    REPY_FN_FOREACH_CACHE(py_rando_cache_locations, "location_info", "recomp_data.ctx.locations_info.items()") {
        REPY_FN_EXEC_CACHE(
            py_rando_cache_location_items,
            "location, network_item = location_info\n"
            "item = network_item.item & 0xFFFFFFFF\n" // hack fix for items being bigger than 32 bits (recomp items shouldn't be)
            "player = network_item.player\n"
            "flags = network_item.flags\n"
        );

        u32 location = REPY_FN_GET_U32("location");
        recomputil_u32_value_hashmap_insert(rando_location_item_map, location, REPY_FN_GET_U32("item"));
        recomputil_u32_value_hashmap_insert(rando_location_player_map, location, REPY_FN_GET_U32("player"));
        recomputil_u32_value_hashmap_insert(rando_location_flag_map, location, REPY_FN_GET_U32("flags"));
    }

    REPY_FN_CLEANUP;

    randoDataInitialized = true;
}

// TODO: combine above with this
void rando_update_cache() {
    REPY_FN_SETUP_RANDO;

    REPY_FN_EVAL_CACHE_BOOL(
        py_rando_needs_update,
        "recomp_data.ctx.recomp_needs_updating",
        should_update
    );

    if (!should_update) {
        REPY_FN_CLEANUP;
        return;
    }

    // this assumes both the python and hashset implementations aren't bottlenecked by refilling these
    REPY_FN_FOREACH_CACHE(py_rando_cache_locations, "location_checked", "recomp_data.ctx.local_checked") {
        recomputil_u32_hashset_insert(rando_checked_locations, REPY_FN_GET_U32("location_checked"));
    }

    REPY_FN_EXEC_CACHE(
        py_rando_no_longer_needs_update,
        "recomp_data.ctx.recomp_needs_updating = False"
    );

    REPY_FN_CLEANUP;
}

// should only run when the game is saved
void rando_save_current_state() {
    REPY_FN_SETUP_RANDO;

    REPY_FN_EXEC_CACHE(
        py_rando_save_current_state,
        "RecompClient.save_current_state()"
    );

    REPY_FN_CLEANUP;
}