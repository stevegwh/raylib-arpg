#pragma once

#include <cstdint>
#include <string>

namespace sage::editor
{
    struct PersistentEntityId
    {
        std::uint64_t id = 0;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(id);
        }

        template <class Inspector>
        void inspect(Inspector& inspector)
        {
            inspector.readOnlyField("Id", id);
        }
    };

    struct EditorObjectDescriptor
    {
        std::string name;
        std::string category;
        bool selectable = true;
        bool visibleInHierarchy = true;
        bool locked = false;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(name, category, selectable, visibleInHierarchy, locked);
        }

        template <class Inspector>
        void inspect(Inspector& inspector)
        {
            inspector.field("Name", name);
            inspector.field("Category", category);
            inspector.field("Selectable", selectable);
            inspector.field("Visible In Hierarchy", visibleInHierarchy);
            inspector.field("Locked", locked);
        }
    };

    struct AssetReference
    {
        std::string assetKey;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(assetKey);
        }

        template <class Inspector>
        void inspect(Inspector& inspector)
        {
            inspector.readOnlyField("Asset Key", assetKey);
        }
    };
} // namespace sage::editor
