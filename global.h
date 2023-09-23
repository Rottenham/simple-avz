#pragma once

#include "error.h"

namespace _SimpleAvZInternal {

int last_set_time = -1;
bool is_ice_positions_initialized = false;

// 获得带延迟的时间, 并且更新[last_set_time]
int get_delayed_time_and_update(int delay_time)
{
    if (last_set_time < 0) {
        error("after", "没有延迟的基准, 请先使用固定时间的用炮/用卡函数");
    }
    last_set_time += delay_time;
    return last_set_time;
}

// 设定时间, 并且更新[last_set_time]
void set_time_and_update(int time)
{
    AvZ::SetTime(time);
    last_set_time = time;
}

// scene 相关
bool is_night()
{
    auto scene = AvZ::GetMainObject()->scene();
    return (scene > 4) || (scene % 2 == 1);
}

bool is_backyard()
{
    auto scene = AvZ::GetMainObject()->scene();
    return (scene == 2) || (scene == 3);
}

bool is_roof()
{
    auto scene = AvZ::GetMainObject()->scene();
    return scene >= 4;
}

} // namespace _SimpleAvZInternal
