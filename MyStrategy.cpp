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
    //analysis
    for(const auto & entity : playerView.entities)
    {
            if(entity.playerId==nullptr or *entity.playerId!=myId) continue;
            all_places  += playerView.entityProperties.at(entity.entityType).populationProvide;
            used_places += playerView.entityProperties.at(entity.entityType).populationUse;
    }

    for(const auto & entity : playerView.entities)
    {
        if(entity.playerId==nullptr or *entity.playerId!=myId) continue;
   // cout << myPlayer->resource << "\n" << flush;

        cout << used_places << " / " << all_places << "\n" << flush;
        const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);

        shared_ptr<MoveAction>   moveAction{nullptr};
        shared_ptr<BuildAction>  buildAction{nullptr};
        shared_ptr<AttackAction> attackAction{nullptr};

        if(properties.canMove)
            moveAction = make_shared<MoveAction>(Vec2Int(playerView.mapSize - 1, 0), true, true);
        else if(properties.build != nullptr)
        {
            EntityType entityType = properties.build->options[0];
            size_t currentUnits = count_if(begin(playerView.entities), end(playerView.entities), [&](auto & e)
            {
                return e.playerId!=nullptr and *e.playerId==myId and e.entityType==entityType;
            });

            if(used_places+1 <= all_places)
                buildAction = make_shared<BuildAction>(entityType, Vec2Int(entity.position.x + properties.size, entity.position.y + properties.size - 1));
        }
        vector<EntityType> validAutoAttackTargets;
        if(entity.entityType == BUILDER_UNIT)
        {
cout << myPlayer->resource << " \n" << flush;
            if(used_places+3 >= all_places and myPlayer->resource >= 30)
            {
                cout << "build! \n" << flush;
                buildAction = make_shared<BuildAction>(HOUSE, Vec2Int(0, 0));
                result.entityActions[entity.id] = EntityAction(moveAction, buildAction, attackAction, nullptr);
                continue;
            }

                validAutoAttackTargets.push_back(RESOURCE);
        }
        attackAction = make_shared<AttackAction>(nullptr, make_shared<AutoAttack>(properties.sightRange, validAutoAttackTargets));
        result.entityActions[entity.id] = EntityAction(moveAction, buildAction, attackAction, nullptr);
    }
    return result;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface)
{
    debugInterface.send(DebugCommand::Clear());
    debugInterface.getState();
}
