#pragma once

#include "error.h"
#include "global.h"
#include "libavz.h"
#include "time.h"
#include "util.h"

namespace _SimpleAvZInternal {

PlantType non_imitater(const PlantType& plant_type)
{
    return (plant_type > IMITATOR) ? (PlantType)(plant_type - IMITATOR - 1) : plant_type;
}

// 是否为一次性植物(白天咖啡豆+冰/核的组合也视为一次性)
bool is_instant(std::vector<PlantType> plant_types)
{
    for (auto& plant_type : plant_types)
        plant_type = non_imitater(plant_type);

    for (const auto& plant_type : plant_types) {
        if (is_night() && contains({ICE_SHROOM, DOOM_SHROOM}, plant_type))
            return true;
        if (!is_night() && contains({ICE_SHROOM, DOOM_SHROOM}, plant_type) && contains(plant_types, COFFEE_BEAN))
            return true;
        if (contains({CHERRY_BOMB, JALAPENO, SQUASH}, plant_type))
            return true;
    }
    return false;
}

int get_card_effect_time(Time time, const std::vector<PlantType>& plant_types, const std::string& func_name)
{
    auto effect_time = global.get_effect_time(time, func_name);
    if (time.fix_card_time_to_cob && is_instant(plant_types))
        return effect_time + 1;
    else
        return effect_time;
}

int get_prep_time(const PlantType& plant_type, const std::vector<PlantType>& all_plants)
{
    int prep_time = 0;
    auto non_im_plant_type = non_imitater(plant_type);

    if (contains({LILY_PAD, FLOWER_POT}, non_im_plant_type)) {
        if (all_plants.empty() || (all_plants.size() == 1 && all_plants.at(0) == plant_type))
            return 0;
        else
            return (plant_type > IMITATOR) ? -2000 : -1000; // 之后特殊处理
    }

    if (contains({CHERRY_BOMB, JALAPENO}, non_im_plant_type)) {
        prep_time = 100;
    } else if (non_im_plant_type == SQUASH) {
        prep_time = 182;
    } else if (contains({ICE_SHROOM, DOOM_SHROOM}, non_im_plant_type)) {
        if (is_night())
            prep_time = 100;
        else if (contains(all_plants, COFFEE_BEAN))
            prep_time = 299;
        else if (contains(all_plants, M_COFFEE_BEAN))
            prep_time = 299 + 320;
    } else if (non_im_plant_type == COFFEE_BEAN) {
        if (contains(all_plants, {ICE_SHROOM}) || contains(all_plants, {DOOM_SHROOM})
            || contains(all_plants, {M_ICE_SHROOM}) || contains(all_plants, {M_DOOM_SHROOM}))
            prep_time = 299;
        else
            return 0;
    } else {
        return 0;
    }

    if (plant_type > IMITATOR)
        prep_time += 320;

    return prep_time;
}

int get_prep_time(const PlantType& plant_type)
{
    return get_prep_time(plant_type, {{plant_type}});
}

std::vector<int> get_prep_times(const std::vector<PlantType>& plant_types)
{
    std::vector<int> prep_times;
    for (const auto& plant_type : plant_types)
        prep_times.push_back(get_prep_time(plant_type, plant_types));

    // 特殊处理容器(荷叶, 花盆)
    for (auto& prep_time : prep_times) {
        if (prep_time < 0) {
            if (prep_time == -1000)
                prep_time = *std::max_element(prep_times.begin(), prep_times.end());
            else if (prep_time == -2000)
                prep_time = *std::max_element(prep_times.begin(), prep_times.end()) + 320;
            prep_time = (prep_time < 0) ? 0 : prep_time;
        }
    }

    return prep_times;
}

std::vector<bool> get_set_active_time_flags(const std::vector<PlantType>& plant_types)
{
    std::vector<bool> set_active_time_flags;

    for (const auto& plant_type : plant_types) {
        bool flag = false;
        bool is_imitater = plant_type > IMITATOR;
        auto non_im_plant_type = non_imitater(plant_type);

        if (contains({CHERRY_BOMB, JALAPENO}, non_im_plant_type)) {
            flag = is_imitater;
        } else if (contains({ICE_SHROOM, DOOM_SHROOM}, non_im_plant_type)) {
            if (is_night())
                flag = is_imitater;
            else if (contains(plant_types, COFFEE_BEAN) || contains(plant_types, M_COFFEE_BEAN))
                flag = true;
        }

        set_active_time_flags.push_back(flag);
    }

    return set_active_time_flags;
}

bool get_set_active_time_flag(const PlantType& plant_type)
{
    return get_set_active_time_flags({{plant_type}}).front();
}

void remove_all(int time, int row, int col)
{
    AvZ::SetTime(time);
    AvZ::InsertOperation([=]() {
        auto plant_num = 0;
        for (auto& p : AvZ::alive_plant_filter) {
            if (p.row() + 1 == row) {
                if (p.col() + 1 == col)
                    plant_num++;
                else if (p.col() == col && p.type() == COB_CANNON)
                    plant_num++;
            }
        }
        for (int i = 0; i < plant_num; i++)
            AvZ::ShovelNotInQueue(row, col);
    },
        "remove_all");
}

} // namespace _SimpleAvZInternal

