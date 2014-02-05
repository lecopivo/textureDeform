/*
   "Writing mental ray shaders" -- Utility functions used in shaders
   See http://www.writingshaders.com/ for the current software distribution
*/

#ifndef __MIAUX_H__
#define __MIAUX_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include "shader.h"
#include "geoshader.h"

typedef struct {
    miScalar x;
    miScalar y;
    miScalar z;
    miScalar r;
} miaux_sphere_spec;

typedef void (*miaux_bbox_function)(miObject*, void*);

typedef struct {
    int width;
    int height;
    int *x_offsets;
    unsigned char *image;
} fontimage;

double miaux_fit(
    double v, double oldmin, double oldmax, double newmin, double newmax);
double miaux_blend(miScalar a, miScalar b, miScalar factor);
void miaux_blend_colors(miColor *result, 
                        miColor *color1, miColor *color2, miScalar factor);
void miaux_blend_channels(miColor *result,
                          miColor *blend_color, miColor *blend_fraction);
void miaux_invert_channels(miColor *result, miColor *color);
miBoolean miaux_all_channels_equal(miColor *c, miScalar v);
void miaux_scale_vector(miVector *result, miScalar scale);
void miaux_set_vector(miVector *v, double x, double y, double z);
double miaux_summed_noise ( 
    miVector *point,
    double summing_weight, double octave_scaling, int octave_count);
char* miaux_tag_to_string(miTag tag, char *default_value);
void miaux_add_scaled_color(miColor *result, miColor *color, miScalar scale);
miScalar miaux_quantize(miScalar value, miInteger count);
void miaux_add_diffuse_component(
    miColor *result, 
    miScalar light_and_surface_cosine, 
    miColor *diffuse, miColor *light_color);
void miaux_set_channels(miColor *c, miScalar new_value);
void miaux_light_array(miTag **lights, int *light_count, miState *state,
                       int *offset_param, int *count_param, miTag *lights_param);
miScalar miaux_light_spread(miState *state, miTag light_tag);
miScalar miaux_offset_spread_from_light(miState *state, miTag light_tag);
miTag miaux_current_light_tag(miState *state);
void miaux_scale_color(miColor *result, miScalar scale);
double miaux_sinusoid_fit(
    double v, double oldmin, double oldmax, double newmin, double newmax);
void miaux_add_phong_specular_component(
    miColor *result, miState *state, miScalar exponent,
    miVector *direction_toward_light, 
    miColor *specular, miColor *light_color);
void miaux_add_blinn_specular_component(
    miColor *result, miState *state,
    miScalar roughness, miScalar ior,
    miVector direction_toward_light,
    miColor *specular, miColor *light_color);
void miaux_add_cook_torrance_specular_component(
    miColor *result, miState *state,
    miScalar roughness, miColor *ior,
    miVector direction_toward_light,
    miColor *specular, miColor *light_color);
void miaux_add_ward_specular_component(
    miColor *result, miState *state,
    miScalar shiny_u, miScalar shiny_v,
    miColor *glossy, miScalar normal_dot_light,
    miVector direction_toward_light,
    miColor *light_color);
void miaux_add_color(miColor *result, miColor *c);
void miaux_multiply_colors(miColor *result, miColor *x, miColor *y);
void miaux_scale_channels(miColor *result, miColor* color, 
                          miScalar r_scale, miScalar g_scale, miScalar b_scale);
double miaux_fit_clamp(
    double v, double oldmin, double oldmax, double newmin, double newmax);
double miaux_sinusoid_fit_clamp(
    double v, double oldmin, double oldmax, double newmin, double newmax);
void miaux_add_mock_specular_component(
    miColor *result, miState *state,
    miVector *direction_toward_light,
    miColor *specular_color,
    miColor *cutoff,
    miColor *light_color);
void miaux_brighten_rim(miColor *result, miState *state,
                        miColor *rim_color);
double miaux_shadow_breakpoint (
    double color, double transparency, double breakpoint );
double miaux_shadow_breakpoint_scale (
    double color, double transparency, double breakpoint, 
    double center, double extent );
double miaux_shadow_continuous (
    double color, double transparency, double expansion );
miBoolean miaux_ray_is_entering_material(miState *state);
miScalar miaux_state_incoming_ior(miState *state);
miScalar miaux_state_outgoing_ior(miState *state);
miBoolean miaux_shaders_equal(miState *s1, miState *s2);
miBoolean miaux_parent_exists(miState *state);
miBoolean miaux_ray_is_transmissive(miState *state);
miBoolean miaux_ray_is_entering(
    miState *state,
    miState *state_of_this_shaders_previous_transmission);
void miaux_set_state_refraction_indices(miState *state,
                                        miScalar material_ior);
void miaux_add_multiplied_colors(miColor *result, 
                                 miColor *color, miColor *scaling_color);
void miaux_multiply_color(miColor *result, miColor *color);
void miaux_set_scalar_if_not_default(
    miScalar *result, miState *state, miScalar *param);
void miaux_set_integer_if_not_default(
    miInteger *result, miState *state, miInteger *param);
double miaux_sinusoid(double v, double frequency, double amplitude);
miScalar miaux_color_channel_average(miColor *c);
void miaux_add_vertex(int index, miScalar x, miScalar y, miScalar z);
float miaux_random_range(float min_value, float max_value);
void miaux_random_point_in_unit_sphere(float *x, float *y, float *z);
void miaux_add_random_triangle(int index, float edge_length_max,
                               miVector *bbox_min, miVector *bbox_max);
miTag miaux_object_from_file(
    const char* name, const char* filename, 
    miVector bbox_min, miVector bbox_max);
