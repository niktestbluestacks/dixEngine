cmake_minimum_required(VERSION 3.20)
project(VulcanEngine CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Найти зависимости
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)  # ← Добавить эту строку

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