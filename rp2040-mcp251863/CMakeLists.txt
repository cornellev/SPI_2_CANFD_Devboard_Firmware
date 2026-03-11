add_library(rp2040_mcp251863
    mcp251863.cpp
)

target_link_libraries(rp2040_mcp251863
        hardware_spi
        pico_stdlib
        )

target_include_directories (rp2040_mcp251863 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})