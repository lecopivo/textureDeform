# This file sets up few variables for maya
# MR_LOCATION = mental ray location (cached)
# MR_INCLUDE_DIRS = mental ray include directory
# MR_SHADER_PATH  = mental ray plugin in directory
# MR_C_FLAGS = flags for C compiler
# MR_CXX_FLAGS = flags for CXX compiler

if(WIN32)
 message( FATAL_ERROR "Set up windows configuration for Mental ray!" )

elseif(UNIX)
 # Set Maya location
 set( MR_LOCATION "/usr/autodesk/maya2013-x64/mentalray" CACHE STRING "Mental Ray location.")

 # Set Maya plugin directory
 set( MR_SHADER_PATH  "/home/lecopivo/maya/shaders" CACHE STRING "Mental Ray shaders location.")

set (MR_C_FLAGS "-ansi -pedantic -Wall -fPIC -fno-common -fpermissive")
set (MR_CXX_FLAGS "${MR_C_FLAGS}")
set (MR_LINK_FLAGS " ")

endif(WIN32)

set (MR_INCLUDE_DIRS ${MR_LOCATION}/devkit/include)

# Simple check of mental ray location
if(NOT EXISTS ${MR_INCLUDE_DIRS}/shader.h)
  message(FATAL_ERROR "Mental Ray could not be found")
endif(NOT EXISTS ${MR_INCLUDE_DIRS}/shader.h)
