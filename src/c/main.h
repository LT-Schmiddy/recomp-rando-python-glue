#ifndef __RANDO_GLUE_MAIN__
#define __RANDO_GLUE_MAIN__
#include "repy_api.h"
#include "recompdata.h"

#include "PR/ultratypes.h"
#define bool    u32

#define false 0
#define true 1

#define DEBUG_GLUE false

REPY_EXTERN_SUBINTERPRETER(rando_interp);

#define REPY_FN_SETUP_RANDO \
    REPY_FN_SETUP_INTERP(rando_interp); \
    REPY_FN_IMPORT("recomp_data"); \
    REPY_FN_IMPORT("RecompClient"); \
    REPY_FN_IMPORT("rando_saves");

#endif