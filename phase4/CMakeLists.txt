cmake_minimum_required(VERSION 3.2)
project(phase4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

FIND_PACKAGE(BISON REQUIRED)
SET(BisonOutput ${CMAKE_SOURCE_DIR}/parser.cpp)
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${BisonOutput}
      COMMAND ${BISON_EXECUTABLE}
              -d -v -y
              --output=${BisonOutput}
              ${CMAKE_SOURCE_DIR}/parser.y
      COMMENT "Generating parser.cpp"
    )
ENDIF()

FIND_PACKAGE(FLEX REQUIRED)
SET(FlexOutput ${CMAKE_SOURCE_DIR}/scanner.cpp)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${FlexOutput}
      COMMAND ${FLEX_EXECUTABLE}
              --outfile=${FlexOutput}
              ${CMAKE_SOURCE_DIR}/scanner.l
      DEPENDS ${BisonOutput}
      COMMENT "Generating scanner.cpp"
    )
ENDIF()

set(SOURCE_FILES
	parser.cpp
    alpha.cpp
    alpha.h
    instruction.h
    quad_to_instruction.cpp
    quad_to_instruction.h
    SymbolTable.cpp
    SymbolTable.h)

add_executable(al ${SOURCE_FILES} ${BisonOutput} ${FlexOutput})