#include "ParticleSystem.hpp"
#include "raymath.h"
#include <algorithm>
#include <utility>

namespace sage
{
    // Utility functions & structs.
    //----------------------------------------------------------------------------------
    // GetRandomFloat returns a random float between 0.0 and 1.0.
    float GetRandomFloat(float min, float max)
    {
        float range = max - min;
        float n = (float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX;
        return n * range + min;
    }

    // Assuming these utility functions are available or need to be implemented for Vector3
    Vector3 RotateV3(const Vector3& vec, float angleX, float angleY, float angleZ)
    {
        // Implement or use existing functions to rotate a Vector3 around the X, Y, and Z axes
        Matrix rotationMatrix = MatrixRotateXYZ(Vector3{angleX, angleY, angleZ});
        return Vector3Transform(vec, rotationMatrix);
    }

    // LinearFade fades from Color c1 to Color c2. Fraction is a value between 0 and 1.
    // The interpolation is linear.
    Color LinearFade(Color c1, Color c2, float fraction)
    {
        auto newr = (unsigned char)((float)((int)c2.r - (int)c1.r) * fraction + (float)c1.r);
        auto newg = (unsigned char)((float)((int)c2.g - (int)c1.g) * fraction + (float)c1.g);
        auto newb = (unsigned char)((float)((int)c2.b - (int)c1.b) * fraction + (float)c1.b);
        auto newa = (unsigned char)((float)((int)c2.a - (int)c1.a) * fraction + (float)c1.a);

        Color c = {newr, newg, newb, newa};

        return c;
    }

    bool Particle_DeactivatorAge(Particle* p)
    {
        return p->age > p->ttl;
    }

    // Particle constructor
    Particle::Particle(const std::function<bool(Particle*)>& deactivatorFunc)
        : origin({0, 0, 0}),
          position({0, 0, 0}),
          velocity({0, 0, 0}),
          externalAcceleration({0, 0, 0}),
          originAcceleration(0),
          age(0),
          ttl(0),
          active(false),
          particle_Deactivator(deactivatorFunc ? deactivatorFunc : Particle_DeactivatorAge)
    {
    }

    void Particle::Init(const EmitterConfig& cfg)
    {
        age = 0;
        origin = cfg.origin;

        // Base direction (normalized)
        Vector3 direction = Vector3Normalize(cfg.direction);

        // Get a small random angle to find a random velocity direction.
        float randaX = GetRandomFloat(cfg.directionAngle.min, cfg.directionAngle.max) * DEG2RAD;
        float randaY = GetRandomFloat(cfg.directionAngle.min, cfg.directionAngle.max) * DEG2RAD;
        float randaZ = GetRandomFloat(cfg.directionAngle.min, cfg.directionAngle.max) * DEG2RAD;

        // Rotate base direction with the given angles.
        direction = RotateV3(direction, randaX, randaY, randaZ);

        // Get a random value for velocity range (direction is normalized).
        float randv = GetRandomFloat(cfg.velocity.min, cfg.velocity.max);

        // Multiply direction with factor to set actual velocity in the Particle.
        velocity = Vector3Scale(direction, randv);

        // Get a small random angle to rotate the velocity vector.
        randaX = GetRandomFloat(cfg.velocityAngle.min, cfg.velocityAngle.max) * DEG2RAD;
        randaY = GetRandomFloat(cfg.velocityAngle.min, cfg.velocityAngle.max) * DEG2RAD;
        randaZ = GetRandomFloat(cfg.velocityAngle.min, cfg.velocityAngle.max) * DEG2RAD;

        // Rotate velocity vector with given angles.
        velocity = RotateV3(velocity, randaX, randaY, randaZ);

        // Get a smaller random value for origin offset and apply it to position.
        float rando = GetRandomFloat(cfg.offset.min, cfg.offset.max) * 0.1f;
        position.x = cfg.origin.x + direction.x * rando;
        position.y = cfg.origin.y + direction.y * rando;
        position.z = cfg.origin.z + direction.z * rando;

        // Get a random value for the intrinsic particle acceleration
        float rands = GetRandomFloat(cfg.originAcceleration.min, cfg.originAcceleration.max);
        originAcceleration = rands;
        externalAcceleration = cfg.externalAcceleration;
        ttl = GetRandomFloat(cfg.age.min, cfg.age.max);
        active = true;
        size = cfg.size;
    }

    void Particle::Update(float dt)
    {
        if (!active)
        {
            return;
        }

        age += dt;

        if (particle_Deactivator(this))
        {
            active = false;
            return;
        }

        Vector3 toOrigin = Vector3Normalize(Vector3Subtract(origin, position));

        // Update velocity by internal acceleration.
        velocity = Vector3Add(velocity, Vector3Scale(toOrigin, originAcceleration * dt));

        // Update velocity by external acceleration.
        velocity = Vector3Add(velocity, Vector3Scale(externalAcceleration, dt));

        // Apply centripetal force to create a swirl effect
        //	Vector3 centripetalForce = { -toOrigin.z, 0, toOrigin.x };
        //	velocity = Vector3Add(velocity, Vector3Scale(centripetalForce, originAcceleration * dt));

        // Update position by velocity.
        position = Vector3Add(position, Vector3Scale(velocity, dt));
    }

    // Emitter constructor
    Emitter::Emitter(EmitterConfig cfg) : config(std::move(cfg)), mustEmit(0), isEmitting(false)
    {
        offset.x = config.texture.width / 2;
        offset.y = config.texture.height / 2;
        particles.reserve(config.capacity);
        for (size_t i = 0; i < config.capacity; i++)
        {
            particles.push_back(std::make_unique<Particle>(config.particle_Deactivator));
        }
    }

    // Emitter_Reinit reinits the given Emitter with a new EmitterConfig.
    bool Emitter::Reinit(const EmitterConfig& cfg)
    {
        if (cfg.capacity > config.capacity)
        {
            particles.reserve(cfg.capacity);
            for (size_t i = config.capacity; i < cfg.capacity; i++)
            {
                particles.push_back(std::make_unique<Particle>(cfg.particle_Deactivator));
            }
        }
        else if (cfg.capacity < config.capacity)
        {
            particles.resize(cfg.capacity);
        }

        config = cfg;

        for (size_t i = 0; i < config.capacity; i++)
        {
            particles[i]->particle_Deactivator = config.particle_Deactivator;
        }

        return true;
    }

    // Emitter_Start activates Particle emission.
    void Emitter::Start()
    {
        isEmitting = true;
    }

    // Emitter_Stop deactivates Particle emission.
    void Emitter::Stop()
    {
        isEmitting = false;
    }

    // Emitter_Burst emits a specified amount of particles at once,
    // ignoring the state of e->isEmitting. Use this for singular events
    // instead of continuous output.
    void Emitter::Burst()
    {
        size_t emitted = 0;
        int amount = GetRandomValue(config.burst.min, config.burst.max);

        for (size_t i = 0; i < config.capacity; i++)
        {
            auto& p = particles[i];
            if (!p->active)
            {
                p->Init(config);
                p->position = config.origin;
                emitted++;
            }
            if (emitted >= amount)
            {
                return;
            }
        }
    }

    // Emitter_Update updates all particles and returns
    // the current amount of active particles.
    void Emitter::Update(float dt)
    {
        size_t emitNow = 0;

        if (isEmitting)
        {
            mustEmit += dt * (float)config.emissionRate;
            emitNow = (size_t)mustEmit; // floor
        }

        for (size_t i = 0; i < config.capacity; i++)
        {
            auto& p = particles[i];
            if (p->active)
            {
                p->Update(dt);
            }
            else if (isEmitting && emitNow > 0)
            {
                // emit new particles here
                p->Init(config);
                p->Update(dt);
                emitNow--;
                mustEmit--;
            }
        }
    }

    void Emitter::DrawNearestFirst(Camera3D* const camera) const
    {
        std::vector<Particle*> activeParticles;
        for (size_t i = 0; i < config.capacity; i++)
        {
            if (particles[i]->active)
            {
                activeParticles.push_back(particles[i].get());
            }
        }

        std::sort(activeParticles.begin(), activeParticles.end(), [&camera](const Particle* a, const Particle* b) {
            return Vector3Distance(a->position, camera->position) < Vector3Distance(b->position, camera->position);
        });

        BeginBlendMode(config.blendMode);
        for (const auto& p : activeParticles)
        {
            DrawBillboard(
                *camera,
                config.texture,
                p->position,
                p->size,
                LinearFade(config.startColor, config.endColor, p->age / p->ttl));
        }
        EndBlendMode();
    }

    void Emitter::DrawNearestFirst(Camera3D* const camera, const Shader& shader) const
    {
        BeginShaderMode(shader);
        DrawNearestFirst(camera);
        EndShaderMode();
    }

    void Emitter::DrawOldestFirst(Camera3D* const camera) const
    {
        std::vector<Particle*> activeParticles;
        for (size_t i = 0; i < config.capacity; i++)
        {
            if (particles[i]->active)
            {
                activeParticles.push_back(particles[i].get());
            }
        }

        std::sort(activeParticles.begin(), activeParticles.end(), [](const Particle* a, const Particle* b) {
            return a->age < b->age;
        });

        BeginBlendMode(config.blendMode);
        for (const auto& p : activeParticles)
        {
            DrawBillboard(
                *camera,
                config.texture,
                p->position,
                p->size,
                LinearFade(config.startColor, config.endColor, p->age / p->ttl));
        }
        EndBlendMode();
    }

    void Emitter::DrawOldestFirst(Camera3D* const camera, const Shader& shader) const
    {
        BeginShaderMode(shader);
        DrawOldestFirst(camera);
        EndShaderMode();
    }

    // Emitter_Draw draws all active particles.
    void Emitter::Draw(Camera3D* const camera) const
    {
        BeginBlendMode(config.blendMode);
        for (const auto& p : particles)
        {
            DrawBillboard(
                *camera,
                config.texture,
                p->position,
                p->size,
                LinearFade(config.startColor, config.endColor, p->age / p->ttl));
        }
        EndBlendMode();
    }

    void Emitter::Draw(Camera3D* const camera, const Shader& shader) const
    {
        BeginShaderMode(shader);
        Draw(camera);
        EndShaderMode();
    }

    // ParticleSystem constructor
    ParticleSystem::ParticleSystem(Camera* _camera)
        : camera(_camera), active(false), length(0), capacity(1), origin(Vector3{0, 0, 0})
    {
        emitters.reserve(capacity);
    }

    // ParticleSystem_Update runs Emitter_Update on all registered Emitters.
    void ParticleSystem::Update(float dt)
    {
        for (size_t i = 0; i < length; i++)
        {
            emitters[i]->Update(dt);
        }
    }

    // ParticleSystem_Register registers an emitter to the system.
    // The emitter will be controlled by all particle system functions.
    // Returns true on success and false otherwise.
    bool ParticleSystem::Register(std::unique_ptr<Emitter> emitter)
    {
        // If there is no space for another emitter we have to realloc.
        if (length >= capacity)
        {
            // Double capacity.
            emitters.reserve(capacity * 2);
            capacity *= 2;
        }

        // Now the new Emitter can be registered.
        emitters.push_back(std::move(emitter));
        length++;

        return true;
    }

    // ParticleSystem_Deregister deregisters an Emitter by its pointer.
    // Returns true on success and false otherwise.
    bool ParticleSystem::Deregister(Emitter* emitter)
    {
        for (size_t i = 0; i < length; i++)
        {
            if (emitters[i].get() == emitter)
            {
                // Remove this emitter by replacing its pointer with the
                // last pointer, if it is not the only Emitter.
                std::swap(emitters[i], emitters.back());
                emitters.pop_back();
                length--;
                return true;
            }
        }
        // Emitter not found.
        return false;
    }

    // ParticleSystem_SetOrigin sets the origin for all registered Emitters.
    void ParticleSystem::SetOrigin(Vector3 _origin)
    {
        origin = _origin;
        for (auto& emitter : emitters)
        {
            emitter->config.origin = _origin;
        }
    }

    void ParticleSystem::SetDirection(Vector3 _direction)
    {
        for (auto& emitter : emitters)
        {
            emitter->config.direction = _direction;
        }
    }

    // ParticleSystem_Start runs Emitter_Start on all registered Emitters.
    void ParticleSystem::Start()
    {
        for (auto& emitter : emitters)
        {
            emitter->Start();
        }
    }

    // ParticleSystem_Stop runs Emitter_Stop on all registered Emitters.
    void ParticleSystem::Stop()
    {
        for (auto& emitter : emitters)
        {
            emitter->Stop();
        }
    }

    // ParticleSystem_Burst runs Emitter_Burst on all registered Emitters.
    void ParticleSystem::Burst()
    {
        for (auto& emitter : emitters)
        {
            emitter->Burst();
        }
    }

    // ParticleSystem_Draw runs Emitter_Draw on all registered Emitters.
    void ParticleSystem::Draw() const
    {
        std::vector<Emitter*> activeEmitters;
        for (const auto& emitter : emitters)
        {
            if (emitter->isEmitting)
            {
                activeEmitters.push_back(emitter.get());
            }
        }

        std::sort(activeEmitters.begin(), activeEmitters.end(), [this](const Emitter* a, const Emitter* b) {
            return Vector3Distance(a->config.origin, camera->position) <
                   Vector3Distance(b->config.origin, camera->position);
        });

        {
            for (auto& emitter : activeEmitters)
            {
                emitter->Draw(camera);
            }
        }
    }
    void ParticleSystem::Draw(const Shader& shader) const
    {
        std::vector<Emitter*> activeEmitters;
        for (const auto& emitter : emitters)
        {
            if (emitter->isEmitting)
            {
                activeEmitters.push_back(emitter.get());
            }
        }

        std::sort(activeEmitters.begin(), activeEmitters.end(), [this](const Emitter* a, const Emitter* b) {
            return Vector3Distance(a->config.origin, camera->position) <
                   Vector3Distance(b->config.origin, camera->position);
        });

        for (auto& emitter : activeEmitters)
        {
            emitter->Draw(camera, shader);
        }
    }

    void ParticleSystem::DrawNearestFirst() const
    {

        for (auto& emitter : emitters)
        {
            emitter->DrawNearestFirst(camera);
        }
    }

    void ParticleSystem::DrawNearestFirst(const Shader& shader) const
    {
        std::vector<Emitter*> activeEmitters;
        for (const auto& emitter : emitters)
        {
            if (emitter->isEmitting)
            {
                activeEmitters.push_back(emitter.get());
            }
        }

        std::sort(activeEmitters.begin(), activeEmitters.end(), [this](const Emitter* a, const Emitter* b) {
            return Vector3Distance(a->config.origin, camera->position) <
                   Vector3Distance(b->config.origin, camera->position);
        });

        for (auto& emitter : activeEmitters)
        {
            emitter->DrawNearestFirst(camera, shader);
        }
    }

    void ParticleSystem::DrawOldestFirst() const
    {
        for (auto& emitter : emitters)
        {
            emitter->DrawOldestFirst(camera);
        }
    }

    void ParticleSystem::DrawOldestFirst(const Shader& shader) const
    {
        std::vector<Emitter*> activeEmitters;
        for (const auto& emitter : emitters)
        {
            if (emitter->isEmitting)
            {
                activeEmitters.push_back(emitter.get());
            }
        }

        std::sort(activeEmitters.begin(), activeEmitters.end(), [this](const Emitter* a, const Emitter* b) {
            return Vector3Distance(a->config.origin, camera->position) <
                   Vector3Distance(b->config.origin, camera->position);
        });

        for (auto& emitter : activeEmitters)
        {
            emitter->DrawOldestFirst(camera, shader);
        }
    }
} // namespace sage