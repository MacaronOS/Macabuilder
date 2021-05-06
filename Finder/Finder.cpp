#include "Finder.h"

#include <filesystem>
#include <glob.h>
#include <vector>

std::vector<std::filesystem::path> Finder::glob(const std::string& pattern)
{
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    int res = ::glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
    std::vector<std::filesystem::path> filenames {};

    if (res >= 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; i++) {
            filenames.emplace_back(glob_result.gl_pathv[i]);
        }
    }

    globfree(&glob_result);

    return filenames;
}
