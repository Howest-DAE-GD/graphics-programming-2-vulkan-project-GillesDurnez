#include <chrono>
#include <iostream>
#include <thread>

#include "Time.h"

#include "Renderer/Window.h"
#include "Renderer/vkRenderer.h"


class HelloTriangleApplication
{
public:
    void Run()
	{
		gp2::Time::FixedTimeStep = m_FixedTimeStep;
        gp2::Time::Last_time = std::chrono::high_resolution_clock::now();

        while (!m_Renderer.GetWindow().ShouldClose())
        {
            const auto current_time = std::chrono::high_resolution_clock::now();
            const float delta_time = std::chrono::duration<float>(current_time - gp2::Time::Last_time).count();
            gp2::Time::Last_time = current_time;
            gp2::Time::Lag += delta_time;
            gp2::Time::DeltaTime = delta_time;

            while (gp2::Time::Lag >= gp2::Time::FixedTimeStep)
            {
                //FixedUpdate();
                gp2::Time::Lag -= gp2::Time::FixedTimeStep;
            }

            glfwPollEvents();
            m_Renderer.Update();
            m_Renderer.RenderFrame();

            const auto sleep_time = current_time + std::chrono::milliseconds(m_MsPerFrame) - std::chrono::high_resolution_clock::now();
            std::this_thread::sleep_for(sleep_time);
        }
    }

private:
    gp2::VkRenderer m_Renderer{};

    float m_FixedTimeStep{ 0.02f };
    int m_MsPerFrame{ 16 };
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

 