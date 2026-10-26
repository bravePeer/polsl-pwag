#include "app/app.hpp"

constexpr uint32_t kWidth = 800;
constexpr uint32_t kHeight = 600;

int main()
{
    try
    {
        App app(kWidth, kHeight, "Vulkan App");
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}