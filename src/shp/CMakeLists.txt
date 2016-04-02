
PROTOBUF_GENERATE_CPP(VT_PROTO_SOURCES VT_PROTO_HEADERS ${SRC_ROOT}/protobuf/vector_tile.proto)

SET ( SHP2MBTILES_SOURCES
	${SRC_ROOT}/vector/mb_tile_writer.cpp
	${SRC_ROOT}/vector/vector_tile_writer.cpp

	${SRC_ROOT}/map/map_config.cpp
	${SRC_ROOT}/map/map_file.cpp
	${SRC_ROOT}/map/geom_helpers.cpp

	${SRC_ROOT}/util/dictionary.cpp
	${SRC_ROOT}/util/database.cpp

	${SRC_ROOT}/shp/shp2mbtiles.cpp
	${SRC_ROOT}/shp/shp_processor.cpp

	${SRC_ROOT}/vector/vector_tile_writer.hpp
	${SRC_ROOT}/vector/mb_tile_writer.hpp

	${SRC_ROOT}/map/map_config.hpp
	${SRC_ROOT}/map/geom_helpers.hpp
	${SRC_ROOT}/map/map_file.hpp

	${SRC_ROOT}/util/dictionary.hpp
	${SRC_ROOT}/util/database.hpp
)

LIST(APPEND SHP2MBTILES_SOURCES ${VT_PROTO_SOURCES} ${VT_PROTO_HEADERS})

ADD_EXECUTABLE(shp2mbtiles  ${SHP2MBTILES_SOURCES} )
TARGET_LINK_LIBRARIES(shp2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} ${SHAPELIB_LIBRARY} ${Boost_LIBRARIES})


