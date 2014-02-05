/*
   "Writing mental ray shaders" -- Utility functions used in shaders
   See http://www.writingshaders.com/ for the current software distribution
*/

#include "miaux.h"


/* Chapter 7 -- Color from position ----------------------------------------- */

/* Shader: depth_fade_tint_2 */

double miaux_fit(
    double v, double oldmin, double oldmax, double newmin, double newmax)    
{
    return newmin + ((v - oldmin) / (oldmax - oldmin)) * (newmax - newmin);
}

double miaux_blend(miScalar a, miScalar b, miScalar factor)
{
    return a * factor + b * (1.0 - factor);
}

void miaux_blend_colors(miColor *result, 
			miColor *color1, miColor *color2, miScalar factor)
{
    result->r = miaux_blend(color1->r, color2->r, factor);
    result->g = miaux_blend(color1->g, color2->g, factor);
    result->b = miaux_blend(color1->b, color2->b, factor);
}


/* Chapter 8 -- The transparency of a surface ------------------------------- */

/* Shader: transparent_modularized */

void miaux_blend_channels(miColor *result,
			  miColor *blend_color, miColor *blend_fraction)
{
    result->r = miaux_blend(result->r, blend_color->r, blend_fraction->r);
    result->g = miaux_blend(result->g, blend_color->g, blend_fraction->g);
    result->b = miaux_blend(result->b, blend_color->b, blend_fraction->b);
}

void miaux_invert_channels(miColor *result, miColor *color)
{
    result->r = 1.0 - color->r;
    result->g = 1.0 - color->g;
    result->b = 1.0 - color->b;
    result->a = 1.0 - color->a;
}

miBoolean miaux_all_channels_equal(miColor *c, miScalar v)
{
    if (c->r == v && c->g == v && c->b == v && c->a == v)
	return miTRUE;
    else
	return miFALSE;
}


/* Chapter 9 -- Color from functions ---------------------------------------- */

/* Shader: summed_noise_color */

void miaux_scale_vector(miVector *result, miScalar scale)
{
    result->x *= scale;
    result->y *= scale;
    result->z *= scale;
}

