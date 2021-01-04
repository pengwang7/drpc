## libev CMake module file

find_path(LIBEV_INCLUDE ev.h /usr/include /usr/local/include)
find_library(ADD_LIBRARY NAMES ev PATHS /usr/lib /usr/local/lib)

if (LIBEV_INCLUDE AND ADD_LIBRARY)
    set(LIBEV_FOUND TRUE)
endif()
