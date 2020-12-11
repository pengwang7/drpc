## libev CMake module file

find_path(LIBEV_INCLUDE ev.h /usr/include /usr/local/include)
find_library(ADD_LIBRARY NAMES ev PATHS /usr/lib/libev /usr/local/lib/libev)

if (LIBEV_INCLUDE AND ADD_LIBRARY)
    set(LIBEV_FOUND TRUE)
endif()