void miaux_set_vector(miVector *v, double x, double y, double z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

double miaux_summed_noise ( 
    miVector *point,
    double summing_weight, double octave_scaling, int octave_count)
{
    int i;
    double noise_value,
	noise_sum = 0.0, noise_scale = 1.0, maximum_noise_sum = 0.0;
    miVector scaled_point;
    miaux_set_vector(&scaled_point, point->x, point->y, point->z);

    for (i = 0; i < octave_count; i++) {
	noise_value = mi_unoise_3d(&scaled_point);
	noise_sum += noise_value / noise_scale;
	maximum_noise_sum += 1.0 / noise_scale;
	noise_scale *= summing_weight;
	miaux_scale_vector(&scaled_point, octave_scaling);
    }
    return noise_sum/maximum_noise_sum;
}


/* Chapter 10 -- The color of edges ----------------------------------------- */

/* Shader: c_output */

char* miaux_tag_to_string(miTag tag, char *default_value)
{
    char *result = default_value;
    if (tag != 0) {
	result = (char*)mi_db_access(tag);
	mi_db_unpin(tag);
    }
    return result;
}


/* Shader: show_barycentric */

void miaux_add_scaled_color(miColor *result, miColor *color, miScalar scale)
{
    result->r += color->r * scale;
    result->g += color->g * scale;
    result->b += color->b * scale;
}


/* Shader: front_bright_steps */

miScalar miaux_quantize(miScalar value, miInteger count)
{
    miScalar q = (miScalar)count;
    if (count < 2)
	return q;
    else
	return (miScalar)((int)(value * q) / (q - 1));
}


/* Shader: lambert_steps */

void miaux_add_diffuse_component(
    miColor *result, 
    miScalar light_and_surface_cosine, 
    miColor *diffuse, miColor *light_color)
{
    result->r += light_and_surface_cosine * diffuse->r * light_color->r;
    result->g += light_and_surface_cosine * diffuse->g * light_color->g;
    result->b += light_and_surface_cosine * diffuse->b * light_color->b;
}

void miaux_set_channels(miColor *c, miScalar new_value)
{
    c->r = c->g = c->b = c->a = new_value;
}

void miaux_light_array(miTag **lights, int *light_count, miState *state,
		       int *offset_param, int *count_param, miTag *lights_param)
{
    int array_offset = *mi_eval_integer(offset_param);
    *light_count = *mi_eval_integer(count_param);
    *lights = mi_eval_tag(lights_param) + array_offset;
}


/* Chapter 11 -- Lights ----------------------------------------------------- */

/* Shader: spotlight */

miScalar miaux_light_spread(miState *state, miTag light_tag)
{
    miScalar light_spread;
    mi_query(miQ_LIGHT_SPREAD, state, light_tag, &light_spread);
    return light_spread;
}

miScalar miaux_offset_spread_from_light(miState *state, miTag light_tag)
{
    miVector light_direction, light_to_sample_point;

    mi_query(miQ_LIGHT_DIRECTION, state, light_tag, &light_direction);
    mi_vector_normalize(&light_direction);

    mi_vector_to_light(state, &light_to_sample_point, &state->dir);
    mi_vector_normalize(&light_to_sample_point); 

    return mi_vector_dot(&light_to_sample_point, &light_direction);
}

miTag miaux_current_light_tag(miState *state)
{
    miTag light_tag;
    mi_query(miQ_INST_ITEM, state, state->light_instance, &light_tag);
    return light_tag;
}


/* Shader: soft_spotlight */

void miaux_scale_color(miColor *result, miScalar scale)
{
    result->r *= scale;
    result->g *= scale;
    result->b *= scale;
}


/* Shader: sinusoid_soft_spotlight */

double miaux_sinusoid_fit(
    double v, double oldmin, double oldmax, double newmin, double newmax)
{
    return miaux_fit(sin(miaux_fit(v, oldmin, oldmax, -M_PI_2, M_PI_2)), 
		     -1, 1, 
		     newmin, newmax);
}


/* Chapter 12 -- Light on a surface ----------------------------------------- */

/* Shader: phong */

void miaux_add_phong_specular_component(
    miColor *result, miState *state, miScalar exponent,
    miVector *direction_toward_light, 
    miColor *specular, miColor *light_color)
{
    miScalar specular_amount =
	mi_phong_specular(exponent, state, direction_toward_light);
    if (specular_amount > 0.0) {
	result->r += specular_amount * specular->r * light_color->r;
	result->g += specular_amount * specular->g * light_color->g;
	result->b += specular_amount * specular->b * light_color->b;
    }
}


/* Shader: blinn */

void miaux_add_blinn_specular_component(
    miColor *result, miState *state,
    miScalar roughness, miScalar ior,
    miVector direction_toward_light,
    miColor *specular, miColor *light_color)
{
    miScalar specular_amount = 
	mi_blinn_specular(&state->dir, &direction_toward_light,
			  &state->normal, roughness, ior);
    if (specular_amount > 0.0) {
	result->r += specular_amount * specular->r * light_color->r;
	result->g += specular_amount * specular->g * light_color->g;
	result->b += specular_amount * specular->b * light_color->b;
    }
}


/* Shader: cook_torrance */

void miaux_add_cook_torrance_specular_component(
    miColor *result, miState *state,
    miScalar roughness, miColor *ior,
    miVector direction_toward_light,
    miColor *specular, miColor *light_color)
{
    miColor specular_reflection_color;
    if (mi_cooktorr_specular(&specular_reflection_color, &state->dir, 
			     &direction_toward_light,
			     &state->normal, roughness, ior)) {
	result->r += specular_reflection_color.r * specular->r * light_color->r;
	result->g += specular_reflection_color.g * specular->g * light_color->g;
	result->b += specular_reflection_color.b * specular->b * light_color->b;
    }
}


/* Shader: ward */

void miaux_add_ward_specular_component(
    miColor *result, miState *state,
    miScalar shiny_u, miScalar shiny_v,
    miColor *glossy, miScalar normal_dot_light,
    miVector direction_toward_light,
    miColor *light_color)
{
    miScalar specular_reflection_amount;
    if (shiny_u == shiny_v)  /* Isotropic */
	specular_reflection_amount = normal_dot_light * 
	    mi_ward_glossy(
		&state->dir, &direction_toward_light, &state->normal, shiny_u);

    else {	             /* Anisotropic */
	miVector u = state->derivs[0], v;
	float d = mi_vector_dot(&u, &state->normal);
	u.x -= d * state->normal.x;
	u.y -= d * state->normal.y;
	u.z -= d * state->normal.z;
	mi_vector_normalize(&u);
	/* Set v to be perpendicular to u (in the tangent plane) */
	mi_vector_prod(&v, &state->normal, &u);
	specular_reflection_amount = normal_dot_light * 
	    mi_ward_anisglossy(&state->dir, &direction_toward_light, 
			       &state->normal, &u, &v, shiny_u, shiny_v);
    }
    if (specular_reflection_amount > 0.0) {
	result->r += specular_reflection_amount * glossy->r * light_color->r;
	result->g += specular_reflection_amount * glossy->g * light_color->g;
	result->b += specular_reflection_amount * glossy->b * light_color->b;
    }
}


/* Shader: mock_specular */

void miaux_add_color(miColor *result, miColor *c)
{
    result->r += c->r;
    result->g += c->g;
    result->b += c->b;
    result->a += c->a;
}

void miaux_multiply_colors(miColor *result, miColor *x, miColor *y)
{
    result->r = x->r * y->r;
    result->g = x->g * y->g;
    result->b = x->b * y->b;
}

void miaux_scale_channels(miColor *result, miColor* color, 
			  miScalar r_scale, miScalar g_scale, miScalar b_scale)
{
    result->r = color->r * r_scale;
    result->g = color->g * g_scale;
    result->b = color->b * b_scale;
}

double miaux_fit_clamp(
    double v, double oldmin, double oldmax, double newmin, double newmax)
{
    if (oldmin > oldmax) {
	double temp = oldmin;
        oldmin = oldmax;
	oldmax = oldmin;
	temp = newmin;
        newmin = newmax;
	newmax = newmin;
    }
    if (v < oldmin)
	return newmin;
    else if (v > oldmax)
	return newmax;
    else 
	return miaux_fit(v, oldmin, oldmax, newmin, newmax);
}

double miaux_sinusoid_fit_clamp(
    double v, double oldmin, double oldmax, double newmin, double newmax)
{
    return miaux_fit(sin(miaux_fit_clamp(v, oldmin, oldmax, -M_PI_2, M_PI_2)), 
		     -1, 1, newmin, newmax);
}

void miaux_add_mock_specular_component(
    miColor *result, miState *state,
    miVector *direction_toward_light,
    miColor *specular_color,
    miColor *cutoff,
    miColor *light_color)
{
    miScalar lightdir_offset, r_scale, g_scale, b_scale;
    miColor attenuated_specular = {0,0,0,0};

    lightdir_offset = mi_vector_dot(&state->normal, direction_toward_light);
    r_scale = miaux_sinusoid_fit_clamp(lightdir_offset, cutoff->r, 1.0, 0, 1);
    g_scale = miaux_sinusoid_fit_clamp(lightdir_offset, cutoff->g, 1.0, 0, 1);
    b_scale = miaux_sinusoid_fit_clamp(lightdir_offset, cutoff->b, 1.0, 0, 1);

    miaux_scale_channels(&attenuated_specular, specular_color,
			 r_scale, g_scale, b_scale);
    miaux_multiply_colors(light_color, light_color, &attenuated_specular);
    miaux_add_color(result, light_color);
}


/* Shader: rim_bright */

void miaux_brighten_rim(miColor *result, miState *state,
			miColor *rim_color)
{
    miaux_add_scaled_color(result, rim_color, 1.0 + state->dot_nd);
}


/* Chapter 13 -- Shadows ---------------------------------------------------- */

/* Shader: shadow_breakpoint */

double miaux_shadow_breakpoint (
    double color, double transparency, double breakpoint )
{
    if (transparency < breakpoint)
	return miaux_fit(transparency, 0, breakpoint, 0, color);
    else
	return miaux_fit(transparency, breakpoint, 1, color, 1);
}


/* Shader: shadow_breakpoint_scale */

double miaux_shadow_breakpoint_scale (
    double color, double transparency, double breakpoint, 
    double center, double extent )
{
    double scaled_color = 
	miaux_fit(color, 0, 1, center - extent/2.0, center + extent/2.0);
    if (transparency < breakpoint)
	return miaux_fit(transparency, 0, breakpoint, 0, scaled_color);
    else
	return miaux_fit(transparency, breakpoint, 1, scaled_color, 1);
}


/* Shader: shadow_continuous */

double miaux_shadow_continuous (
    double color, double transparency, double expansion )
{
    return transparency 
	+ miaux_fit(transparency,
		    0, 1, 
		    expansion * transparency * (color - transparency), 0);
}


/* Chapter 15 -- Refraction ------------------------------------------------- */

/* Shader: specular_refraction_simple */

miBoolean miaux_ray_is_entering_material(miState *state)
{
    miState *s;
    miBoolean entering = miTRUE;
    for (s = state; s != NULL; s = s->parent)
	if (s->material == state->material)
	    entering = !entering;
    return entering;
}


/* Shader: specular_refraction */

miScalar miaux_state_incoming_ior(miState *state)
{
    miScalar unassigned_ior = 0.0, default_ior = 1.0;
    if (state != NULL && state->ior_in != unassigned_ior)
	return state->ior_in;
    else
	return default_ior;
}

miScalar miaux_state_outgoing_ior(miState *state)
{
    miScalar unassigned_ior = 0.0, default_ior = 1.0;
    if (state != NULL && state->ior != unassigned_ior)
	return state->ior;
    else
	return default_ior;
}

miBoolean miaux_shaders_equal(miState *s1, miState *s2)
{
    return s1->shader == s2->shader;
}

miBoolean miaux_parent_exists(miState *state)
{ 
    return state->parent != NULL;
}

miBoolean miaux_ray_is_transmissive(miState *state)
{
    return state->type == miRAY_TRANSPARENT ||
	state->type == miRAY_REFRACT;
}

miBoolean miaux_ray_is_entering(
    miState *state,
    miState *state_of_this_shaders_previous_transmission)
{
    miState *s;
    miBoolean ray_is_entering = miTRUE;
    state_of_this_shaders_previous_transmission = NULL;
    for (s = state; s; s = s->parent)	
	if (miaux_ray_is_transmissive(s) &&
	    miaux_parent_exists(s) &&
	    miaux_shaders_equal(s->parent, state)) {
	    ray_is_entering = !ray_is_entering;
	    if (state_of_this_shaders_previous_transmission == NULL)
		state_of_this_shaders_previous_transmission = s->parent;
	}
    return ray_is_entering;
}

void miaux_set_state_refraction_indices(miState *state,
					miScalar material_ior)
{
    miState *previous_transmission = NULL;
    miScalar incoming_ior, outgoing_ior;
       
    if (miaux_ray_is_entering(state, previous_transmission)) {
	outgoing_ior = material_ior;
	incoming_ior = miaux_state_outgoing_ior(state->parent);
    } else {	
	incoming_ior = material_ior;
	outgoing_ior = miaux_state_incoming_ior(previous_transmission);
    }
    state->ior_in = incoming_ior;
    state->ior = outgoing_ior;
}


/* Chapter 16 -- Light from other surfaces ---------------------------------- */

/* Shader: global_lambert */

void miaux_add_multiplied_colors(miColor *result, 
				 miColor *color, miColor *scaling_color)
{
    result->r += color->r * scaling_color->r;
    result->g += color->g * scaling_color->g;
    result->b += color->b * scaling_color->b;
}


/* Shader: store_diffuse_photon */

void miaux_multiply_color(miColor *result, miColor *color)
{
    result->r *= color->r;
    result->g *= color->g;
    result->b *= color->b;
}


/* Shader: average_radiance_options */

void miaux_set_scalar_if_not_default(
    miScalar *result, miState *state, miScalar *param)
{
    miScalar use_default_flag = -1.0;
    miScalar param_value = *mi_eval_scalar(param);
    if (param_value != use_default_flag)
	*result = param_value;
}

void miaux_set_integer_if_not_default(
    miInteger *result, miState *state, miInteger *param) 
{
    miInteger use_default_flag = -1;
    miInteger param_value = *mi_eval_integer(param);
    if (param_value != use_default_flag)
	*result = param_value;
}


/* Chapter 17 -- Modifying surface geometry --------------------------------- */

/* Shader: displace_wave */

double miaux_sinusoid(double v, double frequency, double amplitude)
{
    return sin(v * frequency * M_PI * 2.0) * amplitude;
}


/* Chapter 18 -- Modifying surface orientation ------------------------------ */

/* Shader: bump_texture */

miScalar miaux_color_channel_average(miColor *c)
{
    return (c->r + c->g + c->b) / 3.0;
}


/* Chapter 19 -- Creating geometric objects --------------------------------- */

/* Shader: triangles */

void miaux_add_vertex(int index, miScalar x, miScalar y, miScalar z)
{
    miVector v;
    v.x = x; v.y = y; v.z = z;
    mi_api_vector_xyz_add(&v);
    mi_api_vertex_add(index);
}

float miaux_random_range(float min_value, float max_value)
{
    return miaux_fit(mi_random(), 0.0, 1.0, min_value, max_value);
}

void miaux_random_point_in_unit_sphere(float *x, float *y, float *z)
{
    do {
	*x = miaux_random_range(-.5, .5);
	*y = miaux_random_range(-.5, .5);
	*z = miaux_random_range(-.5, .5);
    } while (sqrt((*x * *x) +(*y * *y) + (*z * *z)) >= 0.5);
}

void miaux_add_random_triangle(int index, float edge_length_max,
			       miVector *bbox_min, miVector *bbox_max)
{
    int vertex_index;
    float offset_max = edge_length_max / 2.0;
    float offset_min = -offset_max;
    float x, y, z;

    miaux_random_point_in_unit_sphere(&x, &y, &z);
    x = miaux_fit(x, -.5, .5, bbox_min->x, bbox_max->x);
    y = miaux_fit(y, -.5, .5, bbox_min->y, bbox_max->y);
    z = miaux_fit(z, -.5, .5, bbox_min->z, bbox_max->z);

    miaux_add_vertex(index, x, y, z);
    miaux_add_vertex(index + 1, 
		     x + miaux_random_range(offset_min, offset_max),
		     y + miaux_random_range(offset_min, offset_max),
		     z + miaux_random_range(offset_min, offset_max));
    miaux_add_vertex(index + 2, 
		     x + miaux_random_range(offset_min, offset_max),
		     y + miaux_random_range(offset_min, offset_max),
		     z + miaux_random_range(offset_min, offset_max));

    mi_api_poly_begin_tag(1, 0);
    for (vertex_index = 0; vertex_index < 3; vertex_index++)
	mi_api_poly_index_add(index + vertex_index);
    mi_api_poly_end();
}


/* Shader: object_file */

miTag miaux_object_from_file(
    const char* name, const char* filename, 
    miVector bbox_min, miVector bbox_max)
{
    miObject *obj = mi_api_object_begin(mi_mem_strdup(name));
    obj->visible = miTRUE;
    obj->shadow = 3;
    obj->bbox_min = bbox_min;
    obj->bbox_max = bbox_max;
    mi_api_object_file(mi_mem_strdup(filename));
    return mi_api_object_end();
}


/* Chapter 20 -- Modeling hair ---------------------------------------------- */

/* Shader: hair_geo_2v */

void miaux_define_hair_object(
    miTag name_tag, miaux_bbox_function bbox_function, void *params,
    miTag *geoshader_result, miApi_object_callback callback)
{
    miTag tag;
    miObject *obj;
    char *name = miaux_tag_to_string(name_tag, "::hair");
    obj = mi_api_object_begin(mi_mem_strdup(name));
    obj->visible = miTRUE;
    obj->shadow = obj->reflection = obj->refraction = 3;
    bbox_function(obj, params);
    if (geoshader_result != NULL && callback != NULL) {
	mi_api_object_callback(callback, params);
	tag = mi_api_object_end();
	mi_geoshader_add_result(geoshader_result, tag);
	obj = (miObject *)mi_scene_edit(tag);
	obj->geo.placeholder_list.type = miOBJECT_HAIR;
	mi_scene_edit_end(tag);
    }
}

void miaux_describe_bbox(miObject *obj)
{
    mi_progress("Object bbox: %f,%f,%f  %f,%f,%f",
		obj->bbox_min.x, obj->bbox_min.y, obj->bbox_min.z,
		obj->bbox_max.x, obj->bbox_max.y, obj->bbox_max.z);
}

void miaux_adjust_bbox(miObject *obj, miVector *v, miScalar extra)
{
    miVector v_extra, vmin, vmax;
    miaux_set_vector(&v_extra, extra, extra, extra);
    mi_vector_sub(&vmin, v, &v_extra);
    mi_vector_add(&vmax, v, &v_extra);
    mi_vector_min(&obj->bbox_min, &obj->bbox_min, &vmin);
    mi_vector_max(&obj->bbox_max, &obj->bbox_max, &vmax);
}

void miaux_init_bbox(miObject *obj)
{
    obj->bbox_min.x = miHUGE_SCALAR;
    obj->bbox_min.y = miHUGE_SCALAR;
    obj->bbox_min.z = miHUGE_SCALAR;
    obj->bbox_max.x = -miHUGE_SCALAR;
    obj->bbox_max.y = -miHUGE_SCALAR;
    obj->bbox_max.z = -miHUGE_SCALAR;
}


/* Shader: hair_geo_4v */

void miaux_append_hair_vertex(miScalar **scalar_array, miVector *v)
{
    (*scalar_array)[0] = v->x;
    (*scalar_array)[1] = v->y;
    (*scalar_array)[2] = v->z;
    *scalar_array += 3;
}


/* Shader: hair_geo_4v_texture */

void miaux_append_hair_data(
    miScalar **scalar_array, miVector *v, miScalar position,
    miScalar root_radius, miColor *root, miScalar tip_radius, miColor *tip )
{
    (*scalar_array)[0] = v->x;
    (*scalar_array)[1] = v->y;
    (*scalar_array)[2] = v->z;
    (*scalar_array)[3] = miaux_fit(position, 0, 1, root_radius, tip_radius);
    (*scalar_array)[4] = miaux_fit(position, 0, 1, root->r, tip->r);
    (*scalar_array)[5] = miaux_fit(position, 0, 1, root->g, tip->g);
    (*scalar_array)[6] = miaux_fit(position, 0, 1, root->b, tip->b);
    (*scalar_array)[7] = miaux_fit(position, 0, 1, root->a, tip->a);
    *scalar_array += 8;
}

double miaux_max(double a, double b)
{
    return a > b ? a : b;
}


/* Shader: hair_color_texture */

void miaux_copy_color(miColor *result, miColor *color)
{
    result->r = color->r;
    result->g = color->g;
    result->b = color->b;
    result->a = color->a;
}


/* Shader: hair_color_light */

void miaux_opacity_set_channels(miState *state, miScalar value)
{
    miColor opacity;
    miaux_set_channels(&opacity, value);
    mi_opacity_set(state, &opacity);
}

void miaux_add_transparent_hair_component(miColor *result, miState *state)
{
    miColor background_color;
    mi_trace_transparent(&background_color, state);
    if (result->a == 0) {
	miaux_opacity_set_channels(state, 0.0);
	miaux_copy_color(result, &background_color);
    } else {
	miaux_opacity_set_channels(state, result->a);
	miaux_blend_colors(result, result, &background_color, result->a); 
    }
}

void miaux_add_specular_hair_component(
    miColor *result, miVector *hair_tangent, miVector *to_light,
    miVector *to_camera,
    miColor *specular, miColor *light_color)
{
    miScalar light_angle = acos(mi_vector_dot(hair_tangent, to_light));
    miScalar view_angle = acos(mi_vector_dot(hair_tangent, to_camera));
    miScalar sum = light_angle + view_angle;
    miScalar specular_factor = fabs(M_PI_2 - fmod(sum, M_PI)) / M_PI_2;

    result->r += specular_factor * specular->r * light_color->r;
    result->g += specular_factor * specular->g * light_color->g;
    result->b += specular_factor * specular->b * light_color->b;
}

void miaux_add_diffuse_hair_component(
    miColor *result, miVector *hair_tangent, miVector *to_light,
    miColor *diffuse, miColor *light_color)
{
    miScalar diffuse_factor = 1.0 - fabs(mi_vector_dot(hair_tangent, to_light));
    miaux_add_diffuse_component(result, diffuse_factor, diffuse, light_color);
}

double miaux_clamp(double v, double minval, double maxval)
{
    return v < minval ? minval : v > maxval ? maxval : v;
}

void miaux_clamp_color(miColor *result)
{
    result->r = miaux_clamp(result->r, 0.0, 1.0);
    result->g = miaux_clamp(result->g, 0.0, 1.0);
    result->b = miaux_clamp(result->b, 0.0, 1.0);
    result->a = miaux_clamp(result->a, 0.0, 1.0);
}


/* Shader: hair_geo_curl */

void miaux_point_between(
    miVector *result, miVector *u, miVector *v, float fraction)
{
    result->x = miaux_fit(fraction, 0, 1, u->x, v->x);
    result->y = miaux_fit(fraction, 0, 1, u->y, v->y);
    result->z = miaux_fit(fraction, 0, 1, u->z, v->z);
}

void miaux_perpendicular_point(miVector *result, miVector *v, float distance)
{
    result->x = -v->y;
    result->y = v->x;
    result->z = 0;
    mi_vector_normalize(result);
    mi_vector_mul(result, distance);
}

void miaux_hair_spiral(
    miScalar** scalar_array, miVector *start_point, miVector *end_point, 
    float turns, int point_count, float angle_offset, float spiral_radius,
    miScalar root_radius, miScalar tip_radius)
{
    miVector base_point, spiral_axis, spiral_point;
    miVector axis_point;
    miMatrix matrix;
    float angle, pi_2 = 2 * M_PI, max_index = point_count - 1;
    int i;

    mi_vector_sub(&spiral_axis, end_point, start_point);
    mi_vector_normalize(&spiral_axis);
    miaux_perpendicular_point(&axis_point, &spiral_axis, spiral_radius);

    for (i = 0; i < point_count; i++) {
	float fraction = i / max_index;
	miaux_point_between(&base_point, start_point, end_point, fraction);
	angle = angle_offset + fraction * turns * pi_2;
	mi_matrix_rotate_axis(matrix, &spiral_axis, angle);
	mi_point_transform(&spiral_point, &axis_point, matrix);
	mi_vector_add(&spiral_point, &spiral_point, &base_point);

	*(*scalar_array)++ = spiral_point.x;
	*(*scalar_array)++ = spiral_point.y;
	*(*scalar_array)++ = spiral_point.z;
	*(*scalar_array)++ =
	    miaux_fit_clamp(i, 0, max_index, root_radius, tip_radius);
    }
}

void miaux_random_point_on_sphere(
    miVector *result, miVector *center, miScalar radius)
{
    miMatrix transform;
    result->x = radius;
    result->y = result->z = 0.0;
    mi_matrix_rotate(transform, 0,
		     miaux_random_range(0, M_PI * 2),
		     miaux_random_range(0, M_PI * 2));
    mi_vector_transform(result, result, transform);
    mi_vector_add(result, result, center);
}


/* Shader: hair_geo_datafile */

void miaux_read_hair_data_file(char* filename, miScalar radius)
{
    int vertex_count, total_vertex_count, hair_scalar_size, vertex_total = 0,
	index_array_size, v, *hair_indices, *hi, hair_count, per_hair_scalars;
    float xmin, ymin, zmin, xmax, ymax, zmax, age;
    miScalar coord, *hair_scalars;
    miGeoIndex *harray;
    FILE *fp;

    fp = fopen(filename, "r");
    fscanf(fp, "%d %d ", &hair_count, &total_vertex_count);
    fscanf(fp, "%f %f %f %f %f %f ", 
	   &xmin, &ymin, &zmin, &xmax, &ymax, &zmax);
    mi_progress("particle bounding box: %f %f %f %f %f %f ",
		xmin, ymin, zmin, xmax, ymax, zmax);

    per_hair_scalars = 2;
    mi_api_hair_info(0, 'r', 1);
    mi_api_hair_info(0, 't', 1);

    hair_scalar_size = hair_count * per_hair_scalars + total_vertex_count * 3;
    hair_scalars = mi_api_hair_scalars_begin(hair_scalar_size);

    index_array_size = 1 + hair_count;
    hi = hair_indices = (int*)mi_mem_allocate(sizeof(int) * index_array_size);
    *hi++ = 0;
    vertex_total = 0;

    while (!feof(fp)) {
	*hair_scalars++ = radius;
	fscanf(fp, "%f ", &age);
	*hair_scalars++ = age; 
	fscanf(fp, "%d ", &vertex_count);
	for (v = 0; v < vertex_count * 3; v++) {
	    fscanf(fp, "%f ", &coord);
	    *hair_scalars++ = coord;
	}
	vertex_total += vertex_count * 3 + per_hair_scalars;
	*hi++ = vertex_total;
    }
    mi_api_hair_scalars_end(hair_scalar_size);
    harray = mi_api_hair_hairs_begin(index_array_size);
    memcpy(harray, hair_indices, index_array_size * sizeof(int));
    mi_api_hair_hairs_end();
}

void miaux_hair_data_file_bounding_box(
    char* filename,
    float *xmin, float *ymin, float *zmin, 
    float *xmax, float *ymax, float *zmax)
{
    int hair_count, data_count;
    FILE* fp = fopen(filename, "r");
    fscanf(fp, "%d %d ", &hair_count, &data_count); /* Ignore. */
    fscanf(fp, "%f %f %f %f %f %f ", xmin, ymin, zmin, xmax, ymax, zmax);
    fclose(fp);
}


/* Shader: hair_color_fire */

void miaux_blend_transparency(miColor *result, 
			      miState *state, miColor *transparency)
{
    miColor opacity, background;
    miaux_invert_channels(&opacity, transparency);
    mi_opacity_set(state, &opacity);
    if (!miaux_all_channels_equal(transparency, 1.0)) {
        mi_trace_transparent(&background, state);
        miaux_blend_channels(result, &background, &opacity);
    }
}

void miaux_color_fit(miColor *result, 
		     miScalar f, miScalar start, miScalar end,
		     miColor *start_color, miColor *end_color)
{
    result->r = miaux_fit(f, start, end, start_color->r, end_color->r);
    result->g = miaux_fit(f, start, end, start_color->g, end_color->g);
    result->b = miaux_fit(f, start, end, start_color->b, end_color->b);
}


/* Chapter 21 -- The environment of the scene ------------------------------- */

/* Shader: chrome_ramp */

miScalar miaux_interpolated_lookup(miScalar lookup_table[], int table_size,
				   miScalar t)
{
    int lower_index = (int)(t * (table_size - 1));
    miScalar lower_value = lookup_table[lower_index];
    int upper_index = lower_index + 1;
    miScalar upper_value = lookup_table[upper_index];
    return miaux_fit(
	t * table_size, lower_index, upper_index, lower_value, upper_value);
}

float miaux_altitude(miState *state)
{
    miVector ray;
    mi_vector_to_world(state, &ray, &state->dir);
    mi_vector_normalize(&ray);
    return miaux_fit(asin(ray.y), -M_PI_2, M_PI_2, 0.0, 1.0);
}

void miaux_piecewise_sinusoid(
    miScalar result[], int result_count, 
    int key_count, miScalar key_positions[], miScalar key_values[]) 
{
    int key, i;
    for (key = 1; key < key_count; key++) {
	int start = (int)(key_positions[key-1] * result_count);
	int end = (int)(key_positions[key] * result_count) - 1;
	for (i = start; i <= end; i++) {
	    result[i] = miaux_sinusoid_fit(
		i, start, end, key_values[key-1], key_values[key]);
	}
    }
}


/* Shader: channel_ramp */

miBoolean miaux_release_user_memory(char* shader_name, miState *state, void *params)
{
    if (params != NULL) {  /* Shader instance exit */
	void **user_pointer;                           
	if (!mi_query(miQ_FUNC_USERPTR, state, 0, &user_pointer))
	    mi_fatal("Could not get user pointer in shader exit function %s_exit", 
		     shader_name);
	mi_mem_release(*user_pointer);
    }
    return miTRUE;
}

void* miaux_user_memory_pointer(miState *state, int allocation_size)
{
    void **user_pointer;
    mi_query(miQ_FUNC_USERPTR, state, 0, &user_pointer);
    if (allocation_size > 0) {
	*user_pointer = mi_mem_allocate(allocation_size);
    }
    return *user_pointer;
}


/* Shader: color_ramp */

void miaux_interpolated_color_lookup(miColor* result,
				     miColor lookup_table[], int table_size,
				     miScalar t)
{
    int lower_index = (int)(t * table_size);
    miColor lower_value = lookup_table[lower_index];
    int upper_index = lower_index + 1;
    miColor upper_value = lookup_table[upper_index];
    result->r = miaux_fit(t * table_size, lower_index, upper_index,
			  lower_value.r, upper_value.r);
    result->g = miaux_fit(t * table_size, lower_index, upper_index,
			  lower_value.g, upper_value.g);
    result->b = miaux_fit(t * table_size, lower_index, upper_index,
			  lower_value.b, upper_value.b);
}

void miaux_piecewise_color_sinusoid(
    miColor result[], int result_count, int key_count, miColor key_values[]) 
{
    int key, i;
    for (key = 1; key < key_count; key++) {
	int start = (int)(key_values[key-1].a * result_count);
	int end = (int)(key_values[key].a * result_count) - 1;
	for (i = start; i <= end; i++) {
	    result[i].r =
		miaux_sinusoid_fit(
		    i, start, end, key_values[key-1].r, key_values[key].r);
	    result[i].g = 
		miaux_sinusoid_fit(
		    i, start, end, key_values[key-1].g, key_values[key].g);
	    result[i].b = 
		miaux_sinusoid_fit(
		    i, start, end, key_values[key-1].b, key_values[key].b);
	}
    }
}


/* Chapter 22 -- A visible atmosphere --------------------------------------- */

/* Shader: ground_fog */

void miaux_point_along_vector(
    miVector *result, miVector *point, miVector *direction, miScalar distance)
{
    result->x = point->x + distance * direction->x;
    result->y = point->y + distance * direction->y;
    result->z = point->z + distance * direction->z;
}

void miaux_march_point(
    miVector *result, miState *state, miScalar distance)
{
    miaux_point_along_vector(result, &state->org, &state->dir, distance);
}

void miaux_world_space_march_point(
    miVector *result, miState *state, miScalar distance)
{
    miaux_march_point(result, state, distance);
    mi_vector_to_world(state, result, result);
}


/* Chapter 23 -- Volumetric effects ----------------------------------------- */

/* Shader: threshold_volume */

miScalar miaux_threshold_density(
    miVector *point, miVector *center, miScalar radius, 
    miScalar unit_density, miScalar march_increment)
{
    miScalar distance = mi_vector_dist(center, point);
    if (distance <= radius)
	return unit_density * march_increment;
    else
	return 0.0;
}


/* Shader: density_volume */

miScalar miaux_density_falloff(
    miVector *point, miVector *center, miScalar radius, 
    miScalar unit_density, miScalar march_increment)
{
    return march_increment * unit_density * 
	miaux_fit_clamp(mi_vector_dist(center, point), 0.0, radius, 1.0, 0.0);
}


/* Shader: illuminated_volume */

void miaux_alpha_blend_colors(
    miColor *result, miColor *foreground, miColor *background)
{
    double bg_fraction = 1.0 - foreground->a;
    result->r = foreground->r + background->r * bg_fraction;
    result->g = foreground->g + background->g * bg_fraction;
    result->b = foreground->b + background->b * bg_fraction;
}

void miaux_add_transparent_color(
    miColor *result, miColor *color, miScalar transparency)
{
    miScalar new_alpha = result->a + transparency;
    if (new_alpha > 1.0)
	transparency = 1.0 - result->a;
    result->r += color->r * transparency;
    result->g += color->g * transparency;
    result->b += color->b * transparency;
    result->a += transparency;
}

void miaux_total_light_at_point(
    miColor *result, miVector *point, miState *state,
    miTag* light, int light_count) 
{
    miColor sum, light_color;
    int i, light_sample_count;
    miVector original_point = state->point;
    state->point = *point;

    miaux_set_channels(result, 0.0);
    for (i = 0; i < light_count; i++, light++) {
	miVector direction_to_light;
	light_sample_count = 0;
	miaux_set_channels(&sum, 0.0);
	while (mi_sample_light(&light_color, &direction_to_light, NULL, 
			       state, *light, &light_sample_count))
	    miaux_add_scaled_color(&sum, &light_color, 1.0);

	if (light_sample_count)
	    miaux_add_scaled_color(result, &sum, 1/light_sample_count);
    }
    state->point = original_point;
}

miScalar miaux_fractional_occlusion_at_point(
    miVector *start_point, miVector *direction, 
    miScalar total_distance, miVector *center, miScalar radius, 
    miScalar unit_density, miScalar march_increment)
{
    miScalar distance, occlusion = 0.0;
    miVector march_point;
    mi_vector_normalize(direction);
    for (distance = 0; distance <= total_distance; distance += march_increment) {
	miaux_point_along_vector(&march_point, start_point, direction, distance);
	occlusion += miaux_threshold_density(&march_point, center, radius,
					     unit_density, march_increment);
	if (occlusion >= 1.0) {
	    occlusion = 1.0;
	    break;
	}
    }
    return occlusion;
}


/* Shader: parameter_volume */

miScalar miaux_fractional_shader_occlusion_at_point(
    miState *state, miVector *start_point, miVector *direction, 
    miScalar total_distance, miTag density_shader, miTag deform_shader,
    miScalar unit_density, miScalar march_increment)
{
    miScalar density, distance, occlusion = 0.0;
    miVector march_point,deformed_point;
    miVector original_point = state->point;
    mi_vector_normalize(direction);
    for (distance = 0; distance <= total_distance; distance += march_increment) {
	miaux_point_along_vector(&march_point, start_point, direction, distance);
	state->point = march_point;
	mi_call_shader_x((miColor*)&deformed_point, miSHADER_MATERIAL, state, deform_shader, NULL);
	state->point = deformed_point;
	mi_call_shader_x((miColor*)&density, miSHADER_MATERIAL, state, 
			 density_shader, NULL);
	occlusion += density * unit_density * march_increment;
	if (occlusion >= 1.0) {
	    occlusion = 1.0;
	    break;
	}
    }
    state->point = original_point;
    return occlusion;
}


/* Shader: voxel_density */

miBoolean miaux_point_inside(miVector *p, miVector *min_p, miVector *max_p)
{
    return p->x >= min_p->x && p->y >= min_p->y && p->z >= min_p->z &&
	p->x <= max_p->x && p->y <= max_p->y && p->z <= max_p->z;
}

void miaux_read_volume_block(
    char* filename, 
    int *width, int *height, int *depth, float* block)
{
    int count;
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
	mi_fatal("Error opening file \"%s\".", filename);
    }
    fscanf(fp, "%d %d %d ", width, height, depth);
    count = (*width) * (*height) * (*depth);
    mi_progress("Volume dataset: %dx%dx%d", *width, *height, *depth);
    fread(block, sizeof(float), count, fp);
}


