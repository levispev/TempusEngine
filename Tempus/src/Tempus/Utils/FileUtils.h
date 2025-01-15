// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <vector>
#include <filesystem>

namespace Tempus
{

    class TEMPUS_API FileUtils {

    public:

        static std::vector<char> ReadFile(const std::string& filename);
        static void PrintAbsolutePath(const std::string& relativePath);
        static std::string GetExecutablePath();
        static void SetWorkingDirectory(const std::string& directory);
    };

}