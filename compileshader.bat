C:\VulkanSDK\1.3.243.0\Bin\glslc.exe shaders\simple_shader.vert -o shaders\simple_shader.vert.spv
C:\VulkanSDK\1.3.243.0\Bin\glslc.exe shaders\simple_shader.frag -o shaders\simple_shader.frag.spv

C:\VulkanSDK\1.3.243.0\Bin\glslc.exe shaders\text.vert -o shaders\text.vert.spv
C:\VulkanSDK\1.3.243.0\Bin\glslc.exe shaders\text.frag -o shaders\text.frag.spv

::C:\VulkanSDK\1.3.211.0\Bin\glslangvalidator --target-env vulkan1.2 -x -e main -o shaders\simple_shader.frag.spv shaders\simple_shader.frag
::C:\VulkanSDK\1.3.211.0\Bin\glslangvalidator --target-env vulkan1.2 -x -e main -o shaders\text.vert.spv shaders\text.frag.spv

copy shaders\simple_shader.vert.spv build\shaders
copy shaders\simple_shader.frag.spv build\shaders
copy shaders\text.vert.spv build\shaders
copy shaders\text.vert.spv build\shaders

pause