/* Chapter 24 -- Changing the lens ------------------------------------------ */

/* Shader: fisheye */

double miaux_distance(double x1, double y1, double x2, double y2)
{
    double x = x2 - x1, y = y2 - y1;
    return sqrt(x * x + y * y);
}


/* Shader: depth_of_field */

void miaux_divide_color(miColor *result, miColor* color, miScalar f)
{
    result->r = color->r / f;
    result->g = color->g / f;
    result->b = color->b / f;
}

void miaux_from_camera_space(
        miState *state, miVector *origin, miVector *direction)
{
    mi_point_from_camera(state, origin, origin);
    mi_vector_from_camera(state, direction, direction);
}

void miaux_square_to_circle(
    float *result_x, float *result_y, float x, float y, float max_radius)
{
    float angle = M_PI * 2 * x;
    float radius = max_radius * sqrt(y);
    *result_x = radius * cos(angle);
    *result_y = radius * sin(angle);
}

void miaux_sample_point_within_radius(
    miVector *result, miVector *center, float x, float y, float max_radius)
{
    float x_offset, y_offset;
    miaux_square_to_circle(&x_offset, &y_offset, x, y, max_radius);
    result->x = center->x + x_offset;
    result->y = center->y + y_offset;
    result->z = center->z;
}

void miaux_z_plane_intersect(
    miVector *result, miVector *origin, miVector *direction, miScalar z_plane)
{
    miScalar z_delta = (z_plane - origin->z) / direction->z;
    result->x = origin->x + z_delta * direction->x;
    result->y = origin->y + z_delta * direction->y;
    result->z = z_plane;
}

