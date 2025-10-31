// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <vector>
#include <string>
#include <filesystem>

namespace Tempus
{

    class TEMPUS_API FileUtils {

    public:

        static std::vector<unsigned char> ReadFile(const std::string& filename);
        static void PrintAbsolutePath(const std::string& relativePath);
        static std::string GetExecutablePath();
        static void SetWorkingDirectory(const std::string& directory);
        static std::string GetWorkingDirectory();
        static void OpenDirectory(const std::string& directory);

        // @TODO May want to move all of these paths to a separate TempusPaths class
        static std::filesystem::path ProjectRoot()
        {
            return std::filesystem::path(TPS_PROJECT_ROOT);
        }
        
        static std::filesystem::path ContentDir()
        {
            return std::filesystem::path(TPS_CONTENT_DIR);
        }
        
        static std::filesystem::path ShaderDir()
        {
            return std::filesystem::path(TPS_SHADER_DIR);
        }

        static std::filesystem::path TextureDir()
        {
            return std::filesystem::path(TPS_TEXTURE_DIR);
        }
        
        static std::filesystem::path Shader(const std::string& shaderName)
        {
            return ShaderDir() / shaderName;
        }
        
        static std::filesystem::path Texture(const std::string& textureName)
        {
            return TextureDir() / textureName;
        }
        
        static std::filesystem::path Content(const std::string& relativePath)
        {
            return ContentDir() / relativePath;
        }
        
        static std::filesystem::path Project(const std::string& relativePath)
        {
            return ProjectRoot() / relativePath;
        }
        
        static std::filesystem::path LogsDir()
        {
            auto logsPath = ProjectRoot() / "logs";
            if (!std::filesystem::exists(logsPath))
            {
                std::filesystem::create_directories(logsPath);
            }
            return logsPath;
        }
    };

}