#include <iostream>

#include "Renderer/Window.h"
#include "Renderer/vkRenderer.h"

class HelloTriangleApplication
{
public:
    void Run()
	{
        while (!m_Renderer.GetWindow().ShouldClose())
        {
            glfwPollEvents();
            m_Renderer.RenderFrame();
        }
    }

private:
    gp2::VkRenderer m_Renderer{};
};

int main(int argc, char* argv[])
{
    HelloTriangleApplication app;

    try 
    {
        app.Run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

 