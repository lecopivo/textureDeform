#include "shader.h"
#include "miaux.h"

struct td_scalar_kernel {
  miColor color;
  miTag density_shader;
  miTag deform_shader;
  miScalar unit_density;
  miScalar march_increment;
  int     i_light;
  int     n_light; 
  miTag   light[1];
};

extern "C"
DLLEXPORT
int td_scalar_kernel_version(void) { return 1; }

extern "C"
DLLEXPORT
miBoolean td_scalar_kernel ( 
		     miColor *result, miState *state, struct td_scalar_kernel *params  )
{
  miScalar unit_density, march_increment, density;
  miTag density_shader, deform_shader,*light;
  int light_count;

  if (state->type == miRAY_LIGHT)
    return miTRUE;

  density_shader = *mi_eval_tag(&params->density_shader);
  deform_shader  = *mi_eval_tag(&params->deform_shader);
  unit_density = *mi_eval_scalar(&params->unit_density);
  march_increment = *mi_eval_scalar(&params->march_increment);
  miaux_light_array(&light, &light_count, state,
		    &params->i_light, &params->n_light, params->light);

  if (state->type == miRAY_SHADOW) {
    miScalar occlusion = miaux_fractional_shader_occlusion_at_point (
								     state, &state->org, &state->dir, state->dist,
								     density_shader, deform_shader, unit_density, march_increment);
    miaux_scale_color(result, 1.0 - occlusion);
  } else {
    miColor *color = mi_eval_color(&params->color);
    miScalar distance;
    miColor volume_color = {0,0,0,0}, light_color, point_color;
    miVector original_point = state->point;
    miRc_intersection* original_state_pri = state->pri;
    state->pri = NULL;

    for (distance = 0; distance <= state->dist; distance += march_increment) {
      miVector march_point,deformed_point;
      miaux_march_point(&march_point, state, distance);
      state->point = march_point;
      mi_call_shader_x((miColor*)&deformed_point, miSHADER_MATERIAL, state, deform_shader, NULL);
      state->point = deformed_point;
      mi_call_shader_x((miColor*)&density, 
		       miSHADER_MATERIAL, state, density_shader, NULL);
      if (density > 0) {
	density *= unit_density * march_increment;
	miaux_total_light_at_point(
				   &light_color, &deformed_point, state, light, light_count); 
	miaux_multiply_colors(&point_color, color, &light_color);
	miaux_add_transparent_color(&volume_color, &point_color, density);
      }
      if (volume_color.a == 1.0)
	break;
    }
    miaux_alpha_blend_colors(result, &volume_color, result);
    state->point = original_point;
    state->pri = original_state_pri;
  }
  return miTRUE;
}


struct td_world_coord{
};

extern "C"
DLLEXPORT
int td_world_coord_version(void){return 1;}

extern "C"
DLLEXPORT
miBoolean td_world_coord(miVector *result, miState *state, struct td_world_coord *params)
{
  *result = state->point;
  return miTRUE;
}

extern "C"
DLLEXPORT
int td_gray_version(void) { return 1; }

struct td_gray {
    miScalar value;
};

extern "C"
DLLEXPORT
miBoolean td_gray ( 
    miScalar *result, miState *state, struct td_gray *params  )
{
    *result = *mi_eval_scalar(&params->value);

    return miTRUE;
}
