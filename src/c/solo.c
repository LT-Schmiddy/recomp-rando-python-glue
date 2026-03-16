#include "main.h"

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