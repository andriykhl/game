#include "MyStrategy.hpp"
#include <exception>
#include <functional>
#include <iostream>
MyStrategy::MyStrategy() {}

Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface)
{
    using namespace std;

    Action result = Action(unordered_map<int, EntityAction, hash<int>>());
    int myId = playerView.myId;
    auto myPlayer = find_if(playerView.players.begin(), playerView.players.end(), [&](auto p){ return p.id==myId; });

    int all_places{};
    int used_places{};
    int builder{};
    int troops{};
    vector<pair<int, Vec2Int>> houses{};
    for(const auto & entity : playerView.entities)
    {
            if(!entity.playerId or *entity.playerId!=myId) continue;
            all_places  += playerView.entityProperties.at(entity.entityType).populationProvide;
            used_places += playerView.entityProperties.at(entity.entityType).populationUse;
            if(entity.entityType == HOUSE and entity.health!=playerView.entityProperties.at(entity.entityType).maxHealth)
                houses.push_back({entity.id,entity.position});
            builder += entity.entityType == BUILDER_UNIT;
            troops  += entity.entityType == RANGED_UNIT;
    }

    for(const auto & entity : playerView.entities)
    {
        if(!entity.playerId or *entity.playerId!=myId) continue;
        const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);

        shared_ptr<MoveAction>   moveAction{nullptr};
        shared_ptr<BuildAction>  buildAction{nullptr};
        shared_ptr<AttackAction> attackAction{nullptr};
        shared_ptr<RepairAction> repareAction{nullptr};

        if(properties.build)
        {
            EntityType entityType = properties.build->options[0];
            if(used_places+1 <= all_places)
            {
                if(entity.entityType==RANGED_BASE or entity.entityType==BUILDER_BASE and builder<2*troops)
                buildAction = make_shared<BuildAction>(entityType, Vec2Int(entity.position.x+properties.size, entity.position.y+properties.size-1));
            }
        }
        vector<EntityType> validAutoAttackTargets;
        if(entity.entityType == BUILDER_UNIT)
        {
            if(used_places+3 >= all_places and myPlayer->resource >= 60)
            {
                cout << "build! \n" << flush;
                buildAction = make_shared<BuildAction>(HOUSE, Vec2Int(entity.position.x+properties.size, entity.position.y+properties.size-1));
                result.entityActions[entity.id] = EntityAction(moveAction, buildAction, attackAction, nullptr);
                continue;
            }
            moveAction = make_shared<MoveAction>(Vec2Int(playerView.mapSize - 1, playerView.mapSize - 1), true, true);

            validAutoAttackTargets.push_back(RESOURCE);
            attackAction = make_shared<AttackAction>(nullptr, make_shared<AutoAttack>(properties.sightRange, validAutoAttackTargets));
            for(const auto & i : houses)
                if(abs(i.second.x-entity.position.x)+abs(i.second.y-entity.position.y)<=5)
                {
                    repareAction = make_shared<RepairAction>(i.first);
                    attackAction=nullptr;
                }
        }

        if(entity.entityType == RANGED_UNIT or entity.entityType == MELEE_UNIT)
        {
            if(troops<10)
                moveAction = make_shared<MoveAction>(Vec2Int(playerView.mapSize/3-1, playerView.mapSize/3-1), true, true);
            else
                moveAction = make_shared<MoveAction>(Vec2Int(playerView.mapSize-1, playerView.mapSize-1), true, true);
            attackAction = make_shared<AttackAction>(nullptr, make_shared<AutoAttack>(properties.sightRange, validAutoAttackTargets));
        }
        result.entityActions[entity.id] = EntityAction(moveAction, buildAction, attackAction, repareAction);
    }
    return result;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface)
{
    debugInterface.send(DebugCommand::Clear());
    debugInterface.getState();
}
