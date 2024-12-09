# Findjemalloc.Cmake

# Locate jemalloc library
# This module defines
#  JEMALLOC_FOUND, JEMALLOC_LIBRARIES, JEMALLOC_INCLUDE_DIRS

find_path(JEMALLOC_INCLUDE_DIR jemalloc/jemalloc.h)

find_library(JEMALLOC_LIBRARY NAMES jemalloc)

if(JEMALLOC_INCLUDE_DIR AND JEMALLOC_LIBRARY)
    set(JEMALLOC_FOUND TRUE)
    set(JEMALLOC_LIBRARIES ${JEMALLOC_LIBRARY})
    set(JEMALLOC_INCLUDE_DIRS ${JEMALLOC_INCLUDE_DIR})
else()
    set(JEMALLOC_FOUND FALSE)
endif()

mark_as_advanced(JEMALLOC_INCLUDE_DIR JEMALLOC_LIBRARY)