cmake_minimum_required(VERSION 3.20)
project(aegis_prime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(AEGIS_ENABLE_ASAN "Build with AddressSanitizer + UBSan" OFF)
option(AEGIS_ENABLE_TSAN "Build with ThreadSanitizer" OFF)

if(AEGIS_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
elseif(AEGIS_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -g)
    add_link_options(-fsanitize=thread)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY FALSE
)
FetchContent_MakeAvailable(googletest)


enable_testing()
add_subdirectory(engine)