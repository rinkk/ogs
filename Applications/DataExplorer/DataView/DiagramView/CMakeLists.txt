set(SOURCES
	DetailWindow.cpp
	DiagramList.cpp
	DiagramPrefsDialog.cpp
	DiagramScene.cpp
	DiagramView.cpp
	QArrow.cpp
	QGraphicsGrid.cpp
)

set(HEADERS
	DiagramList.h
	QArrow.h
	QGraphicsGrid.h
	DiagramScene.h
	DetailWindow.h
	DiagramPrefsDialog.h
	DiagramView.h
)

set(SOURCE_DIR_REL ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(GUI_SOURCE_DIR_REL ${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories(
	${SOURCE_DIR_REL}/BaseLib
	${SOURCE_DIR_REL}/GeoLib
	${SOURCE_DIR_REL}/MathLib
	${GUI_SOURCE_DIR_REL}/Base
	${GUI_SOURCE_DIR_REL}/DataView
	${CMAKE_CURRENT_BINARY_DIR}/../../DataView
	${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB_RECURSE UIS *.ui)
source_group("UI Files" FILES ${UIS})

add_library(QtDiagramView STATIC
	${SOURCES}
	${HEADERS}
	${UIS}
)

target_link_libraries(QtDiagramView PUBLIC Qt4::QtGui)

set_property(TARGET QtDiagramView PROPERTY FOLDER "DataExplorer")
