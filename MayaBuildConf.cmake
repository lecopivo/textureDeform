# This file sets up few variables for maya
# MAYA_LOCATION = maya location (cached)
# MAYA_INCLUDE_DIRS = maya include directory
# MAYA_LIBRARY_DIRS = maya library directory
# MAYA_PLUG_IN_PATH = maya plugin in directory
# MAYA_C_FLAGS = flags for C compiler
# MAYA_CXX_FLAGS = flags for CXX compiler
# MAYA_LINK_FLAGS = flags for LD linker

if(WIN32)
 message( FATAL_ERROR "Set up windows configuration!" )

elseif(UNIX)
 # Set Maya location
 set( MAYA_LOCATION "/usr/autodesk/maya2013-x64" CACHE STRING "Maya location.")

 # Set Maya plugin directory
 set( MAYA_PLUG_IN_PATH "/home/lecopivo/maya/plugins" CACHE STRING "Maya plugin location.")

 # Set compiler, linker flags
 set (MAYA_C_FLAGS "-DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -fPIC -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wno-deprecated -Wall -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char -Wno-reorder -fno-gnu-keywords -ftemplate-depth-25 -pthread" )
 set (MAYA_CXX_FLAGS "${MAYA_C_FLAGS} -Wno-deprecated -fno-gnu-keywords" )
 set (MAYA_LINK_FLAGS "-Wl,-Bsymbolic" )

endif(WIN32)

set (MAYA_INCLUDE_DIRS ${MAYA_LOCATION}/include/)
set (MAYA_LIBRARY_DIRS ${MAYA_LOCATION}/lib/)

# Simple check of maya location
if(NOT EXISTS ${MAYA_LOCATION}/include/maya/MFn.h)
  message(FATAL_ERROR "Maya could not be found")
endif(NOT EXISTS ${MAYA_LOCATION}/include/maya/MFn.h)