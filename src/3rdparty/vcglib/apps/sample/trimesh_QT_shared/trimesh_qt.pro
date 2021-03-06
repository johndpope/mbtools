# Base options
TEMPLATE = app
LANGUAGE  = C++

# QT modules
QT += opengl

# Executable name
TARGET = trimesh_qt

# Directories
DESTDIR = .
UI_DIR = build/ui
MOC_DIR = build/moc
OBJECTS_DIR = build/obj

# Lib headers
INCLUDEPATH += .
INCLUDEPATH += ../../..

# Lib sources
SOURCES += ../../../wrap/ply/plylib.cpp
SOURCES += ../../../wrap/gui/trackball.cpp
SOURCES += ../../../wrap/gui/trackmode.cpp


# Compile glew
DEFINES += GLEW_STATIC
INCLUDEPATH += ../../../../lib/glew/include
SOURCES += ../../../../lib/glew/src/glew.c

# Awful problem with windows..
win32{
  DEFINES += NOMINMAX
}

# Input
HEADERS += mainwindow.h
HEADERS += glarea.h
HEADERS += ml_thread_safe_memory_info.h
HEADERS += ml_scene_renderer.h
HEADERS += ml_atomic_guard.h



SOURCES += main.cpp
SOURCES += mainwindow.cpp
SOURCES += glarea.cpp
SOURCES += ml_thread_safe_memory_info.cpp
SOURCES += ml_scene_renderer.cpp

FORMS += mainwindow.ui

linux-g++:LIBS += -lGLU
linux-g++-32:LIBS += -lGLU
linux-g++-64:LIBS += -lGLU
