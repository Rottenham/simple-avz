#pragma once

#include "error.h"
#include "global.h"
#include "libavz.h"
#include "util.h"

namespace _SimpleAvZInternal {

class MyWave {
public:
    int value;
    MyWave(int v)
        : value(v)
    {
    }
};

void init(MyWave& wave)
{
    last_set_time = -1;
    AvZ::SetTime(3000, wave.value); // 如果之后没有SetTime, 就会有报错提醒, 而非静默出错
}

std::string to_string(const std::vector<MyWave> waves)
{
    std::vector<std::string> strings(waves.size());
    for (const auto& w : waves)
        strings.push_back(std::to_string(w.value));
    return concat(strings, ",");
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
}

template <typename... Args>
_SimpleAvZInternal::MyWaves waves(Args... args)
{
    std::vector<_SimpleAvZInternal::MyWave> waves_vec = {_SimpleAvZInternal::MyWave(args)...};
    for (const auto& w : waves_vec) {
        if (w.value <= 0 || w.value > 20) {
            _SimpleAvZInternal::error("waves", "波数应在1~20内\n当前为: waves(" + to_string(waves_vec) + ")");
        }
    }
    return _SimpleAvZInternal::MyWaves(waves_vec);
}

_SimpleAvZInternal::MyWaves waves(const std::array<int, 2>& wave_range, int step)
{
    if (wave_range[0] > wave_range[1]) {
        _SimpleAvZInternal::error("waves", "起始波数应≤终止波数\n起始波数: #\n终止波数: #", wave_range[0], wave_range[1]);
    }
    if (step <= 0) {
        _SimpleAvZInternal::error("waves", "循环长度应>0\n循环长度: #", step);
    }
    std::vector<_SimpleAvZInternal::MyWave> waves_vec;
    for (int w = wave_range[0]; w <= wave_range[1]; w += step)
        waves_vec.push_back(
            _SimpleAvZInternal::MyWave(w));
    return waves_vec;
}
