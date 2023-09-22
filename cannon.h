#pragma once

#include "libavz.h"
#include "global_vars.h"
#include "time.h"

class CobOperator : public AvZ::PaoOperator {
public:
    // 发射激活炮（五行场地炸2、4路，六行场地炸2、5路）
    // 若不指定列数，默认炸9列
    // *** 使用示例：
    // p.PP(278)-------------炸(2,9)与(5,9), 278cs生效
    // p.PP(after(110))------同上，但延迟110cs生效
    // p.PP(278, 8)----------炸(2,8)与(5,8), 278cs生效
    // p.PP(278, {8, 9})-----炸(2,8)与(5,9), 278cs生效
    void PP(Time time, const std::array<float, 2>& cols)
    {
        auto scene = AvZ::GetMainObject()->scene();
        auto row2 = (scene == 2 || scene == 3) ? 5 : 4;
        P(time, {{2, cols[0]}, {row2, cols[1]}});
    }

    void PP(Time time, float col)
    {
        PP(time, {col, col});
    }

    void PP(Time time)
    {
        PP(time, {9, 9});
    }

    // 发射拦截炮（五行场地炸1、4路，六行场地炸1、5路）
    // *** 使用示例：
    // p.DD(278, 8)----------炸(1,8)与(5,8), 278cs生效
    // p.DD(after(110))------同上，但延迟110cs生效
    // p.DD(278, {8, 9})-----炸(1,8)与(5,9), 278cs生效
    void DD(Time time, const std::array<float, 2>& cols)
    {
        auto scene = AvZ::GetMainObject()->scene();
        auto row2 = (scene == 2 || scene == 3) ? 5 : 4;
        P(time, {{1, cols[0]}, {row2, cols[1]}});
    }

    void DD(Time time, float col)
    {
        DD(time, {col, col});
    }

    // 基础发炮函数
    // *** 使用示例:
    // p.P(278, 2, 9)-----------------炸(2,9), 278cs生效
    // p.P(after(110), 2, 9)----------同上，但延迟110cs生效
    // p.P(278, {1, 6}, 9)------------炸(1,9)与(6,9), 278cs生效
    // p.P(278, {{1, 8}, {6, 9}})-----炸(1,8)与(6,9), 278cs生效
    void P(Time time, const std::vector<AvZ::Position>& positions)
    {
        auto effect_time = time.time;
        if (time.type == Time::Type::REL) {
            effect_time = _SimpleAvZInternal::get_last_ash_effect_time() + time.time;
        }
        _SimpleAvZInternal::last_ash_effect_time = effect_time;
        auto scene = AvZ::GetMainObject()->scene();
        
        for (const auto& each : positions) {
            auto row = each.row;
            auto col = each.col;
            if (scene >= 4) {
                AvZ::SetTime(effect_time - 387);
                PaoOperator::roofPao(row, col);
            } else if (AvZ::RangeIn(scene, {2, 3}) && AvZ::RangeIn(row, {3, 4})) {
                AvZ::SetTime(effect_time - 378);
                PaoOperator::pao(row, col);
            } else {
                AvZ::SetTime(effect_time - 373);
                PaoOperator::pao(row, col);
            }
        }
    }

    void P(Time time, const std::vector<int> rows, float col)
    {
        std::vector<AvZ::Position> positions;
        for (const auto& row : rows)
            positions.push_back({row, col});
        P(time, positions);
    }

    void P(Time time, int row, float col)
    {
        P(time, {{row, col}});
    }

    void virtual beforeScript() override
    {
        initialState();
        AvZ::SetTime(-599, 1);
        autoGetPaoList();
    }
};

CobOperator cob_operator;

// 发射激活炮（五行场地炸2、4路，六行场地炸2、5路）
// 若不指定列数，默认炸9列
// *** 使用示例：
// PP(278)-------------炸(2,9)与(5,9), 278cs生效
// PP(278, 8)----------炸(2,8)与(5,8), 278cs生效
// PP(278, {8, 9})-----炸(2,8)与(5,9), 278cs生效
void PP(Time time, const std::array<float, 2>& cols)
{
    cob_operator.PP(time, cols);
}

void PP(Time time, float col)
{
    cob_operator.PP(time, col);
}

void PP(Time time)
{
    cob_operator.PP(time);
}

// 发射拦截炮（五行场地炸1、4路，六行场地炸1、5路）
// *** 使用示例：
// DD(278, 8)----------炸(1,8)与(5,8), 278cs生效
// DD(after(110))------同上，但延迟110cs生效
// DD(278, {8, 9})-----炸(1,8)与(5,9), 278cs生效
void DD(Time time, const std::array<float, 2>& cols)
{
    cob_operator.DD(time, cols);
}

void DD(Time time, float col)
{
    cob_operator.DD(time, col);
}

// 基础发炮函数
// *** 使用示例:
// P(278, 2, 9)-----------------炸(2,9), 278cs生效
// P(after(110), 2, 9)----------同上，但延迟110cs生效
// P(278, {1, 6}, 9)------------炸(1,9)与(6,9), 278cs生效
// P(278, {{1, 8}, {6, 9}})-----炸(1,8)与(6,9), 278cs生效
void P(Time time, const std::vector<AvZ::Position>& positions)
{
    cob_operator.P(time, positions);
}

void P(Time time, const std::vector<int> rows, float col)
{
    cob_operator.P(time, rows, col);
}

void P(Time time, int row, float col)
{
    cob_operator.P(time, row, col);
}

// namespace _SimpleAvZInternal {

// class GlobalCobOperatorInit : AvZ::GlobalVar {
// public:
//     void virtual beforeScript() override
//     {
//         AvZ::SetTime(-599, 1);
//         cob_operator.autoGetPaoList();
//     }
// };

// GlobalCobOperatorInit _global_cob_operator_init;
// } // namespace _SimpleAvZInternal