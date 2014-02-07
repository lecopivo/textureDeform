#include "shader.h"
#include <math.h>
#include <algorithm>

typedef struct {
  miColor		color;
} td_simple_color_t;


extern "C"
DLLEXPORT int td_simple_color_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_simple_color(
				    miColor *result,
				    miState *state,
				    td_simple_color_t *param)
{
  *result = *mi_eval_color(&param->color);

  return(miTRUE);
}


typedef struct {
  miScalar	side;
  miColor		color1;
  miColor		color2;
} td_color_checker_t;


extern "C"
DLLEXPORT int td_color_checker_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_color_checker(
				     miColor *result,
				     miState *state,
				     td_color_checker_t *param)
{
  miScalar side = *mi_eval_scalar(&param->side);

  int n = floor(state->point.x/side) +
    floor(state->point.y/side) +
    floor(state->point.z/side);

  if( n % 2 == 0 ){
    *result = *mi_eval_color(&param->color1);
  }else{
    *result = *mi_eval_color(&param->color2);
  }
	   
  return(miTRUE);
}

typedef struct {
  miScalar	side;
  miScalar	value1;
  miScalar	value2;
} td_scalar_checker_t;


extern "C"
DLLEXPORT int td_scalar_checker_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_scalar_checker(
				      miScalar *result,
				      miState *state,
				      td_scalar_checker_t *param)
{
  miScalar side = *mi_eval_scalar(&param->side);

  int n = (int)(state->point.x/side) +
    (int)(state->point.y/side) +
    (int)(state->point.z/side);

  if( n % 2 == 0 ){
    *result = *mi_eval_scalar(&param->value1);
  }else{
    *result = *mi_eval_scalar(&param->value2);
  }
	   
  return(miTRUE);
}

typedef struct {
  miScalar	side;
  miScalar	value;
} td_box_t;

extern "C"
DLLEXPORT int td_box_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_box(
			   miScalar *result,
			   miState *state,
			   td_box_t *param)
{
  miScalar side = *mi_eval_scalar(&param->side);

  miVector& p = state->point;

  side *= 0.5;

  if( p.x > -side && p.x < side &&
			   p.y > -side && p.y < side &&
						p.z > -side && p.z < side ){
    *result = *mi_eval_scalar(&param->value);
  }else{
    *result = 0;
  }

  return(miTRUE);
}


typedef struct {
  miScalar	radius;
  miScalar	center_value;
  miScalar	radius_value;
} td_radial_falloff_t;


extern "C"
DLLEXPORT int td_radial_falloff_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_radial_falloff(
				      miScalar *result,
				      miState *state,
				      td_radial_falloff_t *param)
{
  miScalar radius = *mi_eval_scalar(&param->radius);
  miScalar center_value = *mi_eval_scalar(&param->center_value);
  miScalar radius_value = *mi_eval_scalar(&param->radius_value);
  miScalar distance;

  //  mi_vector_to_world(state, &point, &state->point);
  distance = mi_vector_norm( &state->point);
  miScalar t = cos( std::min(  1.57075, 1.57075 * distance/radius) );
  *result = t * center_value + (1-t) * radius_value;

  return(miTRUE);
}


