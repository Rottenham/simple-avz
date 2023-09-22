# Simple AvZ 💫

*---------- 一种很新的 AvZ*

Simple AvZ, 即 "AvZ 简化版", 是基于 AvZ 1 设计的一套用于精简脚本的语法框架.

## 使用方式

使用 AvZ VSCode 插件 Get AvZ Extension 下载本插件后, 在脚本中加入 `#include "SimpleAvZ/lib.h"` 即可.

若 Get AvZ Extension 不可用, 可从本仓库 `/release` 目录中手动下载.

## 设计理念

- 简洁 好于 冗长
- 声明 好于 命令
- 安全 好于 效率

下面举一些例子说明上述理念.

## 剔除 SetTime

Simple AvZ 的重要目标之一, 是让脚本中不再出现 `SetTime` 与 `Delay`.

以 [PE]. 最后之作 的减速波为例:
```cpp
// 原生 AvZ 1
for (auto wave : {...}) {
    SetTime(410 - 373, wave);
    pao_operator.pao({{1, 8.75}, {5, 8.75}});
    Delay(220);
    pao_operator.pao({{1, 8.575}, {5, 8.575}});
    SetTime(1100 - 378);
    pao_operator.pao(3, 8.2375);
    SetTime(1887 - 200 - 373);
    pao_operator.pao({{2, 8.5}, {5, 8.5}});
    SetTime(1887 + 10 - 298);
    ice_filler.coffee();
    SetPlantActiveTime(ICE_SHROOM, 298);
}
```

如果你刚写完这段脚本, 并不会觉得有违和感.

可当一段时间后重读, 或是其他人阅读此段时, 大家关心的是**具体操作及时机**, 可满眼望去却都是 `SetTime`, `Delay`, 和反复出现的神秘嘉宾 `-373`.

使用 Simple AvZ, 以上代码简化为:
```cpp
// Simple AvZ 💫
for (auto _ : waves(...)) {
    DD(410, 8.75);
    DD(after(220), 8.575);
    P(1100, 3, 8.8275);
    PP(1887, 8.5);

    I(1887 + 10);
}
```

代码行数减半, 重要信息一览无余, 且十分接近我们熟知的轨道语言, 一看就懂.

我们知道炮的飞行时间, 知道当前场景, 更知道 PE 下多数情况不是炸 2,5 就是炸 1,5 路. 既然如此, 为何不对这些信息加以利用呢? 这是 Simple AvZ 的初衷.

## 以生效时机思考

对手控玩家而言, 掐时机至关重要, 什么时候点冰, 什么时候拖炮都有讲究.

可在键控领域, 我们更关心**生效时机**!

使用 Simple AvZ, 你可以写出这样的代码:

```cpp
// Simple AvZ 💫
PP(318, 8.8);
```

以**声明**我要在 318cs 于 8.8 列生效并炸炮.

至于 PP 炸哪行? 如果有上界之风, 应如何修正发炮时间? 这一切都由 Simple AvZ 自动推测, 编写者无需多虑.

这就是所谓的**声明式编程**, 即以结果为导向,而非以过程为导向.

这不仅让代码更简洁易懂, 增加编写效率, 更避免了一些本不必要的 bug. 例如炮击泳池飞行 378cs, 炮击平地飞行 373cs, 若不小心区分很容易混淆, 可多数情况下我们本不关心这些, 只是想在特定时机让炮生效罢了.


## 基于炮的时间体系

由于游戏的蛋疼设定, 每帧结算顺序为 [植物] --> [僵尸] --> [子弹].

说来也巧, 由于冰与灰烬(樱辣窝核)属于 [植物], 而炮又属于 [子弹], 两者相对于 [僵尸] 的结算顺序便有了 `1cs` 偏差.

> 简单来说, `x cs` 生效的炮 = `x + 1 cs` 生效的卡.

这个不一致性在 AvZ 1 设计伊始就已显现, 如 `AvZ::SetPlantActiveTime()`, 即 Ice3 函数, 其实际效果为将冰生效时机修正为设定时机的 **下一 cs**.

