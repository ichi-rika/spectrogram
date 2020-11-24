cmake_minimum_required(VERSION 3.5)
set(CMAKE_OSX_DEPLOYMENT_TARGET $ENV{MACOSX_DEPLOYMENT_TARGET})
project(CYHSpec)

# Disallow in-source builds
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR
    "In-source builds are not permitted!  Make a separate folder for"
    " building, e.g.,\n"
    "  cmake -E make_directory build\n"
    "  cmake -E chdir build cmake ..\n"
    "Before that, remove the files created by this failed run with\n"
    "  cmake -E remove CMakeCache.txt\n"
    "  cmake -E remove_directory CMakeFiles")
endif ()

set(CMAKE_SUPPORT_DIR "${PROJECT_SOURCE_DIR}/cmake_support")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SUPPORT_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-fPIC -ftree-vectorize -fno-omit-frame-pointer -fno-math-errno")
set(CMAKE_CXX_FLAGS_DEBUG           "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE         "-O3 -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -g0 -DNDEBUG")

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
        src/main.cpp
        src/context/context.h
        src/context/contextbuilder.h
        src/context/contextmanager.cpp
        src/context/ctxmgr_spectrogram.cpp
        src/context/ctxmgr_fftspectrum.cpp
        src/context/ctxmgr_settings.cpp
        src/context/contextmanager.h
        src/backtrace/backtrace.h
        src/android_redirect_log.cpp
        src/modules/audio/base/base.cpp
        src/modules/audio/base/base.h
        src/modules/audio/buffer/buffer.cpp
        src/modules/audio/buffer/buffer.h
        src/modules/audio/queue/queue.cpp
        src/modules/audio/queue/queue.h
        src/modules/audio/resampler/resampler.cpp
        src/modules/audio/resampler/resampler.h
        src/modules/audio/audio.h
        src/modules/target/base/base.cpp
        src/modules/target/base/base.h
        src/modules/target/target.h
        src/modules/renderer/base/base.cpp
        src/modules/renderer/base/parameters.h
        src/modules/renderer/base/base.h
        src/modules/renderer/renderer.h
        src/modules/app/pipeline/pipeline.cpp
        src/modules/app/pipeline/pipeline.h
        src/modules/app/app.h
        src/modules/modules.h
        src/nodes/node/nodeio_audiotime.cpp
        src/nodes/node/nodeio_audiotime.h
        src/nodes/node/nodeio_audiospec.cpp
        src/nodes/node/nodeio_audiospec.h
        src/nodes/node/nodeio_frequencies.cpp
        src/nodes/node/nodeio_frequencies.h
        src/nodes/node/nodeio_iirfilter.cpp
        src/nodes/node/nodeio_iirfilter.h
        src/nodes/node/nodeio.cpp
        src/nodes/node/nodeio.h
        src/nodes/node/node.cpp
        src/nodes/node/node.h
        src/nodes/prereqs/prereqs.cpp
        src/nodes/prereqs/prereqs.h
        src/nodes/resampler/resampler.cpp
        src/nodes/resampler/resampler.h
        src/nodes/spectrum/spectrum.cpp
        src/nodes/spectrum/spectrum.h
        src/nodes/tail/tail.cpp
        src/nodes/tail/tail.h
        src/nodes/preemph/preemph.cpp
        src/nodes/preemph/preemph.h
        src/nodes/nodes.h
        src/analysis/filter/butterworth.cpp
        src/analysis/filter/zpk2sos.cpp
        src/analysis/filter/sosfilter.cpp
        src/analysis/filter/filter.cpp
        src/analysis/filter/filter.h
        src/analysis/fft/realfft.cpp
        src/analysis/fft/complexfft.cpp
        src/analysis/fft/realrealfft.cpp
        src/analysis/fft/wisdom.cpp
        src/analysis/fft/fft.h
        src/analysis/resampler/resampler.cpp
        src/analysis/resampler/resampler.h
        src/analysis/util/find_peaks.cpp
        src/analysis/util/util.h
        src/analysis/analysis.h
        external/r8brain-free-src/r8bbase.cpp
        external/r8brain-free-src/pffft.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED fftw3)

