#pragma once

#include "error.h"
#include "global.h"
#include "libavz.h"
#include "time.h"
#include "util.h"

class CobOperator : public AvZ::PaoOperator {
private:
    std::vector<int> cob_cols;

public:
    // 创建 CobOperator. 指定要用炮尾在哪些列的炮.
    // *** 使用示例:
    // CobOperator c1(1)---------只用炮尾在1列的炮
    // CobOperator c45(4, 5)-----只用炮尾在4或5列的炮
    template <typename... Args>
    CobOperator(Args... args)
        : AvZ::PaoOperator()
    {
        for (const auto col : {args...}) {
            if (col < 1 || col > 8)
                _SimpleAvZInternal::error("CobOperator 构造函数", "炮尾列应在1~8内\n炮尾列: #", col);
            else if (_SimpleAvZInternal::contains(cob_cols, col))
                _SimpleAvZInternal::error("CobOperator 构造函数", "不可重复指定炮尾列\n重复的炮尾列: #", col);
            else
                cob_cols.push_back(col);
        }
    }

    CobOperator()
        : AvZ::PaoOperator()
    {
    }

    // 发射一组并炸炮.
    // 若省略行数, 默认六行场地炸 2,5 路, 五行场地炸 2,4 路. 若省略列数, 默认炸 9 列.
    // *** 使用示例:
    // c.PP(318)-----------------炸(2,9)与(5,9), 318cs生效
    // c.PP(318, 8)--------------炸(2,8)与(5,8)
    // c.PP(318, {8, 9})---------炸(2,8)与(5,9)
    // c.PP(318, {2, 6}, 9)------炸(2,9)与(6,9)
    // c.PP(after(110), ...)-----用法同上, 延迟110cs生效
    void PP(Time time, const std::array<int, 2>& rows, float col)
    {
        cob_internal(time, {{rows[0], col}, {rows[1], col}}, "PP");
    }

    void PP(Time time, const std::array<float, 2>& cols)
    {
        auto row2 = _SimpleAvZInternal::is_backyard() ? 5 : 4;
        cob_internal(time, {{2, cols[0]}, {row2, cols[1]}}, "PP");
    }

    void PP(Time time, float col)
    {
        PP(time, {col, col});
    }

    void PP(Time time)
    {
        PP(time, {9, 9});
    }

    // 发射一组拦截炮.
    // 若省略行数, 六行场地炸 1,5 路, 五行场地炸 1,4 路. 若省略列数, 默认炸 9 列.
    // *** 使用示例:
    // c.DD(318)-----------------炸(1,9)与(5,9), 318cs生效
    // c.DD(318, 8)--------------炸(1,8)与(5,8)
    // c.DD(318, {8, 9})---------炸(1,8)与(5,9)
    // c.DD(318, {2, 5}, 3.5)----炸(2,3.5)与(5,3.5)
    // c.DD(after(110), ...)-----用法同上, 延迟110cs生效
    void DD(Time time, const std::array<int, 2>& rows, float col)
    {
        std::vector<AvZ::Position> positions;
        for (const auto& row : rows)
            positions.push_back({row, col});
        cob_internal(time, positions, "DD");
    }

    void DD(Time time, const std::array<float, 2>& cols)
    {
        auto row2 = _SimpleAvZInternal::is_backyard() ? 5 : 4;
        cob_internal(time, {{1, cols[0]}, {row2, cols[1]}}, "DD");
    }

    void DD(Time time, float col)
    {
        DD(time, {col, col});
    }

    void DD(Time time)
    {
        DD(time, {9, 9});
    }

    // 发射一门炮.
    // 可提供单个落点, 多行同列, 或多个落点. 也可指明要用哪门炮.
    // *** 使用示例:
    // P(318, 2, 9)-----------------炸(2,9), 318cs生效
    // P(318, 1, 1, 2, 8)-----------使用1-1炮炸(2,8)
    // P(after(110), ...)-----------用法同上, 延迟110cs生效
    void P(Time time, int row, float col)
    {
        cob_internal(time, {{row, col}}, "P");
    }

    void P(Time time, int cob_row, int cob_col, int row, float col)
    {
        cob_internal(time, {{row, col}}, "P", {cob_row, cob_col});
    }