void miaux_to_camera_space(
    miState *state, miVector *origin, miVector *direction)
{
    mi_point_to_camera(state, origin, &state->org);
    mi_vector_to_camera(state, direction, &state->dir);
}


/* Chapter 25 -- Rendering image components --------------------------------- */

/* Shader: shadowpass */

void miaux_divide_colors(miColor *result, miColor *x, miColor *y)
{
    result->r = y->r == 0.0 ? 1.0 : x->r / y->r;
    result->g = y->g == 0.0 ? 0.0 : x->g / y->g;
    result->b = y->b == 0.0 ? 1.0 : x->b / y->b;
}

void miaux_add_light_color(
    miColor *result, miState *state, miTag light, char shadow_state)
{
    int light_sample_count = 0;
    miScalar dot_nl;
    miVector direction_toward_light;
    miColor sample_color, single_light_color;

    const miOptions *original_options = state->options;
    miOptions options_copy = *original_options;
    options_copy.shadow = shadow_state;
    state->options = &options_copy;

    miaux_set_channels(&single_light_color, 0.0);
    while (mi_sample_light(&sample_color, &direction_toward_light,
			   &dot_nl, state, light, &light_sample_count))
	miaux_add_scaled_color(&single_light_color, &sample_color, dot_nl);
    if (light_sample_count)
	miaux_add_scaled_color(result, &single_light_color,
			       1.0/light_sample_count);

    state->options = (miOptions*)original_options;
}


