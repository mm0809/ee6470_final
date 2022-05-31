file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-basic-final
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-basic-final PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-final
        main.cpp)

target_link_libraries(riscv-vp-final rv32 platform-basic-final platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