set(R8B_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/external/r8brain-free-src")
set(NANO_SVG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/external/nanosvg")

if(TRUE)
    message(STATUS "Including audio module: dummy")
    set(AUDIO_USE_DUMMY TRUE)
    list(APPEND SOURCES
        src/modules/audio/dummy/dummy.cpp
        src/modules/audio/dummy/dummy
    )
endif()

pkg_check_modules(alsa QUIET alsa)
if(alsa_FOUND)
    message(STATUS "Including audio module: alsa")
    set(AUDIO_USE_ALSA TRUE)
    list(APPEND SOURCES
        src/modules/audio/alsa/alsa.cpp
        src/modules/audio/alsa/alsa.h
    )
endif()

pkg_check_modules(pulse QUIET libpulse)
if(pulse_FOUND)
    message(STATUS "Including audio module: pulse")
    set(AUDIO_USE_PULSE TRUE)
    list(APPEND SOURCES
        src/modules/audio/pulse/pulse.cpp
        src/modules/audio/pulse/pulse.h
    )
endif()

pkg_check_modules(portaudio QUIET portaudio-2.0)
if(portaudio_FOUND)
    message(STATUS "Including audio module: portaudio")
    set(AUDIO_USE_PORTAUDIO TRUE)
    list(APPEND SOURCES
        src/modules/audio/portaudio/portaudio.cpp
        src/modules/audio/portaudio/portaudio.h
    )
endif()

if(ANDROID)
    message(STATUS "Including audio module: oboe")
    set(OBOE_DIR "${PROJECT_SOURCE_DIR}/external/oboe")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    add_subdirectory(${OBOE_DIR} ${PROJECT_BINARY_DIR}/oboe)
    set(AUDIO_USE_OBOE TRUE)
    list(APPEND SOURCES
        src/modules/audio/oboe/oboe.cpp
        src/modules/audio/oboe/oboe.h
    )
endif()

if(EMSCRIPTEN)
    message(STATUS "Including audio module: webaudio")
    set(AUDIO_USE_WEBAUDIO TRUE)
    list(APPEND SOURCES
        src/modules/audio/webaudio/webaudio.cpp
        src/modules/audio/webaudio/webaudio.h)
endif()

pkg_check_modules(sdl2 QUIET sdl2)
if(sdl2_FOUND)
    message(STATUS "Including target module: sdl2")
    set(TARGET_USE_SDL2 TRUE)
    list(APPEND SOURCES
        src/modules/target/sdl2/sdl2.cpp
        src/modules/target/sdl2/sdl2.h
    )
endif()

if(TRUE)
    message(STATUS "Including renderer module: nanovg")
    set(NVG_DIR "${PROJECT_SOURCE_DIR}/external/nanovg")
    add_subdirectory(${NVG_DIR} ${PROJECT_BINARY_DIR}/nanovg)
    set(RENDERER_USE_NVG TRUE)
    list(APPEND SOURCES
        src/modules/renderer/nanovg/nanovg.cpp
        src/modules/renderer/nanovg/nanovg.h
        src/modules/renderer/nanovg/fontattachment.cpp
        src/modules/renderer/nanovg/nvg_gl.c
        src/modules/renderer/nanovg/nvg_gl.h)
    if(TARGET_USE_SDL2)
        list(APPEND SOURCES
            src/modules/target/sdl2/sdl2_nanovg.cpp
        )
    endif()
endif()

if(TRUE)
    message(STATUS "Including font module: freetype")
    set(FREETYPE_DIR "${PROJECT_SOURCE_DIR}/external/freetype")
    set(BUILD_SHARED_LIBS TRUE)
    add_subdirectory(${FREETYPE_DIR} ${PROJECT_BINARY_DIR}/freetype)
    set(FONT_USE_FREETYPE TRUE)
    list(APPEND SOURCES
        src/modules/freetype/instance.cpp
        src/modules/freetype/fontfile.cpp
        src/modules/freetype/font.cpp
        src/modules/freetype/freetype.h
    )
endif()

if(ANDROID)
    add_library(CYHSpec SHARED ${SOURCES})
else()
    add_executable(CYHSpec ${SOURCES})
endif()

target_include_directories(CYHSpec SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} ${R8B_INCLUDE_DIR} ${NANO_SVG_INCLUDE_DIR})
target_link_libraries(CYHSpec PUBLIC ${SDL2_LDFLAGS} ${FFTW_LDFLAGS})
target_compile_definitions(CYHSpec PUBLIC -DR8B_PFFFT=1)

if(AUDIO_USE_DUMMY)
    target_compile_definitions(CYHSpec PUBLIC -DAUDIO_USE_DUMMY=1)
endif()

if(AUDIO_USE_ALSA)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${alsa_INCLUDE_DIRS})
    target_link_libraries(CYHSpec PUBLIC ${alsa_LDFLAGS})
    target_compile_definitions(CYHSpec PUBLIC -DAUDIO_USE_ALSA=1)
endif()

if(AUDIO_USE_PULSE)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${pulse_INCLUDE_DIRS})
    target_link_libraries(CYHSpec PUBLIC ${pulse_LDFLAGS})
    target_compile_definitions(CYHSpec PUBLIC -DAUDIO_USE_PULSE=1)
endif()

if(AUDIO_USE_PORTAUDIO)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${portaudio_INCLUDE_DIRS})
    target_link_libraries(CYHSpec PUBLIC ${portaudio_LIBRARIES})
    target_compile_definitions(CYHSpec PUBLIC -DAUDIO_USE_PORTAUDIO=1)
endif()

if(AUDIO_USE_OBOE)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${OBOE_DIR}/include)
    target_link_libraries(CYHSpec PUBLIC oboe)
    target_compile_definitions(CYHSpec PUBLIC -DAUDIO_USE_OBOE=1)
endif()

