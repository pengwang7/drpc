## libev CMake module file

find_path(LIBEV_INCLUDE ev.h /usr/local/include /usr/include)
find_library(LIBEV_LIBRARY NAMES ev PATHS /usr/local/lib /usr/lib)

if (LIBEV_INCLUDE AND LIBEV_LIBRARY)
    set(LIBEV_FOUND TRUE)
endif()
