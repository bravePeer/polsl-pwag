#include "app/app.hpp"
#include "app/app_config.hpp"

int main()
{
    try
    {
        App app(kWidth, kHeight, "Vulkan App");
        app.init();
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}