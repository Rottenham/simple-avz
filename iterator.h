#pragma once

#include "error.h"
#include "global.h"
#include "libavz.h"
#include "util.h"

namespace _SimpleAvZInternal {

class UsedWaves : AvZ::GlobalVar {
public:
    std::vector<int> waves = {};

    void virtual beforeScript() override
    {
        waves = {};
    }

    void virtual afterScript() override
    {
        if (global.validate_waves_integrity) {
            int count[21] = {};

            for (int w : waves) {
                count[w]++;
            }

            std::vector<int> missing_waves;
            for (int i = 1; i <= 20; i++) {
                if (count[i] == 0) {
                    missing_waves.push_back(i);
                }
            }
            if (!missing_waves.empty()) {
                error("waves 完整性检查", "缺少波数: " + concat(ints_to_strings(missing_waves), ","));
            }

            std::vector<int> duplicate_waves;
            for (int i = 1; i <= 20; i++) {
                if (count[i] > 1) {
                    duplicate_waves.push_back(i);
                }
            }
            if (!duplicate_waves.empty()) {
                error("waves 完整性检查", "重复的波数: " + concat(ints_to_strings(duplicate_waves), ","));
            }
        }
    }
};

UsedWaves used;

class MyWave {
public:
    int value;
    MyWave(int v)
        : value(v)
    {
    }
    bool operator==(int int_value) const
    {
        return value == int_value;
    }

    bool operator!=(int int_value) const
    {
        return value != int_value;
    }

    bool operator<(int int_value) const
    {
        return value < int_value;
    }

    bool operator>(int int_value) const
    {
        return value > int_value;
    }

    bool operator<=(int int_value) const
    {
        return value <= int_value;
    }

    bool operator>=(int int_value) const
    {
        return value >= int_value;
    }
};

void init(MyWave& wave)
{
    _SimpleAvZInternal::global.reset_last_set_time();
    AvZ::SetTime(3000, wave.value); // 如果之后没有SetTime, 就会有报错提醒, 而非静默出错
}

class MyWaveIterator : public std::iterator<std::input_iterator_tag, MyWave> {
    std::vector<MyWave>::iterator it;

public:
    MyWaveIterator(std::vector<MyWave>::iterator i)
        : it(i)
    {
    }
    MyWave operator*()
    {
        init(*it);
        return *it;
    }
    MyWaveIterator& operator++()
    {
        ++it;
        return *this;
    }
    bool operator!=(const MyWaveIterator& other)
    {
        return it != other.it;
    }
};

class MyWaves {
    std::vector<MyWave> waves;

public:
    MyWaves(std::vector<MyWave> w)
        : waves(w)
    {
    }
    MyWaveIterator begin()
    {
        return MyWaveIterator(waves.begin());
    }
    MyWaveIterator end()
    {
        return MyWaveIterator(waves.end());
    }
};

std::vector<MyWave> wave_range_to_waves_vec(const std::array<int, 2>& wave_range, int step, const std::string& func_name)
{
    if (wave_range[0] > wave_range[1]) {
        _SimpleAvZInternal::error(func_name, "起始波数应≤终止波数\n起始波数: #\n终止波数: #", wave_range[0], wave_range[1]);
    }
    std::vector<_SimpleAvZInternal::MyWave> waves_vec;
    for (int w = wave_range[0]; w <= wave_range[1]; w += step) {
        if (w < 1 || w > 20) {
            _SimpleAvZInternal::error(func_name, "波数应在1~20内\n当前为: #", w);
        }
        _SimpleAvZInternal::used.waves.push_back(w);
        waves_vec.push_back(
            _SimpleAvZInternal::MyWave(w));
    }
    return waves_vec;
}

}

// 设定波次.
// 基础用法: for (auto w : waves(...)) {
//              // 具体操作
//          }
// *** 使用例:
// waves(1, 2, 3)-----------------w1, w2, w3(可指定任意多个)
// waves({1, 9}, 4)---------------w1-w9 每4波一循环
// waves({1, 9}, {11, 19}, 4)-----w1-w9, w11-w19 每4波一循环
template <typename... Args>
_SimpleAvZInternal::MyWaves waves(Args... args)
{
    std::vector<_SimpleAvZInternal::MyWave> waves_vec = {_SimpleAvZInternal::MyWave(args)...};
    for (const auto& w : waves_vec) {
        if (w.value < 1 || w.value > 20) {
            _SimpleAvZInternal::error("waves", "波数应在1~20内\n波数: #", w.value);
        }
        _SimpleAvZInternal::used.waves.push_back(w.value);
    }
    return _SimpleAvZInternal::MyWaves(waves_vec);
}

_SimpleAvZInternal::MyWaves waves(const std::array<int, 2>& wave_range_1, const std::array<int, 2>& wave_range_2, int step)
{
    if (step <= 0) {
        _SimpleAvZInternal::error("waves", "循环长度应>0\n循环长度: #", step);
    }

    auto waves_vec_1 = _SimpleAvZInternal::wave_range_to_waves_vec(wave_range_1, step, "waves");
    auto waves_vec_2 = _SimpleAvZInternal::wave_range_to_waves_vec(wave_range_2, step, "waves");
    waves_vec_1.insert(waves_vec_1.end(), waves_vec_2.begin(), waves_vec_2.end());
    return _SimpleAvZInternal::MyWaves(waves_vec_1);
}

_SimpleAvZInternal::MyWaves waves(const std::array<int, 2>& wave_range, int step)
{
    if (step <= 0) {
        _SimpleAvZInternal::error("waves", "循环长度应>0\n循环长度: #", step);
    }

    return _SimpleAvZInternal::MyWaves(_SimpleAvZInternal::wave_range_to_waves_vec(wave_range, step, "waves"));
}

// 检查 waves() 设置完整性, 是否调用 w1~w20 每波正好一次.
// 强烈建议写完脚本后使用本函数.
void ValidateWaves()
{
    _SimpleAvZInternal::global.validate_waves_integrity = true;
}