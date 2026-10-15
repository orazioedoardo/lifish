# SFML 2.6.2 for the web (Emscripten), with an SDL2 + gl4es backend.
#
# SFML has no Emscripten support, so this downloads its source, applies
# sfml-2.6.2-emscripten.patch (platform selection, threadless music streaming, no busy-wait
# sleeps, no sRGB switching, which WebGL lacks) and builds the System, Window, Graphics and Audio modules as static libraries
# together with the backend in backend/. SFML's fixed-function OpenGL renders through gl4es
# to WebGL; audio uses Emscripten's OpenAL and the ogg/vorbis/freetype ports.
#
# Defines the targets sfml-system, sfml-window, sfml-graphics and sfml-audio.

include(FetchContent)
include(ExternalProject)

set(SFML_WEB_DIR "${CMAKE_CURRENT_LIST_DIR}")

# The patch's hash is part of the name, so editing the patch extracts a fresh copy
# (FetchContent doesn't re-apply a patch to an existing source tree)
file(SHA1 "${SFML_WEB_DIR}/sfml-2.6.2-emscripten.patch" SFML_PATCH_HASH)
string(SUBSTRING "${SFML_PATCH_HASH}" 0 8 SFML_PATCH_HASH)
set(SFML_FETCH_NAME "sfml_src_${SFML_PATCH_HASH}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SFML_WEB_DIR}/sfml-2.6.2-emscripten.patch")

