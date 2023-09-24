#pragma once

#include "libavz.h"

struct Time {
    enum class Type {
        ABS, // 绝对时间
        REL, // 相对时间
    };

    Type type;
    int time;
    bool fix_card_time_to_cob; // 是否将冰核樱辣窝修正至炮等效时机

    Time(int _time)
        : type(Type::ABS)
        , time(_time)
        , fix_card_time_to_cob(true)
    {
    }

    Time(Type _type, int _time, bool _fix_card_time_to_cob)
        : type(_type)
        , time(_time)
        , fix_card_time_to_cob(_fix_card_time_to_cob)
    {
    }
};

Time after(int t)
{
    return {Time::Type::REL, t, true};
}

const auto now = after(0);

Time exact(Time t)
{
    t.fix_card_time_to_cob = false;
    return t;
}

struct ShovelTime {
    enum class Type {
        NONE,  // 不指定
        KEEP,  // 指定铲除延迟
        UNTIL, // 指定铲除时机
    };

    Type type;
    int time;

    ShovelTime()
        : type(Type::NONE)
        , time(-1)
    {
    }

    ShovelTime(Type _type, int _time)
        : type(_type)
        , time(_time)
    {
    }
};

ShovelTime keep(int t)
{
    return ShovelTime(ShovelTime::Type::KEEP, t);
}

ShovelTime until(int t)
{
    return ShovelTime(ShovelTime::Type::UNTIL, t);
}
