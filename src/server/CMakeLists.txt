PROTOBUF_GENERATE_CPP(MT_PROTO_SOURCES MT_PROTO_HEADERS ${SRC_ROOT}/protobuf/mesh_tile.proto)

SET ( TILE_SERVER_SOURCES

	${SRC_ROOT}/util/base64.cpp
	${SRC_ROOT}/util/base64.hpp
	${SRC_ROOT}/util/database.cpp
	${SRC_ROOT}/util/database.hpp

	${SRC_ROOT}/util/dictionary.cpp
	${SRC_ROOT}/util/dictionary.hpp

	${SRC_ROOT}/server/map_server.cpp
	${SRC_ROOT}/server/map_server.hpp

	${SRC_ROOT}/server/mesh_tile_renderer.cpp
	${SRC_ROOT}/server/mesh_tile_renderer.hpp

	${SRC_ROOT}/map/geom_helpers.cpp
	${SRC_ROOT}/server/tile_server.cpp

	${SRC_ROOT}/server/connection.cpp
	${SRC_ROOT}/server/connection.hpp
	${SRC_ROOT}/server/connection_manager.hpp
	${SRC_ROOT}/server/connection_manager.cpp
	${SRC_ROOT}/server/io_service_pool.cpp
	${SRC_ROOT}/server/io_service_pool.hpp
	${SRC_ROOT}/server/reply.cpp
	${SRC_ROOT}/server/reply.hpp
	${SRC_ROOT}/server/request_handler.hpp
	${SRC_ROOT}/server/request.hpp
	${SRC_ROOT}/server/request_parser.cpp
	${SRC_ROOT}/server/request_parser.hpp
	${SRC_ROOT}/server/server.cpp
	${SRC_ROOT}/server/server.hpp
	${SRC_ROOT}/server/http_parser.c
	${SRC_ROOT}/server/http_parser.h
	${SRC_ROOT}/server/request_handler_factory.hpp
)


ADD_EXECUTABLE(tileserver ${TILE_SERVER_SOURCES} ${MT_PROTO_SOURCES} ${MT_PROTO_HEADERS})
TARGET_LINK_LIBRARIES(tileserver ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PNG_LIBRARIES}
${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} X11 Xxf86vm Xrandr Xi pthread dl z)
