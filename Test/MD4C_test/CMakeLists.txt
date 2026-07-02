add_executable(MD4C_test MD4C_test.cpp ../../src/md4c/md4c.cpp)
target_include_directories(MD4C_test PRIVATE 
    ${md4c_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
target_link_libraries(MD4C_test PRIVATE Qt6::Core md4c)
