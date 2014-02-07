cmake_minimum_required (VERSION 2.6)
project (mentalRayTemplate)

# Find Maya and Mental ray
include( MentalRayBuildConf.cmake )
include( MayaBuildConf.cmake )


add_subdirectory( kernel )
add_subdirectory( deformers )
add_subdirectory( textures )

