include(FindPkgConfig)

pkg_check_modules(HNODE_PKG glib-2.0)

if (HNODE_PKG_FOUND)
    find_path(HNODE_INCLUDE_DIR  NAMES glib.h PATH_SUFFIXES glib-2.0
       PATHS
       ${HNODE_PKG_INCLUDE_DIRS}
       /usr/include/glib-2.0
       /usr/include
       /usr/local/include
    )
    find_path(HNODE_CONFIG_INCLUDE_DIR NAMES glibconfig.h PATHS ${HNODE_PKG_LIBDIR} PATH_SUFFIXES glib-2.0/include)

    find_library(HNODE_LIBRARIES NAMES glib-2.0
       PATHS
       ${HNODE_PKG_LIBRARY_DIRS}
       /usr/lib
       /usr/local/lib
    )

else (HNODE_PKG_FOUND)
    # Find Glib even if pkg-config is not working (eg. cross compiling to Windows)
    find_library(HNODE_LIBRARIES NAMES glib-2.0)
    string (REGEX REPLACE "/[^/]*$" "" HNODE_LIBRARIES_DIR ${HNODE_LIBRARIES})

    find_path(HNODE_INCLUDE_DIR NAMES glib.h PATH_SUFFIXES glib-2.0)
    find_path(HNODE_CONFIG_INCLUDE_DIR NAMES glibconfig.h PATHS ${HNODE_LIBRARIES_DIR} PATH_SUFFIXES glib-2.0/include)

endif (HNODE_PKG_FOUND)

if (HNODE_INCLUDE_DIR AND HNODE_CONFIG_INCLUDE_DIR AND HNODE_LIBRARIES)
    set(HNODE_INCLUDE_DIRS ${HNODE_INCLUDE_DIR} ${HNODE_CONFIG_INCLUDE_DIR})
endif (HNODE_INCLUDE_DIR AND HNODE_CONFIG_INCLUDE_DIR AND HNODE_LIBRARIES)

if(HNODE_INCLUDE_DIRS AND HNODE_LIBRARIES)
   set(HNODE_FOUND TRUE CACHE INTERNAL "glib-2.0 found")
   message(STATUS "Found glib-2.0: ${HNODE_INCLUDE_DIR}, ${HNODE_LIBRARIES}")
else(HNODE_INCLUDE_DIRS AND HNODE_LIBRARIES)
   set(HNODE_FOUND FALSE CACHE INTERNAL "glib-2.0 found")
   message(STATUS "glib-2.0 not found.")
endif(HNODE_INCLUDE_DIRS AND HNODE_LIBRARIES)

mark_as_advanced(HNODE_INCLUDE_DIR HNODE_CONFIG_INCLUDE_DIR HNODE_INCLUDE_DIRS HNODE_LIBRARIES)


