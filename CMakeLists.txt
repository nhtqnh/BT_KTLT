cmake_minimum_required(VERSION 3.16)

project(CarGame VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Thêm Multimedia vào đây để sau này mắm bật nhạc game không bị lỗi
find_package(Qt6 REQUIRED COMPONENTS Quick Multimedia)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appCarGame
    main.cpp
)

qt_add_qml_module(appCarGame
    URI CarGame
    QML_FILES
        Main.qml
        MenuView.qml
        GameView.qml
    SOURCES
        Logic/gameengine.h
        Logic/gameengine.cpp
        Model/car.h Model/car.cpp
        Model/obstacle.h Model/obstacle.cpp
        Model/entity.h Model/entity.cpp
    RESOURCES
        assets/backgr.png
        assets/main.png
        assets/car.png
        assets/car_1.png
        assets/button.png
        assets/car1.png
        assets/Car2.png
        assets/Car3.png
        assets/Car4.png
        assets/Obstacle.png
        assets/exit.png
        assets/replay.png
        assets/over.png
        assets/coin.png
        assets/barrie.png
        assets/pause.png
        assets/resume.png
        assets/resume_1.png
        sounds/crash.wav
        sounds/game.wav
        sounds/start.wav
        sounds/coin.mp3
        sounds/up.mp3
        sounds/down.mp3
        sounds/game2.mp3
)

set_target_properties(appCarGame PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCarGame
    PRIVATE Qt6::Quick Qt6::Multimedia
)

install(TARGETS appCarGame
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)