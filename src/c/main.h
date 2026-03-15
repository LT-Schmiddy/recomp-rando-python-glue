#ifndef __RANDO_GLUE_MAIN__
#define __RANDO_GLUE_MAIN__
#include "repy_api.h"

REPY_EXTERN_SUBINTERPRETER(rando_interp);

#define REPY_FN_SETUP_RANDO \
    REPY_FN_SETUP_INTERP(rando_interp); \
    REPY_FN_IMPORT("recomp_data");

#endif