一说这有道理, 因为按僵尸时间, 巨人确实运动了 `105/210cs`, 只是由于冰早于僵尸结算, 故实际要 `+1cs`. 一说这不科学, 其它用卡用炮皆以实际时机书写, 为何冰要特殊处理?

Simple AvZ 对此的解决思路如下.

首先，基于**声明好于命令**的理念, 樱辣窝核这四张卡**以炮时机书写更优**, 因为实际时机不如等效炮时机好用. 如此便有:

```cpp
// Simple AvZ 💫
P(359, 2, 9);
C(359, CHERRY, 5, 9);
DD(after(107), 1, 7.8);
```

樱桃实际在 `360` 生效, 但编写者和读者无需考虑这样的细枝末节, 统一作 `359` 即可, 清晰易懂.

由于樱辣窝核以炮时机书写甚为合理, Simple AvZ 选择将冰也纳入此体系, 以炮为纲, 一统江湖. 这也使得其对旧脚本的兼容性更佳, 且更符合 AvZ 的设计初衷.

## 自动设定波次

原生 AvZ 中, 我们常用 `for (auto wave : {...})` 书写循环, 但循环节内部首次 `SetTime` 时, 必须不能忘记带上 `wave`, 否则 AvZ 很生气, 后果很严重.

既然这种写法如此常用, 为何不自动设定波次呢? 于是就有了:

```cpp
// Simple AvZ 💫
for (auto _ : waves(1, 2, 3)) {
    PP(318);
    ...
}
```

使用 Simple AvZ 提供的 `waves()` 函数, 妈妈再也不担心我忘记设定波次啦.

不仅如此, 你还可以表达循环, 例如 `waves({1, 9}, 4)` 表示从 w1 开始, 到 w9 为止, 4 波一循环, 即在 w1, w5, w9 执行循环体中的内容.


## 更便捷的用卡

原生 AvZ 有以下三种常用的用卡方式:
```cpp
Card(PEA_SHOOTER, 1, 1); // 在1-1种植豌豆射手
Card(PEA_SHOOTER, {{1, 1}, {1, 2}}); // 先尝试在1-1种, 不行则改为1-2
Card({{PEA_SHOOTER, 1, 1}, {SUNFLOWER, 1, 2}}); // 1-1种豌豆, 1-2种小向
```

可在实战中, 我们经常需要:
```cpp
// 垫1, 2, 5, 6路
Card({{PUFF_SHROOM, 1, 9}, {SUN_SHROOM, 2, 9}, {SCAREDY_SHROOM, 5, 9}, {FLOWER_POT, 6, 9}});

// 在水上点核
Card({{LILY_PAD, 3, 9}, {DOOM_SHROOM, 3, 9}, {COFFEE_BEAN, 3, 9}});
```

长久以来, 我们都习惯了这样写. 可既然这两种情况如此常见, 为何不提供更便捷的写法呢?

```cpp
// Simple AvZ 💫
C(400, {PUFF, SUN, SCAREDY, POT}, {1, 2, 5, 6}, 9); // 垫1, 2, 5, 6路
C(359, {LILY, DOOM, COFFEE}, 3, 9); // 在水上点核
```

> 对常用的 13 种植物, Simple AvZ 提供了(合理的)缩写.

有种即有铲. 你可以便捷地传入铲除时机:

```cpp
// Simple AvZ 💫
C(400, keep(268), PUFF, 1, 9); // 400cs种植, 268cs后铲
C(400, until(1038), PUFF, 1, 9); // 400cs种植, 于1038cs铲
```

此外, 你也可以指定延迟 `x cs` 生效卡, 或是直接调用铲除函数等, 详见完整 API.


## 完整 API

### 用炮

```cpp
PP(278);               // 炸(2,9)与(5,9), 278cs生效
PP(after(110));        // 同上, 延迟110cs生效
PP(278, 8);            // 炸(2,8)与(5,8), 278cs生效
PP(278, {8, 9});       // 炸(2,8)与(5,9), 278cs生效
```

发射激活炮. 五行场地炸 2,4 路, 六行场地炸 2,5 路. 若省略列数, 默认炸 9 列.

