#pragma once

#include "libavz.h"

namespace _SimpleAvZInternal {

template <typename... Args>
void error(const std::string& content, Args... args)
{
    AvZ::SetErrorMode(AvZ::POP_WINDOW);
    AvZ::ShowErrorNotInQueue(content, args...);
    throw "";
}

} // namespace _SimpleAvZInternal