FetchContent_Declare(${SFML_FETCH_NAME}
	URL https://github.com/SFML/SFML/archive/refs/tags/2.6.2.zip
	URL_HASH SHA256=19d6dbd9c901c74441d9888c13cb1399f614fe8993d59062a72cfbceb00fed04
	PATCH_COMMAND patch -p1 -i "${SFML_WEB_DIR}/sfml-2.6.2-emscripten.patch"
	SOURCE_SUBDIR do-not-add-subdirectory
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(${SFML_FETCH_NAME})
set(SFML_SRC "${${SFML_FETCH_NAME}_SOURCE_DIR}")

# gl4es translates fixed-function OpenGL to GLES2/WebGL
set(GL4ES_DIR "${CMAKE_SOURCE_DIR}/extern/gl4es")
set(GL4ES_LIBRARY "${GL4ES_DIR}/lib/libGL.a")	# gl4es always builds into its own source tree
set(GL4ES_PATCH "${CMAKE_SOURCE_DIR}/web/gl4es-no-texture-0.patch")
ExternalProject_Add(gl4es
	SOURCE_DIR "${GL4ES_DIR}"
	# SFML binds texture 0 to draw untextured shapes, and gl4es then sets up its filtering and mipmaps as if it were
	# OpenGL's default texture; in WebGL it's no texture at all, and each of those calls logs a warning. The check
	# makes re-running this step harmless once the patch is in.
	PATCH_COMMAND sh -c "git apply --reverse --check '${GL4ES_PATCH}' 2>/dev/null || git apply '${GL4ES_PATCH}'"
	CMAKE_ARGS
		-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
		-DCMAKE_BUILD_TYPE=Release
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5
		-DNOX11=ON
		-DNOEGL=ON
		-DSTATICLIB=ON
	INSTALL_COMMAND ""
	BUILD_BYPRODUCTS "${GL4ES_LIBRARY}")
add_library(gl4es_lib STATIC IMPORTED GLOBAL)
set_target_properties(gl4es_lib PROPERTIES IMPORTED_LOCATION "${GL4ES_LIBRARY}")
add_dependencies(gl4es_lib gl4es)

set(SFML_BACKEND "${SFML_WEB_DIR}/backend/SFML")
set(S "${SFML_SRC}/src/SFML")

function(sfml_web_module name)
	add_library(${name} STATIC ${ARGN})
	target_include_directories(${name}
		PUBLIC "${SFML_SRC}/include"
		PRIVATE "${SFML_SRC}/src" "${SFML_WEB_DIR}/backend"
		        "${SFML_SRC}/extlibs/headers/glad/include"
		        "${SFML_SRC}/extlibs/headers/stb_image"
		        "${SFML_SRC}/extlibs/headers/minimp3")
	target_compile_definitions(${name} PUBLIC SFML_STATIC)
	# Recent libc++ lacks the std::char_traits that sf::String needs
	target_compile_options(${name} PUBLIC
		"$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include ${SFML_WEB_DIR}/backend/char_traits_compat.hpp>")
	set_target_properties(${name} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
endfunction()

sfml_web_module(sfml-system
	${S}/System/Clock.cpp ${S}/System/Err.cpp ${S}/System/FileInputStream.cpp ${S}/System/Lock.cpp
	${S}/System/MemoryInputStream.cpp ${S}/System/Mutex.cpp ${S}/System/Sleep.cpp ${S}/System/String.cpp
	${S}/System/Thread.cpp ${S}/System/ThreadLocal.cpp ${S}/System/Time.cpp
	${S}/System/Unix/ClockImpl.cpp ${S}/System/Unix/MutexImpl.cpp ${S}/System/Unix/SleepImpl.cpp
	${S}/System/Unix/ThreadImpl.cpp ${S}/System/Unix/ThreadLocalImpl.cpp
	${SFML_BACKEND}/System/Emscripten/FrameHook.cpp)

sfml_web_module(sfml-window
	${S}/Window/Clipboard.cpp ${S}/Window/Context.cpp ${S}/Window/Cursor.cpp ${S}/Window/GlContext.cpp
	${S}/Window/GlResource.cpp ${S}/Window/Joystick.cpp ${S}/Window/JoystickManager.cpp
	${S}/Window/Keyboard.cpp ${S}/Window/Mouse.cpp ${S}/Window/Sensor.cpp ${S}/Window/SensorManager.cpp
	${S}/Window/Touch.cpp ${S}/Window/VideoMode.cpp ${S}/Window/Vulkan.cpp ${S}/Window/Window.cpp
	${S}/Window/WindowBase.cpp ${S}/Window/WindowImpl.cpp
	${SFML_BACKEND}/Window/Emscripten/SDLState.cpp
	${SFML_BACKEND}/Window/Emscripten/EmscriptenContext.cpp
	${SFML_BACKEND}/Window/Emscripten/WindowImplEmscripten.cpp
	${SFML_BACKEND}/Window/Emscripten/InputImpl.cpp
	${SFML_BACKEND}/Window/Emscripten/JoystickImpl.cpp
	${SFML_BACKEND}/Window/Emscripten/VideoModeImpl.cpp)
target_compile_options(sfml-window PUBLIC "-sUSE_SDL=2")
target_link_options(sfml-window PUBLIC "-sUSE_SDL=2")
# Only the backend needs gl4esinit.h; keep gl4es's GL/gl.h off SFML's own include path
set_source_files_properties(
	${SFML_BACKEND}/Window/Emscripten/SDLState.cpp
	${SFML_BACKEND}/Window/Emscripten/EmscriptenContext.cpp
	PROPERTIES COMPILE_OPTIONS "-I${GL4ES_DIR}/include")
add_dependencies(sfml-window gl4es)
target_link_libraries(sfml-window PUBLIC sfml-system gl4es_lib)

file(GLOB SFML_GRAPHICS_SRC "${S}/Graphics/*.cpp")
sfml_web_module(sfml-graphics ${SFML_GRAPHICS_SRC})
target_compile_options(sfml-graphics PUBLIC "-sUSE_FREETYPE=1")
target_link_options(sfml-graphics PUBLIC "-sUSE_FREETYPE=1")
target_link_libraries(sfml-graphics PUBLIC sfml-window)

sfml_web_module(sfml-audio
	${S}/Audio/ALCheck.cpp ${S}/Audio/AlResource.cpp ${S}/Audio/AudioDevice.cpp ${S}/Audio/InputSoundFile.cpp
	${S}/Audio/Listener.cpp ${S}/Audio/Music.cpp ${S}/Audio/OutputSoundFile.cpp ${S}/Audio/Sound.cpp
	${S}/Audio/SoundBuffer.cpp ${S}/Audio/SoundBufferRecorder.cpp ${S}/Audio/SoundFileFactory.cpp
	${S}/Audio/SoundFileReaderMp3.cpp ${S}/Audio/SoundFileReaderOgg.cpp ${S}/Audio/SoundFileReaderWav.cpp
	${S}/Audio/SoundFileWriterOgg.cpp ${S}/Audio/SoundFileWriterWav.cpp ${S}/Audio/SoundRecorder.cpp
	${S}/Audio/SoundSource.cpp ${S}/Audio/SoundStream.cpp)
target_compile_options(sfml-audio PUBLIC "-sUSE_OGG=1" "-sUSE_VORBIS=1")
# SFML includes <al.h>/<alc.h> without the AL/ prefix
target_include_directories(sfml-audio PRIVATE "${EMSCRIPTEN_SYSROOT}/include/AL")
target_link_options(sfml-audio PUBLIC "-sUSE_OGG=1" "-sUSE_VORBIS=1" "-lopenal")
target_link_libraries(sfml-audio PUBLIC sfml-system)
