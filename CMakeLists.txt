cmake_minimum_required(VERSION 3.16)
project(InputMapEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(Qt6_DIR "F:\\Qt\\6.11.2\\msvc2022_64\\lib\\cmake\\Qt6")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(IMAP_SOURCES
    src/InputModel.h        src/InputModel.cpp
    src/KeyCatalog.h        src/KeyCatalog.cpp
    src/ImapFormat.h        src/ImapFormat.cpp
    src/Validator.h         src/Validator.cpp
    src/InputDocument.h     src/InputDocument.cpp
    src/KesStyle.h          src/KesStyle.cpp
    src/Section.h
    src/KeyPicker.h         src/KeyPicker.cpp
    src/BindingCard.h       src/BindingCard.cpp
    src/ActionListPanel.h   src/ActionListPanel.cpp
    src/ActionDetailsPanel.h src/ActionDetailsPanel.cpp
    src/PreviewPanel.h      src/PreviewPanel.cpp
    src/ProblemsPanel.h     src/ProblemsPanel.cpp
    src/MainWindow.h        src/MainWindow.cpp
)

add_library(imapcore STATIC ${IMAP_SOURCES})
target_include_directories(imapcore PUBLIC src)
target_link_libraries(imapcore PUBLIC Qt6::Widgets)
if(MSVC)
    target_compile_options(imapcore PUBLIC /utf-8 /W3)
endif()

add_executable(InputMapEditor WIN32 MACOSX_BUNDLE src/main.cpp)
target_link_libraries(InputMapEditor PRIVATE imapcore)

if(WIN32)
target_sources(InputMapEditor PRIVATE resource/inputMappingEditor.rc)
    
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt6
            HINTS "${Qt6_DIR}/../../../bin"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET InputMapEditor POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:InputMapEditor>"
                COMMENT "Deploying Qt dependencies with windeployqt6"
        )
    else()
        message(WARNING "windeployqt6 not found; Qt deployment skipped")
    endif()
endif()