void miaux_define_hair_object(
    miTag name_tag, miaux_bbox_function bbox_function, void *params,
    miTag *geoshader_result, miApi_object_callback callback);
void miaux_describe_bbox(miObject *obj);
void miaux_adjust_bbox(miObject *obj, miVector *v, miScalar extra);
void miaux_init_bbox(miObject *obj);
void miaux_append_hair_vertex(miScalar **scalar_array, miVector *v);
void miaux_append_hair_data(
    miScalar **scalar_array, miVector *v, miScalar position,
    miScalar root_radius, miColor *root, miScalar tip_radius, miColor *tip );
double miaux_max(double a, double b);
void miaux_copy_color(miColor *result, miColor *color);
void miaux_opacity_set_channels(miState *state, miScalar value);
void miaux_add_transparent_hair_component(miColor *result, miState *state);
void miaux_add_specular_hair_component(
    miColor *result, miVector *hair_tangent, miVector *to_light,
    miVector *to_camera,
    miColor *specular, miColor *light_color);
void miaux_add_diffuse_hair_component(
    miColor *result, miVector *hair_tangent, miVector *to_light,
    miColor *diffuse, miColor *light_color);
double miaux_clamp(double v, double minval, double maxval);
void miaux_clamp_color(miColor *result);
void miaux_point_between(
    miVector *result, miVector *u, miVector *v, float fraction);
void miaux_perpendicular_point(miVector *result, miVector *v, float distance);
void miaux_hair_spiral(
    miScalar** scalar_array, miVector *start_point, miVector *end_point, 
    float turns, int point_count, float angle_offset, float spiral_radius,
    miScalar root_radius, miScalar tip_radius);
void miaux_random_point_on_sphere(
    miVector *result, miVector *center, miScalar radius);
void miaux_read_hair_data_file(char* filename, miScalar radius);
void miaux_hair_data_file_bounding_box(
    char* filename,
    float *xmin, float *ymin, float *zmin, 
    float *xmax, float *ymax, float *zmax);
void miaux_blend_transparency(miColor *result, 
                              miState *state, miColor *transparency);
void miaux_color_fit(miColor *result, 
                     miScalar f, miScalar start, miScalar end,
                     miColor *start_color, miColor *end_color);
miScalar miaux_interpolated_lookup(miScalar lookup_table[], int table_size,
                                   miScalar t);
float miaux_altitude(miState *state);
void miaux_piecewise_sinusoid(
    miScalar result[], int result_count, 
    int key_count, miScalar key_positions[], miScalar key_values[]);
miBoolean miaux_release_user_memory(char* shader_name, miState *state, void *params);
void* miaux_user_memory_pointer(miState *state, int allocation_size);
void miaux_interpolated_color_lookup(miColor* result,
                                     miColor lookup_table[], int table_size,
                                     miScalar t);
void miaux_piecewise_color_sinusoid(
    miColor result[], int result_count, int key_count, miColor key_values[]);
void miaux_point_along_vector(
    miVector *result, miVector *point, miVector *direction, miScalar distance);
void miaux_march_point(
    miVector *result, miState *state, miScalar distance);
void miaux_world_space_march_point(
    miVector *result, miState *state, miScalar distance);
miScalar miaux_threshold_density(
    miVector *point, miVector *center, miScalar radius, 
    miScalar unit_density, miScalar march_increment);
miScalar miaux_density_falloff(
    miVector *point, miVector *center, miScalar radius, 
    miScalar unit_density, miScalar march_increment);
void miaux_alpha_blend_colors(
    miColor *result, miColor *foreground, miColor *background);
void miaux_add_transparent_color(
    miColor *result, miColor *color, miScalar transparency);
void miaux_total_light_at_point(
    miColor *result, miVector *point, miState *state,
    miTag* light, int light_count);
miScalar miaux_fractional_occlusion_at_point(
    miVector *start_point, miVector *direction, 
    miScalar total_distance, miVector *center, miScalar radius, 
    miScalar unit_density, miScalar march_increment);
miScalar miaux_fractional_shader_occlusion_at_point(
    miState *state, miVector *start_point, miVector *direction, 
    miScalar total_distance, miTag density_shader, miTag deform_shader,
    miScalar unit_density, miScalar march_increment);
miBoolean miaux_point_inside(miVector *p, miVector *min_p, miVector *max_p);
void miaux_read_volume_block(
    char* filename, 
    int *width, int *height, int *depth, float* block);
double miaux_distance(double x1, double y1, double x2, double y2);
void miaux_divide_color(miColor *result, miColor* color, miScalar f);
void miaux_from_camera_space(
        miState *state, miVector *origin, miVector *direction);
void miaux_square_to_circle(
    float *result_x, float *result_y, float x, float y, float max_radius);
void miaux_sample_point_within_radius(
    miVector *result, miVector *center, float x, float y, float max_radius);
void miaux_z_plane_intersect(
    miVector *result, miVector *origin, miVector *direction, miScalar z_plane);
void miaux_to_camera_space(
    miState *state, miVector *origin, miVector *direction);
void miaux_divide_colors(miColor *result, miColor *x, miColor *y);
void miaux_add_light_color(
    miColor *result, miState *state, miTag light, char shadow_state);
int miaux_color_compare(const void *vx, const void *vy);
void miaux_pixel_neighborhood(
    miColor *neighbors, 
    miImg_image *buffer, int x, int y, int radius);
void miaux_release_fontimage(fontimage *fimage);
void miaux_alpha_blend(miColor *x, miColor *y, miScalar alpha);
void miaux_text_image(
    float **text_image, int *width, int *height, 
    fontimage *fimage, char* text);
void miaux_load_fontimage(fontimage *fimage, char* filename);
#endif
