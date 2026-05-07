@echo off
setlocal

cd /d "%~dp0"

echo Compiling shaders...

C:\VulkanSDK\1.4.341.1\Bin\glslc.exe SimpleShader\simple_shader.vert -o SimpleShader\simple_shader.vert.spv
if %errorlevel% neq 0 (
    echo Vertex shader compilation failed!
    exit /b %errorlevel%
)

C:\VulkanSDK\1.4.341.1\Bin\glslc.exe SimpleShader\simple_shader.frag -o SimpleShader\simple_shader.frag.spv
if %errorlevel% neq 0 (
    echo Fragment shader compilation failed!
    exit /b %errorlevel%
)

C:\VulkanSDK\1.4.341.1\Bin\glslc.exe UI\ui.vert -o UI\ui.vert.spv
if %errorlevel% neq 0 (
    echo Vertex shader compilation failed!
    exit /b %errorlevel%
)

C:\VulkanSDK\1.4.341.1\Bin\glslc.exe UI\ui.frag -o UI\ui.frag.spv
if %errorlevel% neq 0 (
    echo Fragment shader compilation failed!
    exit /b %errorlevel%
)

echo Shaders compiled successfully!
endlocal
pause