#include "sim_assets.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
std::vector<uint8_t> s_font_data;
}

const uint8_t* SimAssets::fontData(size_t* size)
{
    if (s_font_data.empty()) {
        const std::string path = std::string(SIM_REPO_ROOT) + "/main/assets/NotoSansSCSubset.ttf";
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            std::cerr << "failed to open font asset: " << path << "\n";
        } else {
            s_font_data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }
    }

    if (size) {
        *size = s_font_data.size();
    }
    return s_font_data.empty() ? nullptr : s_font_data.data();
}
