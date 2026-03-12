#include "repy_api.h"

REPY_EXTERN_SUBINTERPRETER(rando_interp);

void RandoGlue_Init(char* mod_id, char* ap_game_name);
void py_rando_init(char* address, char* player_name, char* password);