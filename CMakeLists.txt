cmake_minimum_required(VERSION 3.20)
project(VulcanEngine CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Найти зависимости

# ==========================================
# 1. FIND DEPENDENCIES
# ==========================================
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)  # ← Добавить эту строку

# Find glslc
if(Vulkan_GLSLC_EXECUTABLE)
    set(GLSLC ${Vulkan_GLSLC_EXECUTABLE})
else()
    set(GLSLC "C:/VulkanSDK/1.4.341.1/Bin/glslc.exe")
endif()



# ==========================================
# 2. SHADER COMPILATION
# ==========================================
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/VulcanEngine/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

file(GLOB_RECURSE SHADER_SOURCES 
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
)

# Debug: Fixed counter (CMAKE_MATCH_COUNT doesn't work with GLOB)
list(LENGTH SHADER_SOURCES SHADER_COUNT)
message(STATUS "Found ${SHADER_COUNT} shader files")

set(SHADER_BINARIES "")
foreach(SHADER ${SHADER_SOURCES})
    file(RELATIVE_PATH SHADER_REL_PATH ${SHADER_SOURCE_DIR} ${SHADER})
    set(SHADER_OUTPUT "${SHADER_BINARY_DIR}/${SHADER_REL_PATH}.spv")
    
    get_filename_component(SHADER_OUTPUT_DIR ${SHADER_OUTPUT} DIRECTORY)
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
    
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER_REL_PATH}"
    )
    list(APPEND SHADER_BINARIES ${SHADER_OUTPUT})
endforeach()

# Create shader target (ALL ensures it runs every build)
add_custom_target(Shaders ALL DEPENDS ${SHADER_BINARIES})

# Собрать все .cpp файлы
file(GLOB_RECURSE ENGINE_SOURCES 
    "${CMAKE_SOURCE_DIR}/VulcanEngine/*.cpp"
    "${CMAKE_SOURCE_DIR}/Applications/*.cpp"
)

# Создать исполняемый файл
add_executable(VulcanApp ${ENGINE_SOURCES})

set_target_properties(VulcanApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Пути к заголовкам
target_include_directories(VulcanApp PRIVATE
    ${CMAKE_SOURCE_DIR}/Applications
    ${CMAKE_SOURCE_DIR}/VulcanEngine
    ${CMAKE_SOURCE_DIR}/Dependecies
)

# Линковка
target_link_libraries(VulcanApp PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm  # ← Добавить эту строку (или просто glm)
)

# Copy compile_commands.json to project root for clangd
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)