//
// Created by Steve Wheeler on 23/07/2024.
//

#include "FlameEffect.hpp"

namespace sage
{
	Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer)
	{
		Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
		float radius = (width < height)? (float)width/2.0f : (float)height/2.0f;

		float centerX = (float)width/2.0f;
		float centerY = (float)height/2.0f;

		// Set outer color's alpha to 0 (fully transparent)
		outer.a = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				float dist = hypotf((float)x - centerX, (float)y - centerY);
				float factor = (dist - radius*density)/(radius*(1.0f - density));

				factor = (float)fmax(factor, 0.0f);
				factor = (float)fmin(factor, 1.f);

				// Calculate alpha first
				unsigned char alpha = (unsigned char)((float)outer.a*factor + (float)inner.a*(1.0f - factor));

				// Only set color if alpha is not zero
				if (alpha > 0)
				{
					pixels[y*width + x].r = (unsigned char)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
					pixels[y*width + x].g = (unsigned char)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
					pixels[y*width + x].b = (unsigned char)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
					pixels[y*width + x].a = alpha;
				}
				else
				{
					pixels[y*width + x] = (Color){0, 0, 0, 0}; // Fully transparent
				}
			}
		}

		Image image = {
				.data = pixels,
				.width = width,
				.height = height,
				.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
				.mipmaps = 1
		};

		return image;
	}
	
	FlameEffect::~FlameEffect()
	{
		UnloadTexture(texCircle8);
		UnloadTexture(texCircle16);
	}

	FlameEffect::FlameEffect(Camera3D* cam) :ParticleSystem(cam)
	{
		Image imgCircle16 = GenImageGradientRadialTrans(16, 16, 0.3f, WHITE, BLACK);
		texCircle16 = LoadTextureFromImage(imgCircle16);
		Image imgCircle8 = GenImageGradientRadialTrans(8, 8, 0.5f, WHITE, BLACK);
		texCircle8 = LoadTextureFromImage(imgCircle8);
		ExportImage(imgCircle16, "resources/radialimage.png");
		
		UnloadImage(imgCircle8);
		UnloadImage(imgCircle16);

		EmitterConfig ecfg1;
		ecfg1.size = 0.5f;
		ecfg1.direction = Vector3{ 0, 1, 0 };
		ecfg1.velocity = FloatRange{ 0.7, 0.73 };
		ecfg1.directionAngle = FloatRange{ -6, 6 };
		ecfg1.velocityAngle = FloatRange{ 0, 0 };
		ecfg1.offset = FloatRange{ 0, 0 };
		ecfg1.originAcceleration = FloatRange{ 0, 0 };
		ecfg1.burst = IntRange{ 0, 0 };
		ecfg1.capacity = 600;
		ecfg1.emissionRate = 200;
		ecfg1.origin = Vector3{ 0, 0, 0 };
		ecfg1.externalAcceleration = Vector3{ 0, 0.85, 0 };
		ecfg1.startColor = Color{ 255, 100, 0, 255 };
		ecfg1.endColor = Color{ 255, 150, 150, 70 };
		ecfg1.age = FloatRange{ 0, 1.0 };
		ecfg1.blendMode = BLEND_ADDITIVE;
		ecfg1.texture = texCircle16;
		ecfg1.particle_Deactivator = Particle_DeactivatorAge;

		auto emitterFountain1 = std::make_unique<Emitter>(ecfg1);
		Register(std::move(emitterFountain1));

		ecfg1.directionAngle = FloatRange{ -1.5, 1.5 };
		ecfg1.velocity = FloatRange{ 0.8, 0.85 };
		ecfg1.texture = texCircle8;
		auto emitterFountain2 = std::make_unique<Emitter>(ecfg1);
		Register(std::move(emitterFountain2));

		Start();

	}
}