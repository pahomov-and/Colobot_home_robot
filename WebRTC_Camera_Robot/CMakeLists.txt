cmake_minimum_required(VERSION 3.0)
project(WebRTC_Camera)

set(CMAKE_CXX_STANDARD 17)

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(GLIB "glib-2.0")
    if (GLIB_FOUND)
        #        target_link_libraries(${PROJECT_NAME} ${GLIB_LIBRARIES})
        message(${GLIB_LIBRARIES})
        add_definitions(${GLIB_CFLAGS} ${GLIB_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(GST "gstreamer-1.0")
    if (GST_FOUND)
        #        target_link_libraries(${PROJECT_NAME} ${GST_LIBRARIES})
        message(${GST_LIBRARIES})
        add_definitions(${GST_CFLAGS} ${GST_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(SPD "gstreamer-sdp-1.0")
    if (SPD_FOUND)
        #        target_link_libraries(${PROJECT_NAME} ${SPD_LIBRARIES})
        message(${SPD_LIBRARIES})
        add_definitions(${SPD_CFLAGS} ${SPD_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(WEBRTC "gstreamer-webrtc-1.0")
    if (WEBRTC_FOUND)
        #        target_link_libraries(${PROJECT_NAME} ${WEBRTC_LIBRARIES})
        message(${WEBRTC_LIBRARIES})
        add_definitions(${WEBRTC_CFLAGS} ${WEBRTC_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(JSON "json-glib-1.0")
    if (JSON_FOUND)
        #        target_link_libraries(${PROJECT_NAME} ${JSON_LIBRARIES})
        message(${JSON_LIBRARIES})
        add_definitions(${JSON_CFLAGS} ${JSON_CFLAGS_OTHER})
    endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(SOUP "libsoup-2.4")
    if (SOUP_FOUND)
        #        target_link_libraries(${PROJECT_NAME} ${SOUP_LIBRARIES})
        message(${SOUP_LIBRARIES})
        add_definitions(${SOUP_CFLAGS} ${SOUP_CFLAGS_OTHER})
    endif()
endif()


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Modules/include)

FILE(GLOB SRC
        ./Modules/src/*.cpp
        )

add_executable(${PROJECT_NAME}
        main.cpp
        ${SRC}
        Modules/src/Utills.cpp Modules/include/Utills.h)

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        ${GLIB_LIBRARIES}
        ${GST_LIBRARIES}
        ${SPD_LIBRARIES}
        ${WEBRTC_LIBRARIES}
        ${JSON_LIBRARIES}
        ${SOUP_LIBRARIES}
        pthread z
        )
