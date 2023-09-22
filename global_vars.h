#pragma once

#include "error.h"

namespace _SimpleAvZInternal {

int last_ash_effect_time = -1;

int get_last_ash_effect_time()
{
    if (last_ash_effect_time < 0) {
        error("没有延迟的基准, 请先使用非after发炮函数");
    }
    return last_ash_effect_time;
}

} // namespace _SimpleAvZInternal
