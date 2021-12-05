#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include "DebugInterface.hpp"
#include "model/Model.hpp"
#include <unordered_set>

class MyStrategy {
public:
    std::unordered_set<int> left_defenders, right_defenders;
    int left_defenders_threshold{1}, right_defenders_threshold{1};

    MyStrategy();
    Action getAction(const PlayerView& playerView, DebugInterface* debugInterface);
    void debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface);
};

#endif
