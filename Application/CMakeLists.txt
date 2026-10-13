add_library(alors_app_core STATIC
    Core/DemoPatterns.h
    Core/DemoPatterns.cpp
    Core/PlaybackController.h
    Core/PlaybackController.cpp
    Core/Workspace.h
    Core/Workspace.cpp)
target_include_directories(alors_app_core PUBLIC Core)
target_link_libraries(alors_app_core PUBLIC alors_engine)

juce_add_gui_app(OnDanse
    PRODUCT_NAME "OnDanse"
    COMPANY_NAME "OnDanse")

target_sources(OnDanse PRIVATE
    Frontend/Main.cpp
    Frontend/MainComponent.h
    Frontend/MainComponent.cpp
    Frontend/EditorPanel.h
    Frontend/EditorPanel.cpp
    Frontend/FileBrowserPanel.h
    Frontend/FileBrowserPanel.cpp
    Frontend/LibraryPanel.h
    Frontend/LibraryPanel.cpp
    Frontend/Shortcuts.h
    Frontend/Theme.h
    Frontend/Theme.cpp)
target_include_directories(OnDanse PRIVATE Frontend)
target_link_libraries(OnDanse PRIVATE alors_app_core)
