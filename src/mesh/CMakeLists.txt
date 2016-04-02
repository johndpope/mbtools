PROTOBUF_GENERATE_CPP(MT_PROTO_SOURCES MT_PROTO_HEADERS ${SRC_ROOT}/protobuf/mesh_tile.proto)

ADD_DEFINITIONS(-DSELF_CHECK -DLINUX -DTRILIBRARY)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/vcglib)
ADD_EXECUTABLE(xyz2mesh ${SRC_ROOT}/mesh/xyz2mesh.cpp 
			${SRC_ROOT}/3rdparty/triangle/triangle.c 
			${SRC_ROOT}/mesh/mesh.cpp)
TARGET_LINK_LIBRARIES(xyz2mesh  ${Boost_LIBRARIES})

SET ( MESH2MBTILES_SOURCES
	${SRC_ROOT}/3rdparty/triangle/triangle.c
	${SRC_ROOT}/mesh/mesh.cpp

	${SRC_ROOT}/mesh/mesh_tileset_writer.cpp
	${SRC_ROOT}/mesh/mesh_tile_writer.cpp

	${SRC_ROOT}/map/map_config.cpp
	${SRC_ROOT}/map/geom_helpers.cpp

	${SRC_ROOT}/util/database.cpp

        ${SRC_ROOT}/mesh/mesh2mbtiles.cpp

	${SRC_ROOT}/mesh/mesh.hpp
	${SRC_ROOT}/mesh/mesh_tile_writer.hpp
	${SRC_ROOT}/mesh/mesh_tileset_writer.hpp
	
	${SRC_ROOT}/map/map_config.hpp
	${SRC_ROOT}/util/database.hpp
)

LIST(APPEND MESH2MBTILES_SOURCES ${MT_PROTO_SOURCES} ${MT_PROTO_HEADERS})

ADD_EXECUTABLE(mesh2mbtiles ${MESH2MBTILES_SOURCES} )
TARGET_LINK_LIBRARIES(mesh2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} ${PNG_LIBRARIES}
${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} X11 Xxf86vm Xrandr pthread Xi )