```cpp
DD(278, 8);            // 炸(1,8)与(5,8), 278cs生效
DD(after(110), 8);     // 同上，延迟110cs生效
DD(278, {8, 9});       // 炸(1,8)与(5,9), 278cs生效
```

发射拦截炮. 五行场地炸 1,4 路, 六行场地炸 2,5 路.

```cpp
P(278, 2, 9);              // 炸(2,9), 278cs生效
P(after(110), 2, 9);       // 同上，延迟110cs生效
P(278, {1, 6}, 9);         // 炸(1,9)与(6,9), 278cs生效
P(278, {{1, 8}, {6, 9}});  // 炸(1,8)与(6,9), 278cs生效
P(278, 1, 1, 2, 9);        // 使用1-1炮炸(2,9), 278cs生效
```

发射任意数量炮. 可提供单个落点, 多行同列, 或多个落点. 也可指明要用哪门炮.

```cpp
excludeCob(400, {3, 5});   // 不使用3-5炮, 400cs生效
resetCob(400);             // 重置为使用所有炮, 400cs生效       
```

不使用特定炮, 或重置为使用所有炮.

```cpp
CobOperator c1(1);         // 只用炮尾在1列的炮
CobOperator c45(4, 5);     // 只用炮尾在4或5列的炮

void Script() {
    c1.PP(...); // 调用方式不变
}
```
在屋顶场合, 你可以声明多个 `CobOperator`, 然后调用以上函数.

### 用卡

> 除特别注明, 冰,核,樱,辣,窝均基于炮等效时间, 见[基于炮的时间体系].

```cpp
I(1, 2);             // 于1-2放置原版冰, 601cs后生效(完美预判冰)
I(after(210), 1, 2); // 同上, 延迟210cs后生效(ice3)
                     // 推荐在激活炮后使用
I(359, 1, 2);        // 同上, 359cs后生效
M_I(...);            // 使用复制冰, 调用方式不变
```

夜间用冰. 自带生效时机修正. 若不指定生效时间, 默认在本波 601cs 后生效.

```cpp
setIce({{1, 1}, {1, 2}});  // 设定在1-1, 1-2存冰(优先使用1-2)
                           // (w1, -599)生效
setIce(400, {...});        // 同上, 400cs生效
```

白昼设置存冰位置. 若不指定生效时间, 默认在 wave 1, -599cs 生效.

```cpp
I();                       // 点冰, 601cs后生效(完美预判冰)
I(after(210));             // 同上, 延迟210cs后生效(ice3)
                           // 推荐在激活炮后使用
I(359);                    // 同上, 359cs后生效
```

白昼点冰. 自带生效时机修正. 若不指定生效时间, 默认在本波 601cs 后生效.

```cpp
C(359, CHERRY, 2, 9);                // 于2-9放置樱桃, 359cs后生效
C(400, {PUFF, SUN}, {1, 2}, 9);      // 于400cs在1-9放置小喷, 在2-9放置阳光菇
C(359, {LILY, DOOM, COFFEE}, 3, 9);  // 于3-9放置荷叶+核武+咖啡豆, 359cs后生效
```

用卡. 可提供单一坐标, 多行同列, 或多卡同坐标.

```cpp
C(after(110), ...);            // 延迟110cs生效
C(exact(800), ...);            // 800cs生效, 不使用炮等效时间
C(after(exact(..)), ...);      // 以上两者的结合
```

用卡生效时机的变种.

```cpp
C(400, keep(266), ...);        // 放置后266cs铲
C(400, until(1036), ...);      // 1036cs铲
```

指定用卡铲除时机.

```cpp
RM(400, SUNFLOWER);            // 于400cs铲除场地上所有小向
RM(400, PUMPKIN, 1, 1);        // 于400cs铲除位于1-1的南瓜（若无, 则跳过）
RM(400, 1, 1);                 // 于400cs铲除位于1-1的植物, 优先铲除非南瓜
RM(400, {1, 2, 5, 6}, 9);      // 于400cs铲除1,2,5,6路9列植物, 优先铲除非南瓜
```

铲除植物. 可提供单一坐标, 多行同列. 也可铲除场地上所有某种植物.

## 脚本示例

待补

## 了解更多

待补
