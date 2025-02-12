#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class HelloTriangleApplication
{
public:
    void run()
	{
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    void InitVulkan()
	{

    }

    void MainLoop()
	{

    }

    void Cleanup()
	{

    }
};

int main(int argc, char* argv[])
{
    HelloTriangleApplication app;

    try 
    {
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

 