#include "shader.h"

struct td_scalar_kernel {
  miVector point;
  miScalar value;
};

extern "C"
DLLEXPORT
int td_scalar_kernel_version(void){return 1;}

extern "C"
DLLEXPORT
miBoolean td_scalar_kernel ( miScalar *result, miState *state, struct td_scalar_kernel *params ){

  miVector newPoint = *mi_eval_vector( &params->point );

  state->point = newPoint;

  *result = *mi_eval_scalar( &params->value );
  
  return miTRUE;
}

struct td_color_kernel {
  miVector point;
  miColor color;
};

extern "C"
DLLEXPORT
int td_color_kernel_version(void){return 1;}

extern "C"
DLLEXPORT
miBoolean td_color_kernel ( miColor *result, miState *state, struct td_color_kernel *params ){
  
  miVector newPoint = *mi_eval_vector( &params->point );

  state->point = newPoint;

  *result = *mi_eval_color( &params->color );

  return miTRUE;
}

