# Findshaderc.Cmake
# Locate shaderc library
# This module defines
#  SHADERC_FOUND, SHADERC_LIBRARIES, SHADERC_INCLUDE_DIRS

find_path(SHADERC_INCLUDE_DIR shaderc/shaderc.hpp
        PATHS
        $ENV{SHADERC_INCLUDE_DIR} # Allows the use of an environment variable
)

find_library(SHADERC_LIBRARY NAMES shaderc
        PATHS
        $ENV{SHADERC_LIBRARY_DIR} # Allows the use of an environment variable
)

find_library(SHADERC_COMBINED_LIBRARY NAMES shaderc_combined
        PATHS
        $ENV{SHADERC_LIBRARY_DIR} # Allows the use of an environment variable
)

if(SHADERC_INCLUDE_DIR AND SHADERC_LIBRARY AND SHADERC_COMBINED_LIBRARY)
    set(SHADERC_FOUND TRUE)
    set(SHADERC_LIBRARIES ${SHADERC_LIBRARY} ${SHADERC_COMBINED_LIBRARY})
    set(SHADERC_INCLUDE_DIRS ${SHADERC_INCLUDE_DIR})
else()
    set(SHADERC_FOUND FALSE)
endif()

mark_as_advanced(SHADERC_INCLUDE_DIR SHADERC_LIBRARY SHADERC_COMBINED_LIBRARY)