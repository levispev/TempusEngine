// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include <vector>
#include <iostream>

namespace Tempus
{

    class TEMPUS_API FileUtils {

    public:

        static std::vector<char> ReadFile(const std::string& filename);
        static void PrintResolvedPath(const std::string& relativePath);

    };


}