INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${SQLITE3_INCLUDE_DIR}
	${SPATIALITE_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${PROTOBUF_INCLUDE_DIRS}
	${GLFW_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/util
	${CMAKE_CURRENT_SOURCE_DIR}/server
	${CMAKE_CURRENT_SOURCE_DIR}/osm
	${CMAKE_CURRENT_SOURCE_DIR}/map
	${CMAKE_CURRENT_SOURCE_DIR}/vector
	${CMAKE_CURRENT_SOURCE_DIR}/mesh
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vcglib
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/triangle

)

IF ( WIN32 )
ADD_DEFINITIONS( -DBOOST_USE_WINDOWS_H )
SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
ENDIF( WIN32 )

ADD_DEFINITIONS( -std=c++11 )

find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

SET ( OSM2MBTILES_SOURCES

	vector/mb_tile_writer.cpp
	vector/vector_tile_writer.cpp

	osm/osm2mbtiles.cpp

	osm/import_config.cpp
	osm/osm_rule_parser.cpp
	osm/osm_filter_functions.cpp
	osm/osm_processor.cpp
	osm/osm_polygon.cpp
	osm/osm_pbf_reader.cpp
	osm/osm_document.cpp

	map/map_file.cpp
	map/geom_helpers.cpp
	map/map_config.cpp

	util/dictionary.cpp
	util/xml_reader.cpp
	util/database.cpp
	util/zfstream.cpp
	util/base64.cpp
)

SET ( OSM2MBTILES_HEADERS

	vector/vector_tile_writer.hpp
	vector/mb_tile_writer.hpp

	osm/import_config.hpp
	osm/osm_filter_functions.hpp
	osm/osm_document.hpp
	osm/osm_rule_parser.hpp

	map/map_config.hpp
	map/map_file.hpp
	map/geom_helpers.hpp

	util/dictionary.hpp
	util/xml_reader.hpp
	util/database.hpp
	util/zfstream.hpp
	util/base64.h
)

PROTOBUF_GENERATE_CPP(OSM_PROTO_SOURCES OSM_PROTO_HEADERS protobuf/osmformat.proto protobuf/fileformat.proto)
PROTOBUF_GENERATE_CPP(VT_PROTO_SOURCES VT_PROTO_HEADERS protobuf/vector_tile.proto)
PROTOBUF_GENERATE_CPP(MT_PROTO_SOURCES MT_PROTO_HEADERS protobuf/mesh_tile.proto)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

FLEX_TARGET(OSM_FILTER_SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/osm/osm.l  ${CMAKE_CURRENT_SOURCE_DIR}/parser/osm_scanner.cpp)
BISON_TARGET(OSM_FILTER_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/osm/osm.y  ${CMAKE_CURRENT_SOURCE_DIR}/parser/osm_parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(OSM_FILTER_SCANNER OSM_FILTER_PARSER)

LIST(APPEND OSM2MBTILES_SOURCES ${FLEX_OSM_FILTER_SCANNER_OUTPUTS} ${BISON_OSM_FILTER_PARSER_OUTPUTS}
	${OSM_PROTO_SOURCES} ${OSM_PROTO_HEADERS}
	${VT_PROTO_SOURCES} ${VT_PROTO_HEADERS}
)

###############################################################################
ADD_EXECUTABLE(osm2mbtiles ${OSM2MBTILES_SOURCES})
TARGET_LINK_LIBRARIES(osm2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} ${Boost_LIBRARIES})

IF ( WIN32 )
TARGET_LINK_LIBRARIES(osm2mbtiles ws2_32)
ENDIF( WIN32 )

INSTALL(TARGETS osm2mbtiles DESTINATION bin  )

##################################################################################3

SET ( SHP2MBTILES_SOURCES
	vector/mb_tile_writer.cpp
	vector/vector_tile_writer.cpp

	map/map_config.cpp
	map/map_file.cpp
	map/geom_helpers.cpp

	util/dictionary.cpp
	util/database.cpp

	shp/shp2mbtiles.cpp
	shp/shp_processor.cpp
)

SET ( SHP2MBTILES_HEADERS
	vector/vector_tile_writer.hpp
	vector/mb_tile_writer.hpp

	map/map_config.hpp
	map/geom_helpers.hpp
	map/map_file.hpp

	util/dictionary.h
	util/database.h
)

LIST(APPEND SHP2MBTILES_SOURCES ${VT_PROTO_SOURCES} ${VT_PROTO_HEADERS})

ADD_EXECUTABLE(shp2mbtiles  ${SHP2MBTILES_SOURCES} )
TARGET_LINK_LIBRARIES(shp2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} ${SHAPELIB_LIBRARY} ${Boost_LIBRARIES})


