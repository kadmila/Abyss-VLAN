add_library(platform)

target_include_directories(platform
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(WIN32)

    target_link_libraries(platform PRIVATE ws2_32 iphlpapi)

    target_sources(platform PRIVATE
        windows/platform.c
    )

elseif(UNIX)

    target_sources(platform PRIVATE
        linux/platform.c
    )

endif()