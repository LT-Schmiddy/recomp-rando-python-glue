#include "main.h"

// initialize all needed features for solo randomization to work
// "apworld_folder" is the name of the folder encompassing the apworld
// used to properly import the apworld from the "worlds" folder for archipelago
// for example, for Majora's Mask Recompiled this is just "mm_recomp" rather than "apworld\mm_recomp"
void RandoGlue_InitSolo(char* apworld_folder) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_STR("apworld_folder", apworld_folder);

    REPY_FN_EXEC_CACHE(
        py_rando_import_apworld,
        "import importlib\n"
        "importlib.import_module(f'worlds.{apworld_folder}')\n"
    );

    REPY_FN_IMPORT("rando_solo");

    REPY_FN_EXEC_CACHE(
        py_rando_initialize_solo,
        "rando_solo.populate_options()\n" // populate options for the solo menu
    );

    // just for testing, immediately generates and hosts a seed on initialization
    // REPY_FN_EXEC_CACHE(
    //     rando_test_gen,
    //     "rando_solo.generate()\n"
    //     "rando_solo.host()\n"
    // );

    REPY_FN_CLEANUP;
}

// maybe move solo_seeds to this file?

void rando_scan_solo_seeds() {
    REPY_FN_SETUP_RANDO;

    REPY_FN_EXEC_CACHE(
        py_rando_scan_solo_seeds,
        // this is dumb
        "def get_creation_time(file):\n"
        "   return file.stat().st_ctime\n"
        "output_folder = recomp_data.mod_data_path.joinpath('Archipelago', 'local', 'output')\n"
        "recomp_data.solo_seeds = sorted(list(output_folder.glob('*.zip')), key=get_creation_time, reverse=True)\n"
    );

    REPY_FN_CLEANUP;
}

u32 rando_solo_count() {
    REPY_FN_SETUP_RANDO;
    REPY_FN_EVAL_CACHE_U32(
        py_rando_solo_count,
        "len(recomp_data.solo_seeds)",
        seed_count
    );
    REPY_FN_CLEANUP;
    return seed_count;
}

void rando_solo_get_seed_name(u32 seed_index, char** out_str) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_U32("seed_index", seed_index);
    
    REPY_FN_EXEC_CACHE(
        py_rando_solo_get_seed_name,
        "name = recomp_data.solo_seeds[seed_index].stem" // just getting the file's name without extension
    );
    
    (*out_str) = REPY_FN_GET_STR("name");
    
    REPY_FN_CLEANUP;
}

void rando_solo_get_generation_date(u32 seed_index, char** out_str) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_U32("seed_index", seed_index);
    
    REPY_FN_EXEC_CACHE(
        py_rando_solo_get_generation_date,
        "from datetime import datetime\n"
        "creation_date = datetime.fromtimestamp(recomp_data.solo_seeds[seed_index].stat().st_birthtime)\n"
        "date = creation_date.strftime('%b %d %Y %H:%M %p')\n"
    );
    
    (*out_str) = REPY_FN_GET_STR("date");

    REPY_FN_CLEANUP;
}

u32 rando_init_solo(u32 seed_index, char** error_msg) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_U32("seed_index", seed_index);
    REPY_FN_IMPORT("rando_solo");
    
    REPY_FN_EXEC_CACHE(
        py_rando_handle_solo_init,
        "selected_seed = recomp_data.solo_seeds[seed_index]\n"
        "host_success = rando_solo.host(str(selected_seed))\n"
        "fail_msg = recomp_data.error_msg\n"
    );

    bool host_success = REPY_FN_GET_BOOL("host_success");

    if (!host_success) {
        (*error_msg) = REPY_FN_GET_STR("fail_msg");
        REPY_FN_CLEANUP;
        return false;
    }

    REPY_FN_EXEC_CACHE(
        py_rando_get_player_info,
        // assuming first key in "connect_names" is our player (and no other players exist, because they could if someone was evil)
        "player = next(iter(recomp_data.server_ctx.connect_names))\n"
        "address = f'localhost:{recomp_data.server_ctx.port}'\n"
    );

    char* player = REPY_FN_GET_STR("player");
    char* address = REPY_FN_GET_STR("address");

    bool success = rando_init(address, player, "", error_msg);

    recomp_free(player);
    recomp_free(address);

    if (!success) {
        // the client may connect at some point past the timeout, but let's just play this on the safe side
        REPY_FN_EXEC_CACHE(
            py_rando_shutdown_solo_server_on_fail,
            "recomp_data.server_ctx.commandprocessor._cmd_exit()" // probably the shutdown function?
        );
    }

    REPY_FN_CLEANUP;
    return success;
}

// temporarily all empty until solo is properly figured out and handled

void rando_yaml_init() {}
void rando_yaml_puts(const char* text, u32 size) {}
void rando_yaml_finalize(const unsigned char* save_path) {}
bool rando_solo_generate() {return 0;}