#pragma once

#include "error.h"
#include "time.h"

namespace _SimpleAvZInternal {

class Global : AvZ::GlobalVar {
private:
    const int LAST_SET_TIME_INIT_VALUE = -999999;
    int last_set_time = LAST_SET_TIME_INIT_VALUE;

public:
    bool is_ice_positions_initialized = false;
    bool validate_waves_integrity = false;

    void virtual beforeScript() override
    {
        last_set_time = LAST_SET_TIME_INIT_VALUE;
        is_ice_positions_initialized = false;
        validate_waves_integrity = true;
    }

    // 获得延迟时间, 并且更新[last_set_time]
    int get_delayed_time_and_update(int delay_time, const std::string& func_name)
    {
        if (last_set_time == LAST_SET_TIME_INIT_VALUE) {
            error(func_name + "-->after", "没有延迟的基准, 请先使用固定时间的用炮/用卡函数");
        }
        last_set_time += delay_time;
        return last_set_time;
    }

    // 获得生效时间, 并且更新[last_set_time]
    // 不适用于卡片, 卡片应用[get_card_effect_time]
    int get_effect_time(Time time, const std::string& func_name)
    {
        switch (time.type) {
        case Time::Type::ABS:
            last_set_time = time.time;
            return time.time;
        case Time::Type::REL:
            return get_delayed_time_and_update(time.time, func_name);
        default:
            assert(false);
        }
    }

    void reset_last_set_time()
    {
        last_set_time = LAST_SET_TIME_INIT_VALUE;
    }
};

Global global;

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