/* Chapter 26 -- Modifying the final image ---------------------------------- */

/* Shader: median_filter */

int miaux_color_compare(const void *vx, const void *vy)
{
    miColor const *x = vx, *y = vy;
    float sum_x = x->r + x->g + x->b;
    float sum_y = y->r + y->g + y->b;
    return sum_x < sum_y ? -1 : sum_x > sum_y ? 1 : 0;
}

void miaux_pixel_neighborhood(
    miColor *neighbors, 
    miImg_image *buffer, int x, int y, int radius)
{
    int xi, yi, xp = 0, yp = 0, i = 0, 
	max_x = buffer->width - 1, max_y = buffer->height - 1;
    miColor current_pixel;

    for (yi = y - radius; yi <= y + radius; yi++) { 
	yp = yi > max_y ? max_x : yi < 0 ? 0 : yi;
	for (xi = x - radius; xi <= x + radius; xi++) {
	    xp = xi > max_x ? max_x : xi < 0 ? 0 : xi;
	    mi_img_get_color(buffer, &current_pixel, xp, yp);
	    neighbors[i++] = current_pixel;
	}
    }
}


/* Shader: annotate */

void miaux_release_fontimage(fontimage *fimage)
{
    mi_mem_release(fimage->x_offsets);
    mi_mem_release(fimage->image);
}