// 铲除植物. 可提供单一坐标, 多行同列. 也可指定要铲除的植物种类.
// *** 使用示例:
// RM(400, SUNFLOWER)-----------于400cs铲除场地上所有小向
// RM(400, PUMPKIN, 1, 1)-------铲除1-1南瓜（没有则不铲）
// RM(400, 1, 1)----------------铲除1-1, 优先铲除非南瓜
// RM(400, {1, 2, 5, 6}, 9)-----铲除1,2,5,6路9列, 优先铲除非南瓜
// RM(after(751), ...)----------用法同上, 延迟751cs生效
void RM(Time time, PlantType target)
{
    target = _SimpleAvZInternal::non_imitater(target);
    if (target == GRAVE_BUSTER) {
        _SimpleAvZInternal::error("RM", "墓碑吞噬者无法铲除");
    }

    auto effect_time = _SimpleAvZInternal::global.get_effect_time(time, "RM");

    AvZ::SetTime(effect_time);
    AvZ::InsertOperation([=]() {
        for (auto& p : AvZ::alive_plant_filter) {
            if (p.type() == target) {
                AvZ::ShovelNotInQueue(p.row() + 1, p.col() + 1, target == PUMPKIN);
            }
        }
    },
        "RM");
}

void RM(Time time, PlantType target, int row, int col)
{
    int max_row = _SimpleAvZInternal::is_backyard() ? 6 : 5;
    if (row < 1 || row > max_row) {
        _SimpleAvZInternal::error("RM", "铲除行应在1~#内\n铲除行: #", max_row, row);
    }
    if (col < 1 || col > 9) {
        _SimpleAvZInternal::error("RM", "铲除列应在1~9内\n铲除列: #", col);
    }

    target = _SimpleAvZInternal::non_imitater(target);
    if (target == GRAVE_BUSTER) {
        _SimpleAvZInternal::error("RM", "墓碑吞噬者无法铲除");
    }

    auto effect_time = _SimpleAvZInternal::global.get_effect_time(time, "RM");

    AvZ::SetTime(effect_time);
    AvZ::InsertOperation([=]() {
        bool found = false;

        for (auto& p : AvZ::alive_plant_filter) {
            if (p.type() == target && p.row() + 1 == row) {
                if (p.col() + 1 == col)
                    found = true;
                else if (p.type() == COB_CANNON && p.col() + 2 == col)
                    found = true;
            }
        }

        if (found) {
            AvZ::ShovelNotInQueue(row, col, target == PUMPKIN);
        }
    },
        "RM");
}

void RM(Time time, const std::vector<int>& rows, int col)
{
    auto effect_time = _SimpleAvZInternal::global.get_effect_time(time, "RM");

    AvZ::SetTime(effect_time);
    for (const auto& row : rows) {
        AvZ::Shovel(row, col);
    }
}

void RM(Time time, int row, int col)
{
    RM(time, {{row}}, col);
}

// 夜间用原版冰. 自带生效时机修正.
// 若不指定生效时间, 默认在本波 601cs 生效.
// *** 使用示例:
// I(1, 2)------------------于1-2放置原版冰, 601cs生效(完美预判冰)
// I(after(210), 1, 2)------延迟210cs生效(ice3), 推荐在激活炮后使用
// I(359, 1, 2)-------------359cs生效
void I(Time time, int row, int col)
{
    if (!_SimpleAvZInternal::is_night()) {
        _SimpleAvZInternal::error("I", "不可在白昼使用夜间版本的I()\n无需提供用冰位置");
    }

    auto effect_time = _SimpleAvZInternal::get_card_effect_time(time, {ICE_SHROOM}, "I");
    AvZ::SetTime(effect_time - 100);
    AvZ::Card(ICE_SHROOM, row, col);
}

void I(int row, int col)
{
    I(601, row, col);
}

// 夜间用复制冰. 自带生效时机修正.
// 若不指定生效时间, 默认在本波 601cs 生效.
// *** 使用示例:
// M_I(1, 2)------------------于1-2放置复制冰, 601cs生效(完美预判冰)
// M_I(after(210), 1, 2)------延迟210cs生效(ice3), 推荐在激活炮后使用
// M_I(359, 1, 2)-------------359cs生效
void M_I(Time time, int row, int col)
{
    if (!_SimpleAvZInternal::is_night()) {
        _SimpleAvZInternal::error("M_I", "M_I为夜间复制冰函数, 不可在白昼使用");
    }

    auto effect_time = _SimpleAvZInternal::get_card_effect_time(time, {M_ICE_SHROOM}, "M_I");
    AvZ::SetTime(effect_time - 420);
    AvZ::Card(M_ICE_SHROOM, row, col);
    AvZ::SetPlantActiveTime(ICE_SHROOM, 419);
}

