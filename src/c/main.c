#include "main.h"

REPY_InterpreterIndex rando_interp = 0;
REPY_Handle rando_globals = 0;

void RandoGlue_Init(char* mod_id, char* ap_game_name) { 
    rando_interp = REPY_RegisterSubinterpreter(); 
    REPY_SetInterpreterAutoDisarm(rando_interp, 1); // A hack fix until we have a proper shutdown event.
    REPY_PushInterpreter(rando_interp);
    REPY_AddNrmToSysPath();
    rando_globals = REPY_CreateDict(0);
    REPY_PopInterpreter();

    recomp_printf("Subinterpreter %s Initialized\n", "rando_interp");

    // REPY_FN_SETUP_INTERP(rando_interp);
    REPY_FN_SETUP_RANDO;

    // create a `recomp_data` module to store variables that can be used in other python code
    REPY_ConstructModuleFromCStr(
        "recomp_data",
        "ctx = None\n"
        "mod_data_path = None\n",
        1
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
        "print(mod_data_path)"
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
    
    REPY_FN_CLEANUP;
}

// the "py" part is temporary until the old glue is fully replaced
void py_rando_init(char* address, char* player_name, char* password) {
    REPY_FN_SETUP_RANDO;

    REPY_FN_SET_STR("ap_address", address);
    REPY_FN_SET_STR("ap_player_name", player_name);
    REPY_FN_SET_STR("ap_password", password);
    
    REPY_FN_EXEC_CACHE(
        rando_connect,
        "import RecompClient\n"
        "import recomp_data\n"
        "ctx = recomp_data.ctx\n"
        "ctx.server_address = 'archipelago://' + ap_address\n"
        "ctx.username = ap_player_name\n"
        "ctx.password = ap_password\n"
        "RecompClient.save_ap_connect(ap_address, ap_player_name, ap_password)\n"
        "RecompClient.connect_client()\n"
    );

    REPY_FN_CLEANUP;
}