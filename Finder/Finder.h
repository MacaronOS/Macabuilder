#pragma once

#include "../utils/Utils.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

class Finder {
    static constexpr auto BeelderExtensionPattern = "*.bee";

public:
    static inline auto FindRootBeelderFiles()
    {
        return glob(BeelderExtensionPattern);
    }
    static inline auto FindBeelderFiles(const std::string& directory, const std::string& pattern = "")
    {
        auto path = std::filesystem::path(directory) / std::filesystem::path(pattern) / BeelderExtensionPattern;
        return glob(path);
    }

    static inline auto FindFiles(const std::string& directory, const std::string& pattern)
    {
        auto path = std::filesystem::path(directory) / std::filesystem::path(pattern);
        return glob(path);
    }

    static inline auto GetExtension(const std::string& filename)
    {
        return std::filesystem::path(filename).extension();
    }

    static inline auto GetFolder(const std::string& path)
    {
        return std::filesystem::path(path).parent_path();
    }

    static inline void CreateDirectory(const std::string& directory)
    {
        auto dirs = Utils::Split(directory, "/");
        std::filesystem::path cur_path = dirs[0];

        for (size_t at = 1; at < dirs.size(); at++) {
            if (!std::filesystem::exists(cur_path)) {
                std::filesystem::create_directory(cur_path);
            }

            cur_path = cur_path / dirs[at];
        }

        if (!std::filesystem::exists(cur_path)) {
            std::filesystem::create_directory(cur_path);
        }
    }

    static std::vector<std::filesystem::path> glob(const std::string& pattern);
};
