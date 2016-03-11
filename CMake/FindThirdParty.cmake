FIND_PACKAGE(Sqlite3 REQUIRED)
FIND_PACKAGE(SPATIALITE REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(PROJ4)

# Boost

# -DBoost_NO_BOOST_CMAKE=ON  -DBoost_NO_SYSTEM_PATHS=ON -DBOOST_ROOT=<local boost install>
set(Boost_ADDITIONAL_VERSIONS "1.49" "1.50.0" "1.51" "1.52" "1.53" "1.54")
set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

set(BOOST_COMPONENTS regex filesystem system serialization)

#IF ( WIN32 )
#    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} thread_win32)
#ELSE( WIN32 )
    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} thread)
#ENDIF( WIN32 )

FIND_PACKAGE(Boost 1.49 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

# Find the QtWidgets library
find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5WebKit)
find_package(Qt5WebKitWidgets)
find_package(Qt5Widgets)


