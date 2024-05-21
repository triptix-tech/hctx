cmake_minimum_required(VERSION 3.20)
project(hctx)

# --- OPTIONS ---
option(CTX_VALGRIND "CTX_VALGRIND" OFF)
option(CTX_ASAN "CTX_ASAN" OFF)

# --- DEPENDENCIES ---
include(cmake/pkg.cmake)

# --- LINT ---
option(NIGIRI_LINT "Run clang-tidy with the compiler." OFF)
if (NIGIRI_LINT)
    # clang-tidy will be run on all targets defined hereafter
    include(cmake/clang-tidy.cmake)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(hctx-compile-options
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-newline-eof
            -Wno-missing-prototypes
            -Wno-padded
            -Wno-double-promotion
            -Wno-undef
            -Wno-undefined-reinterpret-cast
            -Wno-float-conversion
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-switch-enum
            -Wno-c99-designator
            -Wno-zero-as-null-pointer-constant
            -Wno-missing-noreturn
            -Wno-undefined-func-template
            -Wno-unsafe-buffer-usage
            -Wno-c++20-compat
            -Wno-reserved-macro-identifier
            -Wno-documentation-unknown-command
            -Wno-duplicate-enum
            -Wno-reserved-identifier
            -Wno-ctad-maybe-unsupported
            -Werror)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(hctx-compile-options -Wall -Wextra -Werror)
elseif (MSVC)
    set(hctx-compile-options /WX)
else ()
    set(hctx-compile-options
            -Wall
            -Wextra
            -Wno-maybe-uninitialized)
    if (NOT CMAKE_CROSSCOMPILING)
        set(hctx-compile-options ${hctx-compile-options} -Werror)
    endif ()
endif ()


# --- LIB ---
add_library(hctx src/op.cc src/cv.cc src/fn.cc)
target_include_directories(hctx PUBLIC include)
target_compile_features(hctx PUBLIC cxx_std_23)
target_link_libraries(hctx PUBLIC boost_context concurrentqueue cista)
target_compile_options(hctx PRIVATE ${hctx-compile-options})
if (CTX_ASAN)
    target_compile_definitions(hctx PUBLIC CTX_ENABLE_ASAN)
endif ()
if (CTX_VALGRIND)
    target_compile_definitions(hctx PUBLIC CTX_ENABLE_VALGRIND)
endif ()


# --- TEST ---
file(GLOB_RECURSE hctx-test-files test/*.cc)
add_executable(hctx-test ${hctx-test-files})
target_link_libraries(hctx-test hctx gtest gtest_main)
target_compile_options(hctx-test PRIVATE ${hctx-compile-options})
