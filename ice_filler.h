#pragma once

#include "global.h"
#include "libavz.h"
#include "time.h"

// 设置存冰位置.
// 若不指定生效时间, 默认在 wave 1, -599cs 生效.
// *** 使用示例:
// setIce({{1, 1}, {1, 2}})-----在1-1, 1-2存冰(优先使用1-2)
// setIce(400, {...})-----------400cs生效
void setIce(int time, const std::vector<AvZ::Grid>& ice_positions)
{
    int max_row = _SimpleAvZInternal::is_backyard() ? 6 : 5;

    for (const auto& pos : ice_positions) {
        auto row = pos.row;
        auto col = pos.col;
        if (row < 1 || row > max_row) {
            _SimpleAvZInternal::error("setIce", "存冰行数应在1~#内\n存冰行数: #", max_row, row);
        }
        if (col < 1 || col > 9) {
            _SimpleAvZInternal::error("setIce", "存冰列数应在1~9内\n存冰列数: #", col);
        }
    }

    AvZ::SetTime(time);
    if (!_SimpleAvZInternal::is_ice_positions_initialized) {
        _SimpleAvZInternal::is_ice_positions_initialized = true;
        AvZ::ice_filler.start(ice_positions);
    } else {
        AvZ::ice_filler.resetFillList(ice_positions);
    }
}

void setIce(const std::vector<AvZ::Grid>& ice_positions)
{
    AvZ::SetTime(-599, 1);
    setIce(-599, ice_positions);
}

// 白昼点冰. 自带生效时机修正.
// 若不指定生效时间, 默认在本波 601cs 生效.
// *** 使用示例:
// I()---------------点冰, 601cs生效(完美预判冰)
// I(after(210))-----延迟210cs生效(ice3), 推荐在激活炮后使用
// I(359)------------359cs生效
void I(Time time)
{
    time.fix_card_time_to_cob = true;
    auto effect_time = _SimpleAvZInternal::get_effect_time(time, {ICE_SHROOM, COFFEE_BEAN});
    AvZ::SetTime(effect_time - 299);
    ice_filler.coffee();
    AvZ::SetPlantActiveTime(ICE_SHROOM, 298);
}

void I()
{
    I(Time(Time::Type::ABS, 601, true));
}