void M_I(int row, int col)
{
    M_I(601, row, col);
}

// 用卡. 可提供单一坐标, 多行同列, 或多卡同坐标.
// *** 使用示例:
// C(359, CHERRY, 2, 9)-------------------于2-9放置樱桃, 359cs生效
// C(400, {PUFF, SUN}, {1, 2}, 9)---------于1-9放置小喷, 2-9放置阳光菇
// C(359, {LILY, DOOM, COFFEE}, 3, 9)-----于3-9放置荷叶, 核武, 咖啡豆
//
// *** 生效时间的变种:
// C(after(110), ...)-----------用法同上, 延迟110cs生效
// C(exact(800), ...)-----------不使用炮等效时间, 800cs生效
// C(after(exact(..)), ...)-----以上两者的结合
//
// *** 指定铲除时机:
// C(400, keep(266), ...)-------放置后266cs铲
// C(400, until(1036), ...)-----1036cs铲
// 注意: 容器会被一并铲除.
void C(Time time, ShovelTime shovel_time, const std::vector<PlantType>& plant_types, const std::vector<int>& rows, int col)
{
    if (plant_types.empty()) {
        _SimpleAvZInternal::error("C", "要用的卡片不可为空");
    }
    if (rows.empty()) {
        _SimpleAvZInternal::error("C", "用卡行数不可为空");
    }
    if (plant_types.size() != rows.size()) {
        _SimpleAvZInternal::error("C", "卡片数与行数不一致\n卡片数: #\n行数: #", plant_types.size(), rows.size());
    }
    auto effect_time = _SimpleAvZInternal::get_card_effect_time(time, plant_types, "C");

    for (int i = 0; i < plant_types.size(); i++) {
        auto plant_type = plant_types.at(i);
        auto row = rows.at(i);

        auto prep_time = _SimpleAvZInternal::get_prep_time(plant_type);
        auto set_active_time = _SimpleAvZInternal::get_set_active_time_flag(plant_type);

        AvZ::SetTime(effect_time - prep_time);
        AvZ::Card(plant_type, row, col);
        if (set_active_time)
            AvZ::SetPlantActiveTime(_SimpleAvZInternal::non_imitater(plant_type), prep_time - 1);

        if (shovel_time.type != ShovelTime::Type::NONE) {
            if (shovel_time.type == ShovelTime::Type::KEEP)
                _SimpleAvZInternal::remove_all(effect_time + shovel_time.time, row, col);
            else if (shovel_time.type == ShovelTime::Type::UNTIL)
                _SimpleAvZInternal::remove_all(shovel_time.time, row, col);
        }
    }
}

void C(Time time, const std::vector<PlantType>& plant_types, const std::vector<int>& rows, int col)
{
    C(time, ShovelTime(), plant_types, rows, col);
}

void C(Time time, ShovelTime shovel_time, const std::vector<PlantType>& plant_types, int row, int col)
{
    if (plant_types.empty()) {
        _SimpleAvZInternal::error("C", "要用的卡片不可为空");
    }
    auto effect_time = _SimpleAvZInternal::get_card_effect_time(time, plant_types, "C");
    auto prep_times = _SimpleAvZInternal::get_prep_times(plant_types);
    auto set_active_time_flags = _SimpleAvZInternal::get_set_active_time_flags(plant_types);

    for (int i = 0; i < plant_types.size(); i++) {
        auto plant_type = plant_types.at(i);
        auto prep_time = prep_times.at(i);
        auto set_active_time = set_active_time_flags.at(i);

        AvZ::SetTime(effect_time - prep_time);
        AvZ::Card(plant_type, row, col);
        if (set_active_time)
            AvZ::SetPlantActiveTime(_SimpleAvZInternal::non_imitater(plant_type), prep_time - 1);

        if (shovel_time.type != ShovelTime::Type::NONE) {
            if (shovel_time.type == ShovelTime::Type::KEEP)
                _SimpleAvZInternal::remove_all(effect_time + shovel_time.time, row, col);
            else if (shovel_time.type == ShovelTime::Type::UNTIL)
                _SimpleAvZInternal::remove_all(shovel_time.time, row, col);
        }
    }
}

void C(Time time, const std::vector<PlantType>& plant_types, int row, int col)
{
    C(time, ShovelTime(), plant_types, row, col);
}

void C(Time time, ShovelTime shovel_time, PlantType plant_type, int row, int col)
{
    C(time, shovel_time, {{plant_type}}, {{row}}, col);
}

void C(Time time, PlantType plant_type, int row, int col)
{
    C(time, ShovelTime(), {{plant_type}}, {{row}}, col);
}