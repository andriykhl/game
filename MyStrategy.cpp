#include "MyStrategy.hpp"
#include <exception>
#include <functional>
MyStrategy::MyStrategy() {}

class EntitiesIterator
{
public:
    EntitiesIterator(const std::vector<Entity>& entities /*,
                     std::function<bool(const Entity&)> validationCondition = [](const Entity&)
                                                                      {
                                                                          return true;
                                                                      }*/) :
    _entities(entities)/*,
    _validationCondition(validationCondition)*/
    {
    };

    bool isValid()
    {
        return _position < _entities.size();
    }

    bool isEnd()
    {
        return _position == _entities.size();
    }

    const Entity& get()
    {
        return _entities[_position];
    }

    void next()
    {
      // while (isValid() || !validationCondition(get()))
      // {
        ++_position;
      //}
    }

private:
    /*std::function<bool(const Entity&)> _validationCondition;*/

    const std::vector<Entity>& _entities;
    size_t _position;
};


size_t countUnits(const int playerId,
                  const std::vector<Entity>& entities,
                  const EntityType& entityType)
{
    size_t currentUnits = 0;

    for (EntitiesIterator iter = EntitiesIterator(entities); iter.isValid(); iter.next())
    {
        const Entity& entity = iter.get();

        // Condition
        if (entity.playerId != nullptr &&
            *entity.playerId == playerId &&
            entity.entityType == entityType)
        {
        //
            currentUnits++;
        }
    }

    return currentUnits;
}

MoveAction createMoveAction(const int mapSize)
{
  // Moves an entity to the defauil enemy base cornor.
    return MoveAction(Vec2Int(mapSize - 1, mapSize - 1), true, true);
}

BuildAction createBuildAction(const EntityType& entityType,
                              const Vec2Int& position,
                              const int buildingSize)
{

    // Biulds an entity (looks like Units building)
    return BuildAction(entityType,
                       Vec2Int(position.x + buildingSize, position.y + buildingSize - 1));
}

AttackAction createAttackAction(const EntityType& entityType, int sightRange)
{
    std::vector<EntityType> validAutoAttackTargets;
    if (entityType == BUILDER_UNIT)
    {
        validAutoAttackTargets.push_back(RESOURCE);
    }

    return AttackAction(nullptr,
                        std::make_shared<AutoAttack>(sightRange, validAutoAttackTargets));
}

Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface)
{
    Action result = Action(std::unordered_map<int, EntityAction, std::hash<int> >());
    int myId = playerView.myId;

    for (EntitiesIterator iter = EntitiesIterator(playerView.entities); iter.isValid(); iter.next())
    {
        const Entity& entity = iter.get();

        // Condition
        if (entity.playerId == nullptr || *entity.playerId != myId)
        {
        //
            continue;
        }

        const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);

        std::shared_ptr<MoveAction> moveAction = nullptr;
        std::shared_ptr<BuildAction> buildAction = nullptr;
        std::shared_ptr<AttackAction> attackAction =
                      std::make_shared<AttackAction>(createAttackAction(entity.entityType,
                                                                        properties.sightRange));

        if (properties.canMove)
        {
            moveAction = std::make_shared<MoveAction>(createMoveAction(playerView.mapSize));
        }
        else if (properties.build != nullptr)
        {
            EntityType entityType = properties.build->options[0];
            size_t currentUnits = countUnits(myId, playerView.entities, entityType);

            // TODO: What is it?
            int SOME_CONDITION = (currentUnits + 1) * playerView.entityProperties.at(entityType).populationUse;
            if (SOME_CONDITION <= properties.populationProvide)
            {
                buildAction = std::make_shared<BuildAction>(createBuildAction(
                    entityType, entity.position, properties.size));
            }
        }

        result.entityActions[entity.id] = EntityAction(moveAction,
                                                       buildAction,
                                                       attackAction,
                                                       nullptr);
    }

    return result;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface)
{
    debugInterface.send(DebugCommand::Clear());
    debugInterface.getState();
}
