#include "main.h"

REPY_InterpreterIndex rando_interp = 0;
REPY_Handle rando_globals = 0;

void RandoGlue_Init() { 
    rando_interp = REPY_RegisterSubinterpreter(); 
    
    REPY_PushInterpreter(rando_interp);
    rando_globals = REPY_CreateDict(0);
    REPY_AddNrmToSysPath();
    REPY_PopInterpreter();

    recomp_printf("Subinterpreter %s Initialized\n", "rando_interp");

    // REPY_FN_SETUP_INTERP(rando_interp);
    REPY_FN_SETUP_RANDO;

    // test platform code to confirm that python is functioning correctly
    REPY_FN_EXEC_CACHE(
        example1, 
        "import platform\n"
        "print(f'This is example Python code running on {platform.system()}')"
    );
    
    REPY_FN_CLEANUP;
}