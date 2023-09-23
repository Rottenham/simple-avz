#pragma once

#include "libavz.h"

namespace _SimpleAvZInternal {

template <typename... Args>
void error(const std::string& func_name, const std::string& content, Args... args)
{
    AvZ::SetGameSpeed(1); // 恢复至游戏原速, 主要是因为 SelectCardsPlus 没有适配运行时异常的情况

    AvZ::SetErrorMode(AvZ::POP_WINDOW);
    AvZ::ShowErrorNotInQueue("[调用 " + func_name + " 时出错]\n" + content, args...);

    throw "";
}

} // namespace _SimpleAvZInternal
