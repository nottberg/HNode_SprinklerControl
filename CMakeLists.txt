CMAKE_MINIMUM_REQUIRED( VERSION 2.8 ) 
PROJECT( hnode_sprinkler_controller ) 

SET( HNODE_SPRINKLER_CTRL_SRC
     ${CMAKE_SOURCE_DIR}/src/hnode.c
     ${CMAKE_SOURCE_DIR}/src/ilink-obj.c
     ${CMAKE_SOURCE_DIR}/src/i2c_mcp23008.c
)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src )

ADD_EXECUTABLE( hnode_sprinkler_controller_daemon ${HNODE_SPRINKLER_CTRL_SRC} ) 

