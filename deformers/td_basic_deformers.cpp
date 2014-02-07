#include "shader.h"
#include <cmath>
#include <algorithm>

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

// Affine transformation
// This is example how the transformation shader has to be done

typedef struct {
  miVector      point;
  miMatrix	matrix;
  miMatrix	inverseMatrix;
} td_affine_t;


extern "C"
DLLEXPORT int td_affine_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_affine(
			      miVector *result,
			      miState *state,
			      td_affine_t *param)
{
  // Here I will mark important steps of the shader. They must be preformed in this order.
  
  // load parameters
  miScalar * inverseMatrix = mi_eval_transform(&param->inverseMatrix);
  miVector newPoint;

  // #1 transform state->point to newPoint
  // the transformation can ofcourse look different than this
  mi_point_transform( &newPoint, &state->point, inverseMatrix );

  // #2 set state->point to newPoint
  state->point = newPoint;

  // #3 now evaluate next deformation with new state->point
  *result = *mi_eval_vector( &param->point );

  return(miTRUE);
}

typedef struct {
  miScalar	lowBound;
  miScalar	highBound;
  miScalar	startFlareX;
  miScalar	startFlareZ;
  miScalar	endFlareX;
  miScalar	endFlareZ;
  miScalar	curve;
  miVector      point;
  miMatrix	matrix;
  miMatrix	inverseMatrix;
} td_flare_t;


extern "C"
DLLEXPORT int td_flare_version(void) {return(1);}


/*! Transformation(shader has to implement iverse of this)
 *  x' = x*(startFlareX*0.5*(1-y) + endFlareX*0.5*(1+y))*(1+curve*cos(y*pi/2))
 *  y' = y
 *  z' = z*(startFlareZ*0.5*(1-y) + endFlareZ*0.5*(1+y))*(1+curve*cos(y*pi/2))
 *
 *  this formula works only for -1<y<1, and lowBound=-1, highBound=1
 */
extern "C"
DLLEXPORT miBoolean td_flare(
			     miVector *result,
			     miState *state,
			     td_flare_t *param)
{
  miScalar lowBound = *mi_eval_scalar(&param->lowBound);
  miScalar highBound = *mi_eval_scalar(&param->highBound);
  miScalar startFlareX = *mi_eval_scalar(&param->startFlareX);
  miScalar startFlareZ = *mi_eval_scalar(&param->startFlareZ);
  miScalar endFlareX = *mi_eval_scalar(&param->endFlareX);
  miScalar endFlareZ = *mi_eval_scalar(&param->endFlareZ);
  miScalar curve = *mi_eval_scalar(&param->curve);
  miScalar *matrix = mi_eval_transform(&param->matrix);
  miScalar *inverseMatrix = mi_eval_transform(&param->inverseMatrix);
  miVector p,dp;

  mi_point_transform( &p, &state->point, inverseMatrix );

  miScalar ty = (2*p.y - (highBound+lowBound) )/(highBound-lowBound);

  ty = std::max( std::min( 1.0, (double)ty ), -1.0 );

  dp.x = p.x/( ( 0.5*startFlareX*(1-ty) + 0.5*endFlareX*(1+ty)) *
	       (1 + curve*cos(ty*M_PI/2) ) );
  dp.y = p.y;
  dp.z = p.z/( ( 0.5*startFlareZ*(1-ty) + 0.5*endFlareZ*(1+ty)) *
	       (1 + curve*cos(ty*M_PI/2) ) );

  mi_point_transform( &p, &dp, matrix );

  state->point = p;

  *result = *mi_eval_vector( &param->point );

  return(miTRUE);
}


typedef struct {
  miScalar	lowBound;
  miScalar	highBound;
  miScalar	startAngle;
  miScalar	endAngle;
  miVector	point;
  miMatrix	matrix;
  miMatrix	inverseMatrix;
} td_twist_t;


