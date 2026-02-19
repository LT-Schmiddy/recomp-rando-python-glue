#include "main.h"

REPY_REGISTER_SUBINTERPRETER(rando_interp);

REPY_ON_POST_INIT void example_function() {
    REPY_FN_SETUP_INTERP(rando_interp);
    REPY_FN_EXEC_CACHE(
        example1, 
        "import platform\n"
        "print(f'This is example Python code running on {platform.system()}')"
    );
    REPY_FN_CLEANUP;
}