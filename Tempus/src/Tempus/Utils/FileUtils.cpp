// Copyright Levi Spevakow (C) 2025

#include "FileUtils.h"
#include <fstream>
#include "Log.h"
#include <filesystem>
#include <iostream>

std::vector<char> Tempus::FileUtils::ReadFile(const std::string &filename)
{
    // Reading from end of file to determine its size
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    PrintResolvedPath(filename);

    if (!file.is_open()) 
    {
        TPS_CORE_CRITICAL("Failed to open file {0}", filename);
        throw std::runtime_error("Failed to open file!" + filename);
    }

    // Create Buffer
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    // Seek to start
    file.seekg(0);
    // Load buffer
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;

}

void Tempus::FileUtils::PrintResolvedPath(const std::string &relativePath)
{
    try 
    {
        std::filesystem::path absolutePath = std::filesystem::absolute(relativePath);
        std::cout << "Resolved absolute path: " << absolutePath << std::endl;
    } 
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "Error resolving path: " << e.what() << std::endl;
    }
}
