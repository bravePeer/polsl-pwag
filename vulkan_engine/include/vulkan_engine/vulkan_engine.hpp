#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <map>
#include <fstream>
#include <chrono>
#include <functional>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "input/input.hpp"
#include "vertex/vertex_data.hpp"

#include "vulkan_engine/vulkan_config.hpp"
#include "vulkan_resource_manager.hpp"
#include "vulkan_engine/vulkan_shader_loader.hpp"

struct WindowProvider
{
    std::vector<const char*> instanceExtensions;
    std::function<std::pair<int,int>()> getFramebufferSize;
    std::function<VkSurfaceKHR(VkInstance)> createSurface;
};

class VulkanEngine
{
public:
    VulkanEngine(WindowProvider& windowProvider)
    : windowProvider(windowProvider)
    {
        if (!windowProvider.getFramebufferSize || !windowProvider.createSurface)
        {
            throw std::invalid_argument("WindowProvider: missing callback");
        }

        createInstance();
        /// @todo setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        vkResourceManager.emplace(device, physicalDevice, queue, commandPool);
        // createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();
        
        // createVertexBuffer();
        // createIndexBuffer();
        // createUniformBuffers();

        // createDescriptorPool();
        // createDescriptorSets();

        createCommandBuffer();
        createSyncObjects();
    }
    ~VulkanEngine()
    {
        device.waitIdle();

        cleanupSwapChain();
    }

    VulkanEngine(const VulkanEngine&) = delete;
    VulkanEngine& operator=(const VulkanEngine&) = delete;

    /// @brief Temporary method during refactor
    bool drawFramePub()
    {
        return drawFrame();
    }

    void wait()
    {
        device.waitIdle();
    }

    void setWindowResized(bool resized)
    {
        framebufferResized = resized;
    }

    void recreateSwapChainPub()
    {
        device.waitIdle();
        cleanupSwapChain();
        createSwapChain();
        createImageViews();
    }

    VulkanResourceManager& getResourceManager()
    {
        return *vkResourceManager;
    }

private:
    WindowProvider& windowProvider;
    // GLFWwindow* window = nullptr;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;

    vk::raii::PhysicalDevice physicalDevice = nullptr;
    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    vk::raii::Queue graphicsQueue = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue queue = nullptr;
    uint32_t queueIndex;  

    vk::raii::Semaphore presentCompleteSemaphore = nullptr;
    // vk::raii::Semaphore renderFinishedSemaphore  = nullptr;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    vk::raii::Fence     drawFence                = nullptr;

    vk::raii::SurfaceKHR surface = nullptr;

    /// Chain
    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR   swapChainSurfaceFormat;
    vk::Extent2D           swapChainExtent;
    
    // views
    std::vector<vk::raii::ImageView> swapChainImageViews;

    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;
    
    vk::raii::CommandPool commandPool = nullptr;
    vk::raii::CommandBuffer commandBuffer = nullptr;

    bool framebufferResized = false;

    std::optional<VulkanResourceManager> vkResourceManager;

    void createSyncObjects()
    {
        presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
        drawFence                = vk::raii::Fence(device, {.flags = vk::FenceCreateFlagBits::eSignaled});
        // renderFinishedSemaphore  = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        }
    }

    /// @brief 
    /// @return should recreate swap chain 
    bool drawFrame()
    {
        auto fenceResult = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}

        auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);

        if (result == vk::Result::eErrorOutOfDateKHR || framebufferResized)
        {
            framebufferResized = false;
            // recreateSwapChain();
            return true;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
            throw std::runtime_error("failed to acquire swap chain image!");
        }

		device.resetFences(*drawFence);
        updateUniformBuffer(0);
        
        recordCommandBuffer(imageIndex);

        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
        const vk::SubmitInfo   submitInfo{
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &*presentCompleteSemaphore,
            .pWaitDstStageMask    = &waitDestinationStageMask,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &*commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]
        };


        
        queue.submit(submitInfo, *drawFence);
             
             
        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &*swapChain,
            .pImageIndices      = &imageIndex
        };
        result = queue.presentKHR(presentInfoKHR);

        if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR))
        {
            // recreateSwapChain();
            return true;
        }
        else
        {
            // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
            assert(result == vk::Result::eSuccess);
        }
        return false;
    }
    void cleanupSwapChain()
    {
        swapChainImageViews.clear();
        swapChain = nullptr;
    }

    void createInstance()
    {
        dumpAvailableExtensions();
        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName   = "Hello Triangle",
            .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .pEngineName        = "No Engine",
            .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
            .apiVersion         = vk::ApiVersion14
        };

        // Get the required instance extensions from GLFW.
        // Check if the required GLFW extensions are supported by the Vulkan implementation.
        auto extensionProperties = context.enumerateInstanceExtensionProperties();

        for(auto extension : windowProvider.instanceExtensions)
        {
            if (std::ranges::none_of(
                    extensionProperties,
                    [extension](auto const& extensionProperty)
                    { return strcmp(extensionProperty.extensionName, extension) == 0; 
                }))
            {
                throw std::runtime_error("Required instance extension not supported: " + std::string(extension));
            }
        }
        


        /// Validation layers
        std::vector<const char*> requiredLayers;
        if(enableValidationLayers)
        {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }

        auto layerProperties = context.enumerateInstanceLayerProperties();
        auto unsupportedLayerIt = std::ranges::find_if(
            requiredLayers,
		    [&layerProperties](auto const &requiredLayer) {
			    return std::ranges::none_of(
                    layerProperties,
                    [requiredLayer](auto const &layerProperty) { 
                        return strcmp(layerProperty.layerName, requiredLayer) == 0; 
                    });
		    });

        if(unsupportedLayerIt != requiredLayers.end())
        {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
        }

        /// Extensions
        auto requiredExtensions = windowProvider.instanceExtensions;

        // auto extensionProperties = context.enumerateInstanceExtensionProperties();
        auto unsupportedPropertyIt = std::ranges::find_if(
            requiredExtensions, 
            [&extensionProperties](auto const &requiredExtension){
                return std::ranges::none_of(
                    extensionProperties, 
                    [requiredExtension](auto const &extensionProperty){
                        return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                    });
            });

        if(unsupportedPropertyIt != requiredExtensions.end())
        {
            throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
        }

        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data()
        };

        instance = vk::raii::Instance(context, createInfo);
    }

    void dumpAvailableExtensions()
    {
        std::cout << "Available extensions: \n";
        auto extensions = context.enumerateInstanceExtensionProperties();
        for(const auto& extension : extensions)
        {
            std::cout << "\t " << extension.extensionName << "\n";
        }
        
    }
    
    bool isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice )
    {
        // Check if the physicalDevice supports the Vulkan 1.3 API version
        bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

        // Check if any of the queue families support graphics operations
        auto queueFamilies    = physicalDevice.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of( queueFamilies, []( auto const & qfp ) { return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics ); } );

        // Check if all required physicalDevice extensions are available
        auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions =
            std::ranges::all_of( requiredDeviceExtension,
                                [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
                                {
                                return std::ranges::any_of( availableDeviceExtensions,
                                                            [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                                            { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
                                } );

        // Check if the physicalDevice supports the required features (shader draw parameters, dynamic rendering and extended dynamic state)
        auto features                 = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                                            vk::PhysicalDeviceVulkan11Features,
                                                                            vk::PhysicalDeviceVulkan13Features,
                                                                            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                        features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                                        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
                                

        // Return true if the physicalDevice meets all the criteria
        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
    }

    void pickPhysicalDevice()
    {
        std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
        auto const devIter = std::ranges::find_if( physicalDevices, [&]( auto const & physicalDevice ) { return isDeviceSuitable( physicalDevice ); } );
        if ( devIter == physicalDevices.end() )
        {
            throw std::runtime_error( "failed to find a suitable GPU!" );
        }
        physicalDevice = *devIter;
    }
    
    void createLogicalDevice()
    {
        // find the index of the first queue family that supports graphics
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        // get the first index into queueFamilyProperties which supports both graphics and present
        queueIndex = ~0;
        for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
        {
            if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
            {
                // found a queue family that supports both graphics and present
                queueIndex = qfpIndex;
                break;
            }
        }

        if (queueIndex == ~0)
        {
            throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
        }

        // query for Vulkan 1.3 features
        vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> 
            featureChain = {
                {},                                    // vk::PhysicalDeviceFeatures2
                {.shaderDrawParameters = true},        // vk::PhysicalDeviceVulkan11Features
                {.synchronization2 = true, .dynamicRendering = true},            // vk::PhysicalDeviceVulkan13Features
                {.extendedDynamicState = true}         // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
            };

        // create a Device
        float                     queuePriority = 0.5f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
        vk::DeviceCreateInfo      deviceCreateInfo{.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                                .queueCreateInfoCount    = 1,
                                                .pQueueCreateInfos       = &deviceQueueCreateInfo,
                                                .enabledExtensionCount   = static_cast<uint32_t>(requiredDeviceExtension.size()),
                                                .ppEnabledExtensionNames = requiredDeviceExtension.data()};

        device = vk::raii::Device( physicalDevice, deviceCreateInfo );
        queue  = vk::raii::Queue(device, queueIndex, 0);
    }


    void createSurface()
    {
        VkSurfaceKHR tmpSurface = windowProvider.createSurface(*instance);
        surface = vk::raii::SurfaceKHR(instance, tmpSurface);
    }

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        const auto formatIt = std::ranges::find_if(
            availableFormats,
            [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
        return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
    }

    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
    {
        // currentExtent is only set to the special "undefined" value described above
        // when the window manager lets us choose the extent ourselves; any other value
        // means the surface already dictates a fixed extent that we must use as-is.
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        std::pair<int, int> framebufferSize = windowProvider.getFramebufferSize();

        // int width, height;
        // glfwGetFramebufferSize(window, &width, &height);

        return {
            std::clamp<uint32_t>(framebufferSize.first, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(framebufferSize.second, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
    {
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
        {
            minImageCount = surfaceCapabilities.maxImageCount;
        }
        return minImageCount;
    }

    void createSwapChain()
    {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR( *surface );
        swapChainExtent                                = chooseSwapExtent(surfaceCapabilities);
        uint32_t minImageCount                         = chooseSwapMinImageCount(surfaceCapabilities);

        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
        swapChainSurfaceFormat                             = chooseSwapSurfaceFormat(availableFormats);

        vk::SwapchainCreateInfoKHR info{
            .surface          = *surface,
            .minImageCount    = minImageCount,
            .imageFormat      = swapChainSurfaceFormat.format,
            .imageColorSpace  = swapChainSurfaceFormat.colorSpace,
            .imageExtent      = swapChainExtent,
            .imageArrayLayers = 1,
            .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform     = surfaceCapabilities.currentTransform,
            .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode      = vk::PresentModeKHR::eFifo,
            .clipped          = true
        };
        swapChain       = vk::raii::SwapchainKHR(device, info);
        swapChainImages = swapChain.getImages();
    }

    void createImageViews()
    {
        assert(swapChainImageViews.empty());

        vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType         = vk::ImageViewType::e2D,
                                                    .format           = swapChainSurfaceFormat.format,
                                                    .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

        for (auto &image : swapChainImages)
        {
            imageViewCreateInfo.image = image;
            swapChainImageViews.emplace_back( device, imageViewCreateInfo );
        }
    }

    void createGraphicsPipeline()
    {
        ShaderLoader shaders(device, "shaders/slang.spv");
        
        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };


        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
            .topology = vk::PrimitiveTopology::eTriangleList
        };
        vk::Viewport viewport{
            0.0f, 0.0f, 
            static_cast<float>(swapChainExtent.width), 
            static_cast<float>(swapChainExtent.height), 
            0.0f, 1.0f
        };

        vk::Rect2D scissor{vk::Offset2D{ 0, 0 }, swapChainExtent};

        vk::PipelineDynamicStateCreateInfo dynamicState{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };
        vk::PipelineViewportStateCreateInfo viewportState{
            .viewportCount = 1, 
            .pViewports = &viewport, 
            .scissorCount = 1, 
            .pScissors = &scissor
        };


        vk::PipelineRasterizationStateCreateInfo rasterizer{
            .depthClampEnable        = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode             = vk::PolygonMode::eFill,
            .cullMode                = vk::CullModeFlagBits::eBack,
            .frontFace               = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable         = vk::False,
            .lineWidth               = 1.0f
        };

        vk::PipelineMultisampleStateCreateInfo multisampling{
            .rasterizationSamples = vk::SampleCountFlagBits::e1, 
            .sampleShadingEnable = vk::False
        };

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable         = vk::True,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp        = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp        = vk::BlendOp::eAdd,
            .colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy, 
            .attachmentCount = 1, 
            .pAttachments = &colorBlendAttachment
        };
    
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ 
            .setLayoutCount = 1, 
            .pSetLayouts = &*vkResourceManager->getDescriptorSetLayout(),
            .pushConstantRangeCount = 0 
        };
        
        pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);



        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .colorAttachmentCount = 1, 
            .pColorAttachmentFormats = &swapChainSurfaceFormat.format
        };
        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
            {
                .stageCount          = 2,
                .pStages             = shaders.getShaderStages(),
                .pVertexInputState   = &vertexInputInfo,
                .pInputAssemblyState = &inputAssembly,
                .pViewportState      = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState   = &multisampling,
                .pColorBlendState    = &colorBlending,
                .pDynamicState       = &dynamicState,
                .layout              = pipelineLayout,
                .renderPass          = nullptr
            },
            {
                .colorAttachmentCount = 1, 
                .pColorAttachmentFormats = &swapChainSurfaceFormat.format
            }
        };

        graphicsPipeline = vk::raii::Pipeline(device, nullptr, 
            pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
        );
    }

    // [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const
    // {
    //     vk::ShaderModuleCreateInfo createInfo{ 
    //         .codeSize = code.size() * sizeof(char),
    //         .pCode = reinterpret_cast<const uint32_t*>(code.data())
    //     };

    //     vk::raii::ShaderModule shaderModule{device, createInfo };
    //     return shaderModule;
    // }

    const uint32_t frameIndex = 0;
    void recordCommandBuffer(uint32_t imageIndex)
    {
        commandBuffer.begin({});

        // Transition the image layout for rendering
        transition_image_layout(
            imageIndex,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        // Set up the color attachment
        vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        vk::RenderingAttachmentInfo attachmentInfo = {
                .imageView   = swapChainImageViews[imageIndex],
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp      = vk::AttachmentLoadOp::eClear,
                .storeOp     = vk::AttachmentStoreOp::eStore,
                .clearValue  = clearColor};

        // Set up the rendering info
        vk::RenderingInfo renderingInfo = {
                .renderArea           = {.offset = {0, 0}, .extent = swapChainExtent},
                .layerCount           = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments    = &attachmentInfo};

        // Begin rendering
        commandBuffer.beginRendering(renderingInfo);

        // Rendering commands will go here
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
        commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(swapChainExtent.height), static_cast<float>(swapChainExtent.width), -static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
        // commandBuffer.setViewport(0, vk::Viewport(0, 0, (float)swapChainExtent.width, (float)swapChainExtent.height, 0, 1));
        commandBuffer.setScissor(0, vk::Rect2D({0, 0}, swapChainExtent));
        

        auto& mesh = vkResourceManager->getVkMesh();
        commandBuffer.bindVertexBuffers(0, *mesh.vertexBuffer.buffer, {0});
        commandBuffer.bindIndexBuffer(*mesh.indexBuffer.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, 
            pipelineLayout, 0, 
            *vkResourceManager->getDescriptorSets()[frameIndex], 
            nullptr
        );
        commandBuffer.drawIndexed(mesh.size, 1, 0, 0, 0);
            

        // End rendering
        commandBuffer.endRendering();

        // Transition the image layout for presentation
        transition_image_layout(
            imageIndex,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );

        commandBuffer.end();
    }

    void createCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo{
            .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueIndex
        };

        commandPool = vk::raii::CommandPool(device, poolInfo);
    }

    void createCommandBuffer()
    {
        vk::CommandBufferAllocateInfo allocInfo{ 
            .commandPool = commandPool, 
            .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 
        };
        commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
    }

    void transition_image_layout(
	    uint32_t                imageIndex,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask)
    {
            vk::ImageMemoryBarrier2 barrier = {
                .srcStageMask        = src_stage_mask,
                .srcAccessMask       = src_access_mask,
                .dstStageMask        = dst_stage_mask,
                .dstAccessMask       = dst_access_mask,
                .oldLayout           = old_layout,
                .newLayout           = new_layout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = swapChainImages[imageIndex],
                .subresourceRange    = {
                    .aspectMask     = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1}};
            vk::DependencyInfo dependency_info = {
                .dependencyFlags         = {},
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers    = &barrier};
        commandBuffer.pipelineBarrier2(dependency_info);
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time       = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = vkResourceManager->getVkObject().model;
        ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
    
        memcpy(vkResourceManager->getVkObject().uniformBuffersMapped[currentImage],
            &ubo,
            sizeof(UniformBufferObject)
        );
    }
};