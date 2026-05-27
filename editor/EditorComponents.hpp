#pragma once

#include <cstdint>
#include <string>

namespace sage::editor
{
    struct EditorMapEntity
    {
    };

    struct EditorMapBase
    {
    };

    struct PersistentEntityId
    {
        std::uint64_t id = 0;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(id);
        }

        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Id", id, false);
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
        void define_editor_fields(Inspector& i)
        {
            i.field("Name", name);
            i.field("Category", category);
            i.field("Selectable", selectable);
            i.field("Visible In Hierarchy", visibleInHierarchy);
            i.field("Locked", locked);
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
        void define_editor_fields(Inspector& i)
        {
            i.field("Asset Key", assetKey, false);
        }
    };
} // namespace sage::editor
