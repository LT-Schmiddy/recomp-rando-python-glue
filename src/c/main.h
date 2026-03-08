#ifndef __RANDO_GLUE_MAIN__
#define __RANDO_GLUE_MAIN__
#include "repy_api.h"

extern REPY_InterpreterIndex rando_interp;
extern REPY_Handle rando_globals;

#define REPY_FN_SETUP_RANDO \
REPY_FN_SETUP_INTERP_WITH_GLOBALS(rando_interp, rando_globals);

#endif