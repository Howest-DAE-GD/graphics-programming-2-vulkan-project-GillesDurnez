echo "---| Compiling shaders |---"
%VULKAN_SDK%\Bin\glslc.exe shader.vert -o vert.spv
%VULKAN_SDK%\Bin\glslc.exe shader.frag -o frag.spv
echo "---| Finished compiling shaders |---"