    // 发射分离炮.
    // *** 使用示例:
    // c.B(304, 5, 8.225)-----炸(5, 8.225), 304cs生效
    void B(Time time, int row, float col)
    {
        cob_internal(time, {{row, col}}, "B");
    }

    // 发射拦截炮.
    // *** 使用示例:
    // c.D(395, 1, 9)-----炸(1,9), 395cs生效
    void D(Time time, int row, float col)
    {
        cob_internal(time, {{row, col}}, "D");
    }

    // 不使用特定炮.
    // *** 使用用例:
    // c.excludeCob(3, 5)---------不使用3-5炮, 游戏开始时起效
    // c.excludeCob(400, ...)-----400cs起效
    void excludeCob(int time, int row, int col)
    {
        int max_row = _SimpleAvZInternal::is_backyard() ? 6 : 5;

        if (row < 1 || row > max_row) {
            _SimpleAvZInternal::error("excludeCob", "炮行数应在1~#内\n炮行数: #", max_row, row);
        }
        if (col < 1 || col > 8) {
            _SimpleAvZInternal::error("excludeCob", "炮尾列应在1~8内\n炮尾列: #", col);
        }

        // 保留原定炮序
        AvZ::SetTime(time);
        AvZ::InsertOperation([=]() {
            auto prev_index_vec = pao_index_vec;
            auto excluded_index = AvZ::GetPlantIndex(row, col, COB_CANNON);
            std::vector<AvZ::Grid> valid_cobs;

            auto plant_array = AvZ::GetMainObject()->plantArray();
            for (const auto& index : pao_index_vec) {
                if (index != excluded_index)
                    valid_cobs.push_back({plant_array[index].row() + 1,
                        plant_array[index].col() + 1});
            }

            AvZ::InsertGuard _(false);
            resetPaoList(valid_cobs);
        },
            "CobOperator::excludeCob");
    }

    void excludeCob(int row, int col)
    {
        AvZ::SetTime(-599, 1);
        excludeCob(-599, row, col);
    }

    // 重置为使用所有炮.
    // *** 使用用例:
    // c.resetCob(400)-----重置为使用所有炮, 400cs起效
    void resetCob(int time)
    {
        AvZ::SetTime(time);
        autoGetPaoList();
    }

    void virtual beforeScript() override
    {
        initialState();
        sequential_mode = TIME;
        next_pao = 0;

        if (cob_cols.empty()) {
            AvZ::SetTime(-599, 1);
            autoGetPaoList();
        } else {
            AvZ::InsertTimeOperation(
                -599, 1, [=]() {
                    std::vector<AvZ::Grid> valid_cobs;
                    for (auto& p : AvZ::alive_plant_filter) {
                        if (p.type() == COB_CANNON && _SimpleAvZInternal::contains(cob_cols, p.col() + 1))
                            valid_cobs.push_back({p.row() + 1, p.col() + 1});
                    }

                    std::sort(valid_cobs.begin(), valid_cobs.end()); // 按炮位置排序

                    AvZ::InsertGuard _(false);
                    resetPaoList(valid_cobs);
                },
                "CobOperator::beforeScript");
        }
    }

private:
    // 主发炮函数
    void cob_internal(Time time, const std::vector<AvZ::Position>& positions, const std::string& func_name,
        const AvZ::Grid& specified_cob = {-1, -1})
    {
        int max_row = _SimpleAvZInternal::is_backyard() ? 6 : 5;

        for (const auto& pos : positions) {
            if (pos.row < 1 || pos.row > max_row) {
                _SimpleAvZInternal::error(func_name, "落点行应在1~#内\n落点行: #", max_row, pos.row);
            }
            if (pos.col < 0 || pos.col > 10) {
                _SimpleAvZInternal::error(func_name, "落点列应在0.0~10.0内\n落点列: #", pos.col);
            }
        }

        bool specify_cob = false;
        if (specified_cob.row != -1 || specified_cob.col != -1) {
            specify_cob = true;
            if (specified_cob.row < 1 || specified_cob.row > max_row) {
                _SimpleAvZInternal::error(func_name, "要用的炮行数应在1~#内\n炮行数: #", max_row, specified_cob.row);
            }
            if (specified_cob.col < 1 || specified_cob.col > 8) {
                _SimpleAvZInternal::error(func_name, "要用的炮尾列应在1~8内\n炮尾列: #", specified_cob.col);
            }
        }

        auto effect_time = _SimpleAvZInternal::global.get_effect_time(time, func_name);

        for (const auto& pos : positions) {
            auto row = pos.row;
            auto col = pos.col;
            if (_SimpleAvZInternal::is_roof()) {
                AvZ::SetTime(effect_time - 387);
                if (specify_cob)
                    PaoOperator::rawRoofPao(specified_cob.row, specified_cob.col, row, col);
                else
                    PaoOperator::roofPao(row, col);
            } else if (_SimpleAvZInternal::is_backyard() && AvZ::RangeIn(row, {3, 4})) {
                AvZ::SetTime(effect_time - 378);
                if (specify_cob)
                    PaoOperator::rawPao(specified_cob.row, specified_cob.col, row, col);
                else
                    PaoOperator::pao(row, col);
            } else {
                AvZ::SetTime(effect_time - 373);
                if (specify_cob)
                    PaoOperator::rawPao(specified_cob.row, specified_cob.col, row, col);
                else
                    PaoOperator::pao(row, col);
            }
        }
    }
};

