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
	ImportConfig.cpp
	MBTileWriter.cpp
	OSMFilterFunctions.cpp
	OSMProcessor.cpp

	VectorTileWriter.cpp
	GeomHelpers.cpp

	Dictionary.cpp
	XmlReader.cpp
	XmlDocument.cpp
	Database.cpp
	zfstream.cpp
	base64.cpp
)

SET ( OSM2MBTILES_HEADERS
	OsmRuleParser.h
	OsmDocument.h
	VectorTileWriter.h
	MapFile.h
	MapConfig.h
	ImportConfig.h
	GeomHelpers.h
	MBTileWriter.h
	OSMFilterFunctions.h

	Dictionary.h
	XmlReader.h
	XmlDocument.h
	Database.h
	zfstream.h
	base64.h
)

PROTOBUF_GENERATE_CPP(OSM_PROTO_SOURCES OSM_PROTO_HEADERS protobuf/osmformat.proto protobuf/fileformat.proto)
PROTOBUF_GENERATE_CPP(VT_PROTO_SOURCES VT_PROTO_HEADERS protobuf/vector_tile.proto)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

FLEX_TARGET(OSM_FILTER_SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/osm.l  ${CMAKE_CURRENT_SOURCE_DIR}/parser/osm_scanner.cpp)
BISON_TARGET(OSM_FILTER_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/osm.y  ${CMAKE_CURRENT_SOURCE_DIR}/parser/osm_parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(OSM_FILTER_SCANNER OSM_FILTER_PARSER)

LIST(APPEND OSM2MBTILES_SOURCES ${FLEX_OSM_FILTER_SCANNER_OUTPUTS} ${BISON_OSM_FILTER_PARSER_OUTPUTS}
	${OSM_PROTO_SOURCES} ${OSM_PROTO_HEADERS}
	${VT_PROTO_SOURCES} ${VT_PROTO_HEADERS}
)

###############################################################################
ADD_EXECUTABLE(osm2mbtiles osm2mbtiles.cpp ${OSM2MBTILES_SOURCES})
TARGET_LINK_LIBRARIES(osm2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} ${Boost_LIBRARIES})

IF ( WIN32 )
TARGET_LINK_LIBRARIES(osm2mbtiles ws2_32)
ENDIF( WIN32 )

INSTALL(TARGETS osm2mbtiles DESTINATION bin  )

##################################################################################3

SET ( SHP2MBTILES_SOURCES
	MapFile.cpp
	MapConfig.cpp
	SHPProcessor.cpp
	MBTileWriter.cpp
	VectorTileWriter.cpp
	GeomHelpers.cpp
	Dictionary.cpp
	Database.cpp
)

SET ( SHP2MBTILES_HEADERS
	VectorTileWriter.h
	MapFile.h
	MapConfig.h
	GeomHelpers.h
	MBTileWriter.h
	Dictionary.h
	Database.h
)

LIST(APPEND SHP2MBTILES_SOURCES ${VT_PROTO_SOURCES} ${VT_PROTO_HEADERS})

ADD_EXECUTABLE(shp2mbtiles shp2mbtiles.cpp ${SHP2MBTILES_SOURCES} )
TARGET_LINK_LIBRARIES(shp2mbtiles ${PROTOBUF_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} ${SHAPELIB_LIBRARY} ${Boost_LIBRARIES})


######################################################################################

SET ( RSC_TOOL_SOURCES
	Database.cpp
	Dictionary.cpp
	makeres.cpp
)

SET ( RSC_TOOL_HEADERS
	Database.h
	Dictionary.h
)

ADD_EXECUTABLE(makeres ${RSC_TOOL_SOURCES})
TARGET_LINK_LIBRARIES(makeres ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} pthread dl z)

##################################################################################3

SET ( TILE_SERVER_SOURCES
	mongoose/mongoose.c
	base64.cpp
	Database.cpp
	Dictionary.cpp
	HttpServer.cpp
	MapServer.cpp
	tileserver.cpp
)

SET ( TILE_SERVER_HEADERS
	mongoose/mongoose.h
	Database.h
	Dictionary.h
	HttpServer.h
	MapServer.h

)

ADD_EXECUTABLE(tileserver ${TILE_SERVER_SOURCES})
TARGET_LINK_LIBRARIES(tileserver ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} pthread dl z)
