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
    unordered_set<int> new_left_defenders, new_right_defenders;
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
            if(entity.entityType == RANGED_UNIT and left_defenders.count(entity.id))
            {
                new_left_defenders.insert(entity.id);
            }
            if(entity.entityType == RANGED_UNIT and right_defenders.count(entity.id))
                new_right_defenders.insert(entity.id);
    }

    left_defenders =  new_left_defenders;
    right_defenders = new_right_defenders;

    for(const auto & entity : playerView.entities)
    {
        if(!entity.playerId or *entity.playerId!=myId) continue;
        if(/*troops>4 and */entity.entityType == RANGED_UNIT and left_defenders.size()<left_defenders_threshold)
        {
            cout << "INSERT LEFT = "  << entity.id <<"\n" << flush;
            left_defenders.insert(entity.id);
        }
        else if(/*troops>4 and */entity.entityType == RANGED_UNIT and right_defenders.size()<right_defenders_threshold
                and !left_defenders.count(entity.id))
        {
            cout << "INSERT RIGHT = "  << entity.id <<"\n" << flush;
            right_defenders.insert(entity.id);
        }
    }

     cout << "SIZE LEFT = "  << left_defenders.size() <<"\n" << flush;
     cout << "SIZE RIGHT = " << right_defenders.size() <<"\n" << flush;

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
            if(left_defenders.size())
                cout << "ID = " << entity.id << " LEFT ID = " << *left_defenders.begin() << "\n" << flush;
            if(right_defenders.size())
                cout << "ID = " << " RIGHT ID = " << *right_defenders.begin() << "\n" << flush;
            //cout << "ID = " << entity.id << " LEFT ID = " << *left_defenders.begin() << " RIGHT ID = " << *right_defenders.begin() << "\n" << flush;
            if(left_defenders.count(entity.id))
            {
                cout << "LEFT DEFENDER ! \n" << flush;
                moveAction = make_shared<MoveAction>(Vec2Int(20, 10), true, true);
            }
            else if(right_defenders.count(entity.id))
            {
                cout << "RIGHT DEFENDER ! \n" << flush;
                moveAction = make_shared<MoveAction>(Vec2Int(10, 20), true, true);
            }
            else if(troops<10)
            {
                cout << "CENTRAL DEFENDER ! \n" << flush;
                moveAction = make_shared<MoveAction>(Vec2Int(playerView.mapSize/3-1, playerView.mapSize/3-1), true, true);
            }
            else
            {
                cout << "OFFENSE ! \n" << flush;
                moveAction = make_shared<MoveAction>(Vec2Int(playerView.mapSize-1, playerView.mapSize-1), true, true);
            }
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
