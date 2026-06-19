#include "main.h"

// initialize all needed features for solo randomization to work
// "apworld_folder" is the name of the folder encompassing the apworld
// used to properly import the apworld from the "worlds" folder for archipelago
// for example, for Majora's Mask Recompiled this is just "mm_recomp" rather than "apworld\mm_recomp"
void RandoGlue_InitSolo(char* apworld_folder) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_STR("apworld_folder", apworld_folder);

    REPY_FN_EXEC_CACHE(
        rando_import_apworld,
        "import importlib\n"
        "importlib.import_module(f'worlds.{apworld_folder}')\n"
    );

    REPY_FN_IMPORT("rando_solo");

    REPY_FN_EXEC_CACHE(
        rando_initialize_solo,
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

// temporarily all empty until solo is properly figured out and handled

void rando_scan_solo_seeds(const unsigned char* save_filename) {}
u32 rando_solo_count() {return 0;}
u32 rando_solo_get_seed_name(u32 seed_index, char* out, u32 max_length) {return 0;} // Returns the actual string length
u32 rando_solo_get_generation_date(u32 seed_index, char* out, u32 max_length) {return 0;} // Returns the actual string length
u32 rando_init_solo(u32 seed_index) {return 0;}

void rando_yaml_init() {}
void rando_yaml_puts(const char* text, u32 size) {}
void rando_yaml_finalize(const unsigned char* save_path) {}
bool rando_solo_generate() {return 0;}