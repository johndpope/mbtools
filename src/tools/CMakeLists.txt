SET ( RSC_TOOL_SOURCES
	${SRC_ROOT}/util/database.cpp
	${SRC_ROOT}/util/database.hpp
	${SRC_ROOT}/util/dictionary.cpp
	${SRC_ROOT}/util/dictionary.hpp
	${SRC_ROOT}/tools/makeres.cpp
)

ADD_EXECUTABLE(makeres ${RSC_TOOL_SOURCES})
TARGET_LINK_LIBRARIES(makeres ${SQLITE3_LIBRARY} ${Boost_LIBRARIES} pthread dl z)