######################################################################################

SET ( RSC_TOOL_SOURCES
	util/database.cpp
	util/database.hpp
	util/dictionary.cpp
	util/dictionary.hpp
	tools/makeres.cpp
)

ADD_EXECUTABLE(makeres ${RSC_TOOL_SOURCES})
TARGET_LINK_LIBRARIES(makeres ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} pthread dl z)

##################################################################################3

ADD_DEFINITIONS(-DSELF_CHECK -DLINUX -DTRILIBRARY)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/vcglib)
ADD_EXECUTABLE(xyz2mesh mesh/xyz2mesh.cpp 3rdparty/triangle/triangle.c mesh/mesh.cpp)
TARGET_LINK_LIBRARIES(xyz2mesh  ${Boost_LIBRARIES})


##################################################################################3

SET ( MESH2MBTILES_SOURCES
	3rdparty/triangle/triangle.c
	mesh/mesh.cpp

	mesh/mesh_tileset_writer.cpp
	mesh/mesh_tile_writer.cpp

	map/map_config.cpp
	map/geom_helpers.cpp

	util/database.cpp
)

SET ( MESH2MBTILES_HEADERS

	mesh/mesh.hpp
	mesh/mesh_tile_writer.hpp
	mesh/mesh_tileset_writer.h
	
	map/map_config.hpp
	util/database.hpp

	mesh/mesh2mbtiles.cpp
)

LIST(APPEND MESH2MBTILES_SOURCES ${MT_PROTO_SOURCES} ${MT_PROTO_HEADERS})

ADD_EXECUTABLE(mesh2mbtiles ${MESH2MBTILES_SOURCES} )
TARGET_LINK_LIBRARIES(mesh2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} ${PNG_LIBRARIES}
${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} X11 Xxf86vm Xrandr pthread Xi )

########################################################################################

SET ( TILE_SERVER_SOURCES

	util/base64.cpp
	util/base64.hpp
	util/database.cpp
	util/database.hpp

	util/dictionary.cpp
	util/dictionary.hpp

	server/map_server.cpp
	server/map_server.hpp

	server/mesh_tile_renderer.cpp
	server/mesh_tile_renderer.hpp

	map/geom_helpers.cpp
	server/tile_server.cpp

	server/connection.cpp
	server/connection.hpp
	server/connection_manager.hpp
	server/connection_manager.cpp
	server/io_service_pool.cpp
	server/io_service_pool.hpp
	server/reply.cpp
	server/reply.hpp
	server/request_handler.hpp
	server/request.hpp
	server/request_parser.cpp
	server/request_parser.hpp
	server/server.cpp
	server/server.hpp
	server/http_parser.c
	server/http_parser.h
	server/request_handler_factory.hpp
)


ADD_EXECUTABLE(tileserver ${TILE_SERVER_SOURCES} ${MT_PROTO_SOURCES} ${MT_PROTO_HEADERS})
TARGET_LINK_LIBRARIES(tileserver ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PNG_LIBRARIES}
${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} X11 Xxf86vm Xrandr Xi pthread dl z)