void miaux_alpha_blend(miColor *x, miColor *y, miScalar alpha)
{
    x->r = miaux_blend(y->r, x->r, alpha);
    x->g = miaux_blend(y->g, x->g, alpha);
    x->b = miaux_blend(y->b, x->b, alpha);
}

void miaux_text_image(
    float **text_image, int *width, int *height, 
    fontimage *fimage, char* text)
{
    char *c;
    int i, total_width, xpos, first_printable = 32, image_size, 
	index, start, end, font_index, fx, fy, tx, ty, text_index;

    for (i = 0, total_width = 0, c = text; i < strlen(text); i++, c++) {
	index = *c - first_printable;
	start = fimage->x_offsets[index];
	end = fimage->x_offsets[index + 1];
	total_width += end - start + 1;
    }
    *width = total_width;
    *height = fimage->height;
    image_size = *width * *height;
    (*text_image) = (float*)mi_mem_allocate(image_size * sizeof(float));

    for (i = 0, c = text, xpos = 0; i < strlen(text); i++, c++) {
	int index = *c - first_printable;
	start = fimage->x_offsets[index];
	end = fimage->x_offsets[index + 1];
	for (fy = fimage->height - 1, ty = 0; fy >= 0; fy--, ty++) {
	    for (fx = start, tx = xpos; fx < end-1; fx++, tx++) {
		text_index = ty * *width + tx;
		font_index = fy * fimage->width + fx;
		(*text_image)[text_index] = 
		    (float)fimage->image[font_index] / 255.0;
	    }
	}
	xpos += end - start + 1;
    }
}

void miaux_load_fontimage(fontimage *fimage, char* filename)
{ 
    int printable_ascii_size = 126 - 32 + 1;
    int i, image_data_size;
    char identifier[33];

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
	mi_fatal("Could not open font image file: %s", filename);
    fscanf(fp, "%s ", identifier);
    if (strcmp(identifier, "FONTIMAGE") != 0)
	mi_fatal("File '%s' does not look like a fontimage file", filename);

    fscanf(fp, "%d %d ", &fimage->width, &fimage->height);
    image_data_size = fimage->width * fimage->height;
    fimage->x_offsets = 
	(int*)mi_mem_allocate(sizeof(int) * printable_ascii_size);
    for (i = 0; i <= printable_ascii_size; i++)
	fscanf(fp, "%d ", &fimage->x_offsets[i]);
    fimage->image = (unsigned char*)mi_mem_allocate(image_data_size);
    fread(fimage->image, image_data_size, 1, fp);

    fclose(fp);
}

