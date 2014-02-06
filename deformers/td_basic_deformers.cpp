#include "shader.h"

// Identity deformer

extern "C"
DLLEXPORT
int td_identity_version(void){return 1;}


struct td_identity{
};

extern "C"
DLLEXPORT
miBoolean td_identity(miVector *result, miState *state, struct td_identity *params)
{
  *result = state->point;
  return miTRUE;
}

// 

