// Copyright Levi Spevakow (C) 2025

#include "FileUtils.h"
#include <fstream>
#include "Log.h"
#include <iostream>

#ifdef TPS_PLATFORM_WINDOWS
#include <direct.h>
#define ChangeDir _chdir
#endif
#ifdef TPS_PLATFORM_MACOS
#include <unistd.h>
#define ChangeDir chdir
#endif

std::vector<char> Tempus::FileUtils::ReadFile(const std::string& filename)
{
    // Reading from end of file to determine its size
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

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

void Tempus::FileUtils::PrintAbsolutePath(const std::string &relativePath)
{
    try 
    {
        std::filesystem::path absolutePath = std::filesystem::absolute(relativePath);
        TPS_CORE_INFO("Resolved absolute path: {0}", absolutePath.string());
    } 
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "Error resolving path: " << e.what() << std::endl;
    }
}

std::string Tempus::FileUtils::GetExecutablePath()
{
    char buffer[MAX_PATH];

#ifdef TPS_PLATFORM_WINDOWS
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
#endif

    std::string path = buffer;
    // Returning parent path of .exe
    return std::filesystem::path(path).parent_path().string();
}

void Tempus::FileUtils::SetWorkingDirectory(const std::string& directory)
{
    if (ChangeDir(directory.c_str())) 
    {
        throw std::runtime_error("Failed to change working directory to: " + directory);
    }
    else 
    {
        TPS_CORE_INFO("Changed working directory to: {0}", std::filesystem::current_path().string());
    }
}
