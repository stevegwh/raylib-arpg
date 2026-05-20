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
            inspector.UInt64("Id", id, false);
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
            inspector.String("Name", name);
            inspector.String("Category", category);
            inspector.Bool("Selectable", selectable);
            inspector.Bool("Visible In Hierarchy", visibleInHierarchy);
            inspector.Bool("Locked", locked);
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
            inspector.String("Asset Key", assetKey, false);
        }
    };
} // namespace sage::editor
