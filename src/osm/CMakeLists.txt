SET ( OSM2MBTILES_SOURCES
	${SRC_ROOT}/vector/mb_tile_writer.cpp
	${SRC_ROOT}/vector/vector_tile_writer.cpp

	${SRC_ROOT}/osm/osm2mbtiles.cpp

	${SRC_ROOT}/osm/import_config.cpp
	${SRC_ROOT}/osm/osm_rule_parser.cpp
	${SRC_ROOT}/osm/osm_filter_functions.cpp
	${SRC_ROOT}/osm/osm_processor.cpp
	${SRC_ROOT}/osm/osm_polygon.cpp
	${SRC_ROOT}/osm/osm_pbf_reader.cpp
	${SRC_ROOT}/osm/osm_document.cpp

	${SRC_ROOT}/map/map_file.cpp
	${SRC_ROOT}/map/geom_helpers.cpp
	${SRC_ROOT}/map/map_config.cpp

	${SRC_ROOT}/util/dictionary.cpp
	${SRC_ROOT}/util/xml_reader.cpp
	${SRC_ROOT}/util/database.cpp
	${SRC_ROOT}/util/zfstream.cpp
	${SRC_ROOT}/util/base64.cpp

	${SRC_ROOT}/vector/vector_tile_writer.hpp
	${SRC_ROOT}/vector/mb_tile_writer.hpp

	${SRC_ROOT}/osm/import_config.hpp
	${SRC_ROOT}/osm/osm_filter_functions.hpp
	${SRC_ROOT}/osm/osm_document.hpp
	${SRC_ROOT}/osm/osm_rule_parser.hpp

	${SRC_ROOT}/map/map_config.hpp
	${SRC_ROOT}/map/map_file.hpp
	${SRC_ROOT}/map/geom_helpers.hpp

	${SRC_ROOT}/util/dictionary.hpp
	${SRC_ROOT}/util/xml_reader.hpp
	${SRC_ROOT}/util/database.hpp
	${SRC_ROOT}/util/zfstream.hpp
	${SRC_ROOT}/util/base64.hpp
)

PROTOBUF_GENERATE_CPP(OSM_PROTO_SOURCES OSM_PROTO_HEADERS ${SRC_ROOT}/protobuf/osmformat.proto ${SRC_ROOT}/protobuf/fileformat.proto)
PROTOBUF_GENERATE_CPP(VT_PROTO_SOURCES VT_PROTO_HEADERS ${SRC_ROOT}/protobuf/vector_tile.proto)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

FLEX_TARGET(OSM_FILTER_SCANNER ${SRC_ROOT}/osm/osm.l  ${SRC_ROOT}/osm/parser/osm_scanner.cpp)
BISON_TARGET(OSM_FILTER_PARSER ${SRC_ROOT}/osm/osm.y  ${SRC_ROOT}/osm/parser/osm_parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(OSM_FILTER_SCANNER OSM_FILTER_PARSER)

LIST(APPEND OSM2MBTILES_SOURCES ${FLEX_OSM_FILTER_SCANNER_OUTPUTS} ${BISON_OSM_FILTER_PARSER_OUTPUTS}
	${OSM_PROTO_SOURCES} ${OSM_PROTO_HEADERS}
	${VT_PROTO_SOURCES} ${VT_PROTO_HEADERS}
)

ADD_EXECUTABLE(osm2mbtiles ${OSM2MBTILES_SOURCES})
TARGET_LINK_LIBRARIES(osm2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} ${Boost_LIBRARIES})

INSTALL(TARGETS osm2mbtiles DESTINATION bin  )