extern "C"
DLLEXPORT int td_twist_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_twist(
			     miVector *result,
			     miState *state,
			     td_twist_t *param)
{
  miScalar lowBound = *mi_eval_scalar(&param->lowBound);
  miScalar highBound = *mi_eval_scalar(&param->highBound);
  miScalar startAngle = *mi_eval_scalar(&param->startAngle);
  miScalar endAngle = *mi_eval_scalar(&param->endAngle);
  miScalar *matrix = mi_eval_transform(&param->matrix);
  miScalar *inverseMatrix = mi_eval_transform(&param->inverseMatrix);
  miVector p,dp;

  mi_point_transform( &p, &state->point, inverseMatrix );

  miScalar ty = (2*p.y - (highBound+lowBound) )/(highBound-lowBound);

  ty = std::max( std::min( 1.0, (double)ty ), -1.0 );

  double angle = endAngle*0.5*(1+ty) + startAngle*0.5*(1-ty);
  double cosA = cos(angle);
  double sinA = sin(angle);
  dp.x = p.x*cosA - p.z*sinA;
  dp.y = p.y;
  dp.z = p.x*sinA + p.z*cosA;

  mi_point_transform( &p, &dp, matrix );

  state->point = p;

  *result = *mi_eval_vector( &param->point );

  return(miTRUE);
}

typedef struct {
  miScalar	lowBound;
  miScalar	highBound;
  miScalar	amplitude;
  miScalar	wavelenght;
  miScalar	offset;
  miScalar	dropoff;
  miVector	point;
  miMatrix	matrix;
  miMatrix	inverseMatrix;
} td_sine_t;


extern "C"
DLLEXPORT int td_sine_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_sine(
			    miVector *result,
			    miState *state,
			    td_sine_t *param)
{
  miScalar lowBound = *mi_eval_scalar(&param->lowBound);
  miScalar highBound = *mi_eval_scalar(&param->highBound);
  miScalar amplitude = *mi_eval_scalar(&param->amplitude);
  miScalar wavelenght = *mi_eval_scalar(&param->wavelenght);
  miScalar offset = *mi_eval_scalar(&param->offset);
  miScalar dropoff = *mi_eval_scalar(&param->dropoff);
  miScalar *matrix = mi_eval_transform(&param->matrix);
  miScalar *inverseMatrix = mi_eval_transform(&param->inverseMatrix);
  miVector p,dp;

  mi_point_transform( &p, &state->point, inverseMatrix );

  miScalar ty = (2*p.y - (highBound+lowBound) )/(highBound-lowBound);

  ty = std::max( std::min( 1.0, (double)ty ), -1.0 );

  dp.x = p.x - amplitude*sin(wavelenght*M_PI*0.5*(ty+offset)) * ((1-dropoff)+dropoff*(1-abs(ty)));
  dp.y = p.y;
  dp.z = p.z;

  mi_point_transform( &p, &dp, matrix );

  state->point = p;

  *result = *mi_eval_vector( &param->point );
 
  return(miTRUE);
}


typedef struct {
  miScalar	lowBound;
  miScalar	highBound;
  miScalar	curvature;
  miVector	point;
  miMatrix	matrix;
  miMatrix	inverseMatrix;
} td_bend_t;


extern "C"
DLLEXPORT int td_bend_version(void) {return(1);}

extern "C"
DLLEXPORT miBoolean td_bend(
			    miVector *result,
			    miState *state,
			    td_bend_t *param)
{

  // TADY JE TO PORAD NEJAKE ZABUGOVANE
  miScalar lowBound = *mi_eval_scalar(&param->lowBound);
  miScalar highBound = *mi_eval_scalar(&param->highBound);
  miScalar curvature = *mi_eval_scalar(&param->curvature);
  miScalar *matrix = mi_eval_transform(&param->matrix);
  miScalar *inverseMatrix = mi_eval_transform(&param->inverseMatrix);
  miVector p,dp;

  mi_point_transform( &p, &state->point, inverseMatrix );

  miScalar highAngle = curvature/highBound;
  miScalar lowAngle  = curvature/lowBound;

  miScalar r = sqrt( pow(p.x-curvature,2) + pow(p.y,2) );
  miScalar theta = atan2( p.y, p.x-curvature );
  
  if(theta<=highAngle && theta>=lowAngle){
    dp.x = r - curvature;
    dp.y = theta*curvature;
    dp.z = p.z;
  }
  if(theta>highAngle){
    dp.x = r*cos(theta-highAngle) - curvature;
    dp.y = r*sin(theta-highAngle) + highAngle*curvature;
    dp.z = p.z;
  }
  if(theta<lowAngle){
    dp.x = r*cos(theta-lowAngle) - curvature;
    dp.y = r*sin(theta-lowAngle) + lowAngle*curvature;
    dp.z = p.z;
  }

  mi_point_transform( &p, &dp, matrix );

  state->point = p;

  *result = *mi_eval_vector( &param->point );

  return(miTRUE);
}
