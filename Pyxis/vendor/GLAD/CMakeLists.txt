# GLAD library setup

# Add GLAD source files
set(GLAD_SOURCES
    src/glad.c
)

# Add GLAD header files
set(GLAD_HEADERS
    include/glad/glad.h
    include/KHR/khrplatform.h
)

# Create GLAD library
add_library(glad STATIC ${GLAD_SOURCES} ${GLAD_HEADERS})

# Set include directories for GLAD
target_include_directories(glad PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set compile definitions for GLAD
target_compile_definitions(glad PRIVATE GLAD_GLAPI_EXPORT)

# Set compile options for GLAD
if(MSVC)
    target_compile_options(glad PRIVATE /W4)
else()
    target_compile_options(glad PRIVATE -Wall -Wextra -Wpedantic -Wattributes)
endif()

# Install GLAD library and headers
install(TARGETS glad
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
