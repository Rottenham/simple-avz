// This file is for pure C++ helper functions only.

#pragma once

#include <string>
#include <vector>
#include <numeric>

namespace _SimpleAvZInternal {

std::string concat(const std::vector<std::string>& strings, std::string sep)
{
    std::string output = "";
    for (const auto& str : strings) {
        if (!output.empty())
            output += sep;
        output += str;
    }
    return output;
}

template <typename T>
bool contains(const std::vector<T>& vec, const T& elem)
{
    return std::find(vec.begin(), vec.end(), elem) != vec.end();
}

} // namespace _SimpleAvZInternal
