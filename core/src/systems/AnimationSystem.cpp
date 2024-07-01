//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationSystem.hpp"
#include "../components/Renderable.hpp"
#include <omp.h>
#include "rlgl.h"

namespace sage
{

    void UpdateModelAnimation(Model model, ModelAnimation anim, int frame)
    {
        if ((anim.frameCount > 0) && (anim.bones != NULL) && (anim.framePoses != NULL))
        {
            if (frame >= anim.frameCount) frame = frame%anim.frameCount;
    
            // Vector to store which meshes were updated
            std::vector<bool> meshUpdated(model.meshCount, false);
    
            int num_threads = omp_get_max_threads();
            omp_set_num_threads(num_threads);
    
    #pragma omp parallel
            {
    //#pragma omp single
    //            printf("Number of threads: %d\n", omp_get_num_threads());
    
    #pragma omp for
                for (int m = 0; m < model.meshCount; m++)
                {
                    //printf("Processing mesh %d on thread %d\n", m, omp_get_thread_num());
    
                    Mesh* mesh = &model.meshes[m];
    
                    if (mesh->boneIds == NULL || mesh->boneWeights == NULL)
                    {
    #pragma omp critical
                        {
                            TRACELOG(LOG_WARNING, "MODEL: UpdateModelAnimation(): Mesh %i has no connection to bones", m);
                        }
                        continue;
                    }
    
                    bool updated = false;
                    Vector3 animVertex = { 0 };
                    Vector3 animNormal = { 0 };
    
                    Vector3 inTranslation = { 0 };
                    Quaternion inRotation = { 0 };
    
                    Vector3 outTranslation = { 0 };
                    Quaternion outRotation = { 0 };
                    Vector3 outScale = { 0 };
    
                    int boneId = 0;
                    int boneCounter = 0;
                    float boneWeight = 0.0;
    
                    const int vValues = mesh->vertexCount*3;
                    for (int vCounter = 0; vCounter < vValues; vCounter += 3)
                    {
                        mesh->animVertices[vCounter] = 0;
                        mesh->animVertices[vCounter + 1] = 0;
                        mesh->animVertices[vCounter + 2] = 0;
    
                        if (mesh->animNormals != NULL)
                        {
                            mesh->animNormals[vCounter] = 0;
                            mesh->animNormals[vCounter + 1] = 0;
                            mesh->animNormals[vCounter + 2] = 0;
                        }
    
                        // Iterates over 4 bones per vertex
                        for (int j = 0; j < 4; j++, boneCounter++)
                        {
                            boneWeight = mesh->boneWeights[boneCounter];
    
                            if (boneWeight == 0.0f) continue;
    
                            boneId = mesh->boneIds[boneCounter];
                            inTranslation = model.bindPose[boneId].translation;
                            inRotation = model.bindPose[boneId].rotation;
                            outTranslation = anim.framePoses[frame][boneId].translation;
                            outRotation = anim.framePoses[frame][boneId].rotation;
                            outScale = anim.framePoses[frame][boneId].scale;
    
                            animVertex = { mesh->vertices[vCounter], mesh->vertices[vCounter + 1], mesh->vertices[vCounter + 2] };
                            animVertex = Vector3Subtract(animVertex, inTranslation);
                            animVertex = Vector3Multiply(animVertex, outScale);
                            animVertex = Vector3RotateByQuaternion(animVertex, QuaternionMultiply(outRotation, QuaternionInvert(inRotation)));
                            animVertex = Vector3Add(animVertex, outTranslation);
                            mesh->animVertices[vCounter] += animVertex.x*boneWeight;
                            mesh->animVertices[vCounter + 1] += animVertex.y*boneWeight;
                            mesh->animVertices[vCounter + 2] += animVertex.z*boneWeight;
                            updated = true;
    
                            if (mesh->normals != NULL)
                            {
                                animNormal = { mesh->normals[vCounter], mesh->normals[vCounter + 1], mesh->normals[vCounter + 2] };
                                animNormal = Vector3RotateByQuaternion(animNormal, QuaternionMultiply(outRotation, QuaternionInvert(inRotation)));
                                mesh->animNormals[vCounter] += animNormal.x*boneWeight;
                                mesh->animNormals[vCounter + 1] += animNormal.y*boneWeight;
                                mesh->animNormals[vCounter + 2] += animNormal.z*boneWeight;
                            }
                        }
                    }
    
                    if (updated)
                    {
                        meshUpdated[m] = true;
                    }
                }
            }
    
            // Update vertex buffers sequentially after parallel computation
            for (int m = 0; m < model.meshCount; m++)
            {
                if (meshUpdated[m])
                {
                    Mesh* mesh = &model.meshes[m];
                    rlUpdateVertexBuffer(mesh->vboId[0], mesh->animVertices, mesh->vertexCount*3*sizeof(float), 0);
                    if (mesh->animNormals != NULL)
                    {
                        rlUpdateVertexBuffer(mesh->vboId[2], mesh->animNormals, mesh->vertexCount*3*sizeof(float), 0);
                    }
                }
            }
        }
    }
    
	void AnimationSystem::Update() const
	{

        const auto& view = registry->view<Animation, Renderable>();
		for (auto& entity : view)
		{
			auto& a = registry->get<Animation>(entity);
			const auto& r = registry->get<Renderable>(entity);
			const ModelAnimation& anim = a.animations[a.animIndex];

			if (a.animCurrentFrame == 0)
			{
				a.onAnimationStart.publish(entity);
			}

			if (a.animCurrentFrame + 1 >= anim.frameCount)
			{
				a.onAnimationEnd.publish(entity);
				if (a.oneShot) continue;
			}
			if (!registry->valid(entity)) continue;
			a.animCurrentFrame = (a.animCurrentFrame + 1) % anim.frameCount;
			sage::UpdateModelAnimation(r.model, anim, a.animCurrentFrame);
		}
	}
    
    

	void AnimationSystem::Draw()
	{
	}

	AnimationSystem::AnimationSystem(entt::registry* _registry)
		: BaseSystem<Animation>(_registry)
	{
	}
} // sage