if(AUDIO_USE_WEBAUDIO)
    target_compile_definitions(CYHSpec PUBLIC -DAUDIO_USE_WEBAUDIO=1)
endif()

if(TARGET_USE_SDL2)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${sdl2_INCLUDE_DIRS})
    target_link_libraries(CYHSpec PUBLIC ${sdl2_LDFLAGS})
    target_compile_definitions(CYHSpec PUBLIC -DTARGET_USE_SDL2=1)
endif()

if(RENDERER_USE_NVG)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${NVG_DIR}/src)
    target_link_libraries(CYHSpec PUBLIC nanovg)
    target_compile_definitions(CYHSpec PUBLIC -DRENDERER_USE_NVG=1)

    if(ANDROID)
        find_package(OpenGLES3 QUIET)
        target_include_directories(CYHSpec SYSTEM PUBLIC ${OpenGLES3_INCLUDE_DIRS})
        target_link_libraries(CYHSpec PUBLIC ${OpenGLES3_LIBRARIES})
    elseif(EMSCRIPTEN)
        #find_package(OpenGLES2 QUIET)
        #target_include_directories(CYHSpec SYSTEM PUBLIC ${OpenGLES2_INCLUDE_DIRS})
        #target_link_libraries(CYHSpec PUBLIC ${OpenGLES2_LIBRARIES})
    elseif(UNIX OR WIN32 OR APPLE)
        set(OpenGL_GL_PREFERENCE GLVND)
        find_package(OpenGL QUIET)
        target_include_directories(CYHSpec SYSTEM PUBLIC ${OPENGL_INCLUDE_DIRS})
        target_link_libraries(CYHSpec PUBLIC ${OPENGL_LIBRARIES})

        target_sources(CYHSpec PUBLIC "${PROJECT_SOURCE_DIR}/external/glew-2.1.0/src/glew.c")
        target_compile_definitions(CYHSpec PUBLIC -DGLEW_STATIC -DGLEW_NO_GLU)
    endif()
endif()

if(FONT_USE_FREETYPE)
    target_include_directories(CYHSpec SYSTEM PUBLIC ${FREETYPE_DIR}/include)
    target_link_libraries(CYHSpec PUBLIC freetype)
    target_compile_definitions(CYHSpec PUBLIC -DFONT_USE_FREETYPE=1)
endif()

if(EMSCRIPTEN)
    set(EM_FLAGS "-sENVIRONMENT=web" "-sASSERTIONS=0" "-sINITIAL_MEMORY=32MB" "-sALLOW_MEMORY_GROWTH=1" "--no-heap-copy" "-sEMULATE_FUNCTION_POINTER_CASTS=1" "-sERROR_ON_UNDEFINED_SYMBOLS=1" "-sUSE_PTHREADS=0" "-sEXIT_RUNTIME=1")

    set(EMCC_FLAGS ${EM_FLAGS})
    set(EMLD_FLAGS ${EM_FLAGS} "-sEXPORTED_FUNCTIONS=[_main,_webaudio_process_playback,_webaudio_process_capture]" "-sEXPORTED_RUNTIME_METHODS=[ccall]" "-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[$autoResumeAudioContext]" "--preload-file ${PROJECT_SOURCE_DIR}/Montserrat.otf@Montserrat.otf")

    if (CMAKE_BUILD_TYPE MATCHES "^Debug$")
        target_compile_options(CYHSpec PUBLIC ${EMCC_FLAGS} -g -g4 --source-map-base /)
        target_link_libraries(CYHSpec PUBLIC ${EMLD_FLAGS} -g -g4 "--source-map-base /")
    else()
        target_compile_options(CYHSpec PUBLIC ${EMCC_FLAGS})
        target_link_libraries(CYHSpec PUBLIC ${EMLD_FLAGS})
    endif()
endif()

if(ANDROID)
    target_sources(CYHSpec public src/context/ctxmgr_androidcommon.cpp)
    if(target_use_sdl2)
        target_sources(CYHSpec public src/modules/target/sdl2/sdl2_android.cpp)
    endif()
    target_link_libraries(CYHSpec public android)
endif()

if(UNIX AND NOT EMSCRIPTEN)
    target_sources(CYHSpec PUBLIC src/backtrace/unix.cpp)
    target_link_libraries(CYHSpec PUBLIC dl)
    set_target_properties(CYHSpec PROPERTIES LINK_FLAGS "-pthread -static-libstdc++")
endif()

if(NOT ANDROID AND NOT WIN32 AND CMAKE_BUILD_TYPE MATCHES "^Debug$")
    target_compile_options(CYHSpec PUBLIC "-fsanitize=address -fsanitize=undefined")
    target_link_options(CYHSpec PUBLIC "-fsanitize=address -fsanitize=undefined")
endif()

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_sources(CYHSpec PUBLIC src/backtrace/win32.cpp)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_sources(CYHSpec PUBLIC src/backtrace/win64.cpp)
    endif()
    target_link_libraries(CYHSpec PUBLIC ntdll dbghelp)
endif()