CobOperator cob_operator;

// 发射一组并炸炮.
// 若省略行数, 默认六行场地炸 2,5 路, 五行场地炸 2,4 路. 若省略列数, 默认炸 9 列.
// *** 使用示例:
// PP(318)-----------------炸(2,9)与(5,9), 318cs生效
// PP(318, 8)--------------炸(2,8)与(5,8)
// PP(318, {8, 9})---------炸(2,8)与(5,9)
// PP(318, {2, 6}, 9)------炸(2,9)与(6,9)
// PP(after(110), ...)-----用法同上, 延迟110cs生效
void PP(Time time, const std::array<int, 2>& rows, float col)
{
    cob_operator.PP(time, rows, col);
}

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

// 发射一组拦截炮.
// 若省略行数, 六行场地炸 1,5 路, 五行场地炸 1,4 路. 若省略列数, 默认炸 9 列.
// *** 使用示例:
// DD(318)-----------------炸(1,9)与(5,9), 318cs生效
// DD(318, 8)--------------炸(1,8)与(5,8)
// DD(318, {8, 9})---------炸(1,8)与(5,9)
// PP(318, {2, 5}, 3.5)----炸(2,3.5)与(5,3.5)
// DD(after(110), ...)-----用法同上, 延迟110cs生效
void DD(Time time, const std::array<int, 2>& rows, float col)
{
    cob_operator.DD(time, rows, col);
}

void DD(Time time, const std::array<float, 2>& cols)
{
    cob_operator.DD(time, cols);
}

void DD(Time time, float col)
{
    cob_operator.DD(time, col);
}

void DD(Time time)
{
    cob_operator.DD(time);
}

// 发射一门炮. 可指明要用哪门炮.
// *** 使用示例:
// P(318, 2, 9)-----------------炸(2,9), 318cs生效
// P(318, 1, 1, 2, 8)-----------使用1-1炮炸(2,8)
// P(after(110), ...)-----------用法同上, 延迟110cs生效
void P(Time time, int row, float col)
{
    cob_operator.P(time, row, col);
}

// 发射分离炮.
// *** 使用示例:
// B(304, 5, 8.225)-----炸(5, 8.225), 304cs生效
void B(Time time, int row, float col)
{
    cob_operator.B(time, row, col);
}

// 发射拦截炮.
// *** 使用示例:
// D(395, 1, 9)-----炸(1,9), 395cs生效
void D(Time time, int row, float col)
{
    cob_operator.D(time, row, col);
}

// 不使用特定炮.
// *** 使用用例:
// excludeCob(3, 5)---------不使用3-5炮, 游戏开始时起效
// excludeCob(400, ...)-----400cs起效
void excludeCob(int time, int row, int col)
{
    cob_operator.excludeCob(time, row, col);
}

void excludeCob(int row, int col)
{
    cob_operator.excludeCob(row, col);
}

// 重置为使用所有炮.
// *** 使用用例:
// resetCob(400)-----重置为使用所有炮, 400cs起效
void resetCob(int time)
{
    cob_operator.resetCob(time);
}