# source filesHelloWorld
set (MR_SRC kernel.cpp miaux.cpp)

# mr .mi files
set (MR_MI_FILES kernel.mi)

# target
set (MR_TARGET kernel)

# set up flags
set_source_files_properties( ${MR_SRC} 
  PROPERTIES
  COMPILE_FLAGS ${MR_CXX_FLAGS}
  )

#find_package(Maya REQUIRED)
include_directories ("${MR_INCLUDE_DIRS}")

# add the executable
add_library (${MR_TARGET} SHARED ${MR_SRC})

set_target_properties( ${MR_TARGET}
  PROPERTIES
  PREFIX ""
  LINK_FLAGS ${MR_LINK_FLAGS}
)

install ( TARGETS ${MR_TARGET} DESTINATION ${MR_SHADER_PATH} )
install ( FILES ${MR_MI_FILES} DESTINATION ${MR_SHADER_PATH} )