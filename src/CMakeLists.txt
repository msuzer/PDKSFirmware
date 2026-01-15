# Project component: collect C sources only and set include path to src/

file(GLOB_RECURSE app_sources
	${CMAKE_CURRENT_LIST_DIR}/*.c
)

idf_component_register(
	SRCS ${app_sources}
	INCLUDE_DIRS . include
)
