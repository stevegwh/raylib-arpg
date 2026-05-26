//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "ViewSerializer.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "entt/entt.hpp"
#include "raylib-cereal.hpp"
#include <cereal/archives/json.hpp>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

namespace sage::serializer
{
    // Archiving definitions
    struct entity
    {
        unsigned int id;
    };

    template <typename Archive>
    void serialize(Archive& archive, entity& entity)
    {
        archive(entity.id);
    }

    void LoadAssetBinFile(entt::registry* destination, const char* path);

    template <typename T>
    void SaveClassXML(const char* path, const T& toSave)
    {
        std::cout << "START: Saving class data to XML file." << std::endl;
        using namespace entt::literals;
        // std::stringstream storage;

        std::ofstream storage(path);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for writing." << std::endl;
            exit(1);
        }

        {
            // output finishes flushing its contents when it goes out of scope
            cereal::XMLOutputArchive output{storage};
            output(toSave);
        }

        storage.close();
        std::cout << "FINISH: Saving class data to XML file." << std::endl;
    }

    template <typename T>
    void DeserializeXMLFile(const char* path, T& target)
    {
        std::cout << "START: Loading data from file." << std::endl;
        using namespace entt::literals;

        std::ifstream storage(path);
        if (storage.is_open())
        {
            cereal::XMLInputArchive input{storage};
            input(target);
            storage.close();
        }
        else
        {
            // File doesn't exist, create a new file with the default key mapping
            std::cout << "INFO: File not found. Creating a new file with defaults." << std::endl;
            SaveClassXML<T>(path, target);
        }
        std::cout << "FINISH: Loading data from file." << std::endl;
    }

    template <typename T>
    void SaveClassJson(const std::string& path, const T& toSave)
    {
        std::cout << "START: Saving class data to json file." << std::endl;
        using namespace entt::literals;
        // std::stringstream storage;

        std::ofstream storage(path);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for writing." << std::endl;
            exit(1);
        }

        {
            // output finishes flushing its contents when it goes out of scope
            cereal::JSONOutputArchive output{storage};
            // output.setNextName(name);
            output(toSave);
        }

        storage.close();
        std::cout << "FINISH: Saving class data to json file." << std::endl;
    }

    template <typename T>
    void SaveViewJson(entt::registry& source, const char* path)
    {
        std::cout << "START: Saving view data to json file." << std::endl;
        using namespace entt::literals;
        // std::stringstream storage;

        std::ofstream storage(path);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for writing." << std::endl;
            exit(1);
        }

        {
            // output finishes flushing its contents when it goes out of scope
            cereal::JSONOutputArchive output{storage};
            ViewSerializer<T> allData(&source);
            output(allData);
        }

        storage.close();
        std::cout << "FINISH: Saving view data to json file." << std::endl;
    }

    template <typename T>
    void DeserializeJsonFile(const char* path, T& target)
    {
        std::cout << "START: Loading data from file." << std::endl;
        using namespace entt::literals;

        std::ifstream storage(path);
        if (storage.is_open())
        {
            cereal::JSONInputArchive input{storage};
            input(target);
            storage.close();
        }
        else
        {
            // File doesn't exist, create a new file with the default key mapping
            std::cout << "INFO: File not found. Creating a new file with defaults." << std::endl;
            SaveClassJson<T>(path, target);
        }
        std::cout << "FINISH: Loading data from file." << std::endl;
    }

    // Per-file-type magic prefixes for compressed binaries. Bumped if on-disk layout changes.
    inline constexpr char kAssetBinMagic[4] = {'L', 'Q', 'B', '2'};
    inline constexpr char kMapBinMagic[4] = {'L', 'Q', 'M', '2'};

    // Writes a 20-byte header (magic + uncompressed size + compressed size) followed by a
    // DEFLATE-compressed cereal binary payload. The lambda receives a BinaryOutputArchive and
    // is free to call output(...) any number of times.
    template <typename ArchiveFn>
    void WriteCompressedBinary(const char* path, const char (&magic)[4], ArchiveFn&& archiveFn)
    {
        std::ostringstream buf(std::ios::binary);
        {
            cereal::BinaryOutputArchive output{buf};
            archiveFn(output);
        }
        const std::string raw = buf.str();
        const int rawSize = static_cast<int>(raw.size());

        int compSize = 0;
        unsigned char* compData = CompressData(
            reinterpret_cast<const unsigned char*>(raw.data()), rawSize, &compSize);
        if (compData == nullptr || compSize <= 0)
        {
            std::cerr << "ERROR: CompressData failed; aborting save." << std::endl;
            if (compData) MemFree(compData);
            exit(1);
        }

        std::ofstream storage(path, std::ios::binary);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for writing." << std::endl;
            MemFree(compData);
            exit(1);
        }

        const uint64_t uncompressedSize = static_cast<uint64_t>(rawSize);
        const uint64_t compressedSize = static_cast<uint64_t>(compSize);
        storage.write(magic, sizeof(magic));
        storage.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize));
        storage.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));
        storage.write(reinterpret_cast<const char*>(compData), compSize);

        MemFree(compData);
        storage.close();
        std::cout << "  (raw=" << rawSize << "B compressed=" << compSize
                  << "B ratio=" << (rawSize > 0 ? (static_cast<double>(compSize) / rawSize) : 0.0) << ")"
                  << std::endl;
    }

    // Reads a header-prefixed DEFLATE-compressed cereal payload, then invokes the lambda with
    // both a BinaryInputArchive and the underlying istream (so callers that use peek-until-EOF
    // loops still work).
    template <typename ArchiveFn>
    void ReadCompressedBinary(const char* path, const char (&magic)[4], ArchiveFn&& archiveFn)
    {
        std::ifstream storage(path, std::ios::binary);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for reading." << std::endl;
            exit(1);
        }

        char fileMagic[4]{};
        uint64_t uncompressedSize = 0;
        uint64_t compressedSize = 0;
        storage.read(fileMagic, sizeof(fileMagic));
        storage.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
        storage.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));

        if (std::memcmp(fileMagic, magic, sizeof(magic)) != 0)
        {
            std::cerr << "ERROR: file magic mismatch at " << path << " (got '"
                      << std::string(fileMagic, 4) << "', expected '" << std::string(magic, 4) << "')."
                      << std::endl;
            exit(1);
        }

        std::vector<unsigned char> compBuf(compressedSize);
        storage.read(reinterpret_cast<char*>(compBuf.data()), static_cast<std::streamsize>(compressedSize));
        storage.close();

        int decompSize = 0;
        unsigned char* decompData =
            DecompressData(compBuf.data(), static_cast<int>(compressedSize), &decompSize);
        if (decompData == nullptr || static_cast<uint64_t>(decompSize) != uncompressedSize)
        {
            std::cerr << "ERROR: DecompressData failed (got " << decompSize << ", expected "
                      << uncompressedSize << ")." << std::endl;
            if (decompData) MemFree(decompData);
            exit(1);
        }

        std::string decompStr(reinterpret_cast<const char*>(decompData), uncompressedSize);
        MemFree(decompData);

        std::istringstream inBuf(std::move(decompStr), std::ios::binary);
        {
            cereal::BinaryInputArchive input(inBuf);
            archiveFn(input, inBuf);
        }
    }

    template <typename T>
    void SaveClassBinary(const char* path, const T& toSave)
    {
        std::cout << "START: Saving class data to binary file." << std::endl;
        WriteCompressedBinary(path, kAssetBinMagic, [&](cereal::BinaryOutputArchive& output) {
            output(toSave);
        });
        std::cout << "FINISH: Saving class data to binary file." << std::endl;
    }
} // namespace sage::serializer
