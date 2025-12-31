#pragma once
/**
 * This is a heavily modified version of the 'libpartikel' project modified for 3D and ported to C++.
 * The original notice is included below.
 */
/**********************************************************************************************
 *
 *   libpartikel v0.0.3 ALPHA
 *   [https://github.com/dbriemann/libpartikel]
 *
 *
 *   A simple particle system built with and for raylib, to be used as header
 *only library.
 *
 *
 *   FEATURES:
 *       - Supports all platforms that raylib supports
 *
 *   DEPENDENCIES:
 *       raylib >= v2.5.0 and all of its dependencies
 *
 *   CONFIGURATION:
 *   #define LIBPARTIKEL_IMPLEMENTATION
 *       Generates the implementation of the library into the included file.
 *       If not defined, the library is in header only mode and can be included
 *in other headers or source files without problems. But only ONE file should
 *hold the implementation.
 *
 *   LICENSE: zlib/libpng
 *
 *   libpartikel is licensed under an unmodified zlib/libpng license, which is
 *an OSI-certified, BSD-like license that allows static linking with closed
 *source software:
 *
 *   Copyright (c) 2017 David Linus Briemann (@Raging_Dave)
 *
 *   This software is provided "as-is", without any express or implied warranty.
 *In no event will the authors be held liable for any damages arising from the
 *use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose,
 *including commercial applications, and to alter it and redistribute it freely,
 *subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *claim that you wrote the original software. If you use this software in a
 *product, an acknowledgment in the product documentation would be appreciated
 *but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not
 *be misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *distribution.
 *
 **********************************************************************************************/

#include "raylib.h"

#include <cmath>
#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

namespace sage
{
    // Needed forward declarations.
    //----------------------------------------------------------------------------------
    struct Particle;
    struct EmitterConfig;
    struct Emitter;
    struct ParticleSystem;

    bool Particle_DeactivatorAge(Particle* p);

    // Min/Max pair structs for various types.
    struct FloatRange
    {
        float min;
        float max;
    };

    struct IntRange
    {
        int min;
        int max;
    };

    // EmitterConfig type.
    //----------------------------------------------------------------------------------
    struct EmitterConfig
    {
        float size = 1.0f;   // Size of the particle.
        Vector3 direction;   // Direction vector will be normalized.
        FloatRange velocity; // The possible range of the particle velocities.
        // Velocity is a scalar defining the length of the direction vector.
        FloatRange directionAngle;     // The angle range modifying the direction vector.
        FloatRange velocityAngle;      // The angle range to rotate the velocity vector.
        FloatRange offset;             // The min and max offset multiplier for the particle origin.
        FloatRange originAcceleration; // An acceleration towards or from (centrifugal) the origin.
        IntRange burst;                // The range of sudden particle bursts.
        size_t capacity;               // Maximum amounts of particles in the system.
        size_t emissionRate;           // Rate of emitted particles per second.
        Vector3 origin;                // Origin is the source of the emitter.
        Vector3 externalAcceleration;  // External constant acceleration. e.g. gravity.
        Color startColor;              // The color the particle starts with when it spawns.
        Color endColor;                // The color the particle ends with when it disappears.
        FloatRange age;                // Age range of particles in seconds.
        BlendMode blendMode;           // Color blending mode for all particles of this Emitter.
        Texture2D texture;             // The texture used as particle texture.

        std::function<bool(Particle*)> particle_Deactivator; // Pointer to a function that determines when
                                                             // a particle is deactivated.
    };

    // Particle type.
    //----------------------------------------------------------------------------------

    // Particle describes one particle in a particle system.
    struct Particle
    {
        float size;
        Vector3 origin;               // The origin of the particle
        Vector3 position;             // Position of the particle in 2d space.
        Vector3 velocity;             // Velocity vector in 2d space.
        Vector3 externalAcceleration; // Acceleration vector in 2d space.
        float originAcceleration;     // Accelerates velocity vector
        float age;                    // Age is measured in seconds.
        float ttl;                    // Ttl is the time to live in seconds.
        bool active;                  // Inactive particles are neither updated nor drawn.

        std::function<bool(Particle*)> particle_Deactivator; // Pointer to a function that determines
        // when a particle is deactivated.

        explicit Particle(const std::function<bool(Particle*)>& deactivatorFunc);
        void Init(const EmitterConfig& cfg);
        void Update(float dt);
    };

    // Emitter type.
    //----------------------------------------------------------------------------------

    // Emitter is a single (point) source emitting many particles.
    struct Emitter
    {
        EmitterConfig config;
        float mustEmit;   // Amount of particles to be emitted within next update call.
        Vector2 offset{}; // Offset holds half the width and height of the texture.
        bool isEmitting;
        std::vector<std::unique_ptr<Particle>> particles; // Array of all particles (by pointer).

        explicit Emitter(EmitterConfig cfg);
        bool Reinit(const EmitterConfig& cfg);
        void Start();
        void Stop();
        void Burst();
        void Update(float dt);
        void Draw(Camera3D* camera) const;
        void Draw(Camera3D* camera, const Shader& shader) const;
        void DrawNearestFirst(Camera3D* camera) const;
        void DrawNearestFirst(Camera3D* camera, const Shader& shader) const;
        void DrawOldestFirst(Camera3D* camera) const;
        void DrawOldestFirst(Camera3D* camera, const Shader& shader) const;
    };

    // ParticleSystem type.
    //----------------------------------------------------------------------------------

    // ParticleSystem is a set of emitters grouped logically
    // together to achieve a specific visual effect.
    // While Emitters can be used independently, ParticleSystem
    // offers some convenience for handling many Emitters at once.
    class ParticleSystem
    {
      public:
        Camera3D* const camera;
        bool active;
        size_t length;
        size_t capacity;
        Vector3 origin;
        std::vector<std::unique_ptr<Emitter>> emitters;

        explicit ParticleSystem(Camera3D* _camera);
        void Update(float dt);
        bool Register(std::unique_ptr<Emitter> emitter);
        bool Deregister(Emitter* emitter);
        void SetOrigin(Vector3 _origin);
        void SetDirection(Vector3 _direction);
        void Start();
        void Stop();
        void Burst();
        void Draw(const Shader& shader) const;
        void Draw() const;
        void DrawNearestFirst() const;
        void DrawNearestFirst(const Shader& shader) const;
        void DrawOldestFirst() const;
        void DrawOldestFirst(const Shader& shader) const;
    };
} // namespace sage