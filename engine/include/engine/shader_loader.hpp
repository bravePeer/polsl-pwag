#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

class ShaderLoader
{
public:
    ShaderLoader(vk::raii::Device& device, const std::string& pathToShader)
    : device(device), shaderModule(createShaderModule(readFile(pathToShader)))
    {
        shaderStages[0] = { 
            .stage = vk::ShaderStageFlagBits::eVertex, 
            .module = shaderModule,  
            .pName = "vertMain" 
        };
        shaderStages[1] = { 
            .stage = vk::ShaderStageFlagBits::eFragment, 
            .module = shaderModule, 
            .pName = "fragMain" 
        };
    }

    ShaderLoader() = delete;
    ShaderLoader(const ShaderLoader&) = delete;

    vk::PipelineShaderStageCreateInfo* getShaderStages()
    {
        return shaderStages;
    }

private:
    vk::raii::Device& device;
    vk::raii::ShaderModule shaderModule;
    vk::PipelineShaderStageCreateInfo shaderStages[2];

    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{ 
            .codeSize = code.size() * sizeof(char),
            .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };

        vk::raii::ShaderModule shaderModule{device, createInfo};
        return shaderModule;
    }

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename + "");
        }

        std::vector<char> buffer(file.tellg());

        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

        file.close();
        return buffer;
    }
};