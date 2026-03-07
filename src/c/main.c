#include "main.h"

REPY_InterpreterIndex rando_interp = 0;
REPY_Handle rando_globals = 0;

void RandoGlue_Init() { 
    rando_interp = REPY_RegisterSubinterpreter(); 
    
    REPY_PushInterpreter(rando_interp);
    REPY_AddNrmToSysPath();
    rando_globals = REPY_CreateDict(0);
    REPY_PopInterpreter();

    recomp_printf("Subinterpreter %s Initialized\n", "rando_interp");

    // REPY_FN_SETUP_INTERP(rando_interp);
    REPY_FN_SETUP_RANDO;

    REPY_FN_EXEC_CACHE(
        example1, 
        "from rando_test import test_func\n"
        "test_func()\n"
        "from CommonClient import run_as_textclient\n"
    );
    
    REPY_FN_CLEANUP;
}