
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${SQLITE3_INCLUDE_DIR}
	${SPATIALITE_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${PROTOBUF_INCLUDE_DIRS}
)

IF ( WIN32 )
ADD_DEFINITIONS( -DBOOST_USE_WINDOWS_H )
SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
ENDIF( WIN32 )

ADD_DEFINITIONS( -std=c++11 )

SET ( OSM2MBTILES_SOURCES
	OsmRuleParser.cpp
	OsmPBFReader.cpp
	OsmDocument.cpp
	OsmPolygon.cpp
	MapFile.cpp
	MapConfig.cpp

	VectorTile.cpp
	GeomHelpers.cpp

	Dictionary.cpp
	XmlReader.cpp
	XmlDocument.cpp
	Database.cpp
	zfstream.cpp
)

SET ( OSM2MBTILES_HEADERS
	OsmRuleParser.h
	OsmDocument.h
	VectorTile.h
	MapFile.h
	MapConfig.h
	GeomHelpers.h

	Dictionary.h
	XmlReader.h
	XmlDocument.h
	Database.h
	zfstream.h
)

PROTOBUF_GENERATE_CPP(OSM_PROTO_SOURCES OSM_PROTO_HEADERS protobuf/osmformat.proto protobuf/fileformat.proto)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

FLEX_TARGET(OSM_FILTER_SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/osm.l  ${CMAKE_CURRENT_SOURCE_DIR}/parser/osm_scanner.cpp)
BISON_TARGET(OSM_FILTER_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/osm.y  ${CMAKE_CURRENT_SOURCE_DIR}/parser/osm_parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(OSM_FILTER_SCANNER OSM_FILTER_PARSER)

LIST(APPEND OSM2MBTILES_SOURCES ${FLEX_OSM_FILTER_SCANNER_OUTPUTS} ${BISON_OSM_FILTER_PARSER_OUTPUTS} ${OSM_PROTO_SOURCES} ${OSM_PROTO_HEADERS})

###############################################################################
ADD_EXECUTABLE(osm2mbtiles osm2mbtiles.cpp ${OSM2MBTILES_SOURCES})
TARGET_LINK_LIBRARIES(osm2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY}  ${PROJ4_LIBRARY} ${Boost_LIBRARIES})

IF ( WIN32 )
TARGET_LINK_LIBRARIES(osm2mbtiles ws2_32)
ENDIF( WIN32 )

INSTALL(TARGETS osm2mbtiles DESTINATION bin  )


