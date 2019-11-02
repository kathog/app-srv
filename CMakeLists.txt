cmake_minimum_required (VERSION 3.6)
project(nerull-app-srv)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

#set(GLIBCXX_USE_CXX11_ABI 1)
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GLIBCXX_USE_CXX11_ABI}")
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GLIBCXX_USE_CXX11_ABI}")

IF (WIN32)
set(BOOST_ROOT "C:\\local\\boost_1_70_0")
set(BOOST_LIBRARYDIR "C:\\local\\boost_1_70_0\\stage\\lib")
set(BOOST_INCLUDEDIR "C:\\local\\boost_1_70_0\\boost")
ENDIF()

find_package(Boost REQUIRED)
find_package(Boost REQUIRED regex)
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES (${Boost_LIBRARY_DIRS})

#set(RE2_LIBRARIES -L${RE2_LIBRARY_DIR} -l libre2.so)
#SET (RE2_LIBRARIES /usr/local/lib/libre2.so)
#SET (RE2_STATIC_LIBRARIES /usr/local/lib/libre2.a)

#IF (!WIN32)
set(EXTRA_LIBRARY "-lpthread" "-lboost_regex" "-lpcre")
#ENDIF()
set(LIBRARIES_SEARCH_PATHS ${EXTRA_LIBRARY} ${HEADER_SEARCH_PATHS} ${Boost_LIBRARY})

add_executable (nerull-app-srv "nerull-app-srv.cpp" "src/server.cpp" "src/connection.cpp" "src/connection_manager.cpp" "src/mime_types.cpp" "src/reply.cpp" "src/request_handler.cpp"  "src/request_parser.cpp")

add_library(nerull-app-srv-lib STATIC "src/server.cpp" "src/connection.cpp" "src/connection_manager.cpp" "src/mime_types.cpp" "src/reply.cpp" "src/request_handler.cpp"  "src/request_parser.cpp")

#set_target_properties(boost-asio-lib PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(boost-asio-lib PROPERTIES SOVERSION 1)
#set_target_properties(boost-asio-lib PROPERTIES PUBLIC_HEADER src/server.hpp)
install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}-lib DESTINATION lib/)
file(GLOB HEADERS include/*.hpp)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME} ${LIBRARIES_SEARCH_PATHS})
