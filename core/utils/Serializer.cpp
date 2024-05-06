//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include <fstream>
#include <type_traits>
#include <vector>
#include "cereal/cereal.hpp"
//#include <cereal/archives/json.hpp>
#include "cereal/archives/xml.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "entt/entity/snapshot.hpp"

#include "components/Transform.hpp"
#include "components/Renderable.hpp"
#include "components/Collideable.hpp"



namespace sage::serializer
{

struct entity
{
    unsigned int id;
};

template<typename Archive>
void serialize(Archive &archive, entity &entity)
{
    archive(entity.id);
}

void Save(const entt::registry& source)
{
    std::cout << "Save called" << std::endl;
    using namespace entt::literals;
    //std::stringstream storage;

    std::ofstream storage("resources/output.xml");
    if (!storage.is_open()) {
        // Handle file opening error
        return;
    }

    {
        // output finishes flushing its contents when it goes out of scope
        cereal::XMLOutputArchive output{storage};
        const auto view = source.view<sage::Transform, sage::Renderable, sage::Collideable>();
        for (const auto& ent: view) 
        {
            const auto& trans = view.get<sage::Transform>(ent);
            const auto& rend = view.get<sage::Renderable>(ent);
            const auto& col = view.get<sage::Collideable>(ent);
            entity entity{};
            entity.id = entt::entt_traits<entt::entity>::to_entity(ent);
            output.setNextName("entity");
            output(entity);
            output.setNextName("transform");
            output(trans);
            output.setNextName("collideable");
            output(col);
            output.setNextName("renderable");
            output(rend);
        }
    }
    storage.close();

}

void Load(entt::registry* destination)
{
    std::cout << "Load called" << std::endl;
    using namespace entt::literals;
    std::ifstream storage("resources/output.xml");
    if (!storage.is_open()) 
    {
        // Handle file opening error
        return;
    }

    {
        //cereal::JSONInputArchive input{storage};
        cereal::XMLInputArchive input{storage};


        entt::entity currentEntity{};
        while (input.getNodeName() != nullptr)
        {

            std::string componentName = input.getNodeName();

            //input.startNode();

            if (componentName == "entity")
            {
                // TODO: this is currently pointless, but I don't know how to
                // advance cereal's parser without calling input with an object.
                entity id;
                input(id);
                currentEntity = destination->create(); 
            }
            else if (componentName == "transform") 
            {
                auto& transform = destination->emplace<Transform>(currentEntity);
                input(transform);
                
            }
            else if (componentName == "collideable")
            {
                auto& col = destination->emplace<Collideable>(currentEntity);
                input(col);
            }
            else if (componentName == "renderable")
            {
                auto& rend = destination->emplace<Renderable>(currentEntity);
                input(rend);
            }
        }
    }
    storage.close();
}
}