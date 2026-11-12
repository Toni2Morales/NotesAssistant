cmake_minimum_required(VERSION 3.20)

project(NotesAssistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Carpeta de sherpa-onnx
set(SHERPA_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/external/sherpa-onnx"
)

add_executable(NotesAssistant
    src/main.cpp
    src/audio_capture.cpp
    src/audio_capture.h
    src/miniaudio_impl.cpp
)

# Cabeceras de sherpa-onnx
target_include_directories(NotesAssistant PRIVATE
    "${SHERPA_ROOT}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio"
)

# Biblioteca C++ de sherpa-onnx
target_link_libraries(NotesAssistant PRIVATE
    "${SHERPA_ROOT}/lib/sherpa-onnx-cxx-api.lib"
)

# Copiar las DLL necesarias junto al ejecutable
file(GLOB SHERPA_DLLS
    "${SHERPA_ROOT}/lib/*.dll"
    "${SHERPA_ROOT}/bin/*.dll"
)

foreach(DLL_FILE IN LISTS SHERPA_DLLS)
    add_custom_command(TARGET NotesAssistant POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        "$<TARGET_FILE_DIR:NotesAssistant>"
    )
endforeach()
