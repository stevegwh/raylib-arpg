//
// Created by steve on 05/05/2024.
//

#pragma once

#include "raylib.h"
#include "cereal/cereal.hpp"

namespace sage
{
	struct KeyMapping
	{
		KeyboardKey keyA = KEY_A;
		KeyboardKey keyB = KEY_B;
		KeyboardKey keyC = KEY_C;
		KeyboardKey keyD = KEY_D;
		KeyboardKey keyE = KEY_E;
		KeyboardKey keyF = KEY_F;
		KeyboardKey keyG = KEY_G;
		KeyboardKey keyH = KEY_H;
		KeyboardKey keyI = KEY_I;
		KeyboardKey keyJ = KEY_J;
		KeyboardKey keyK = KEY_K;
		KeyboardKey keyL = KEY_L;
		KeyboardKey keyM = KEY_M;
		KeyboardKey keyN = KEY_N;
		KeyboardKey keyO = KEY_O;
		KeyboardKey keyP = KEY_P;
		KeyboardKey keyQ = KEY_Q;
		KeyboardKey keyR = KEY_R;
		KeyboardKey keyS = KEY_S;
		KeyboardKey keyT = KEY_T;
		KeyboardKey keyU = KEY_U;
		KeyboardKey keyV = KEY_V;
		KeyboardKey keyW = KEY_W;
		KeyboardKey keyX = KEY_X;
		KeyboardKey keyY = KEY_Y;
		KeyboardKey keyZ = KEY_Z;
		KeyboardKey keyEscape = KEY_ESCAPE;
		KeyboardKey keySpace = KEY_SPACE;
		KeyboardKey keyDelete = KEY_DELETE;

		template <class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(keyA),
				CEREAL_NVP(keyB),
				CEREAL_NVP(keyC),
				CEREAL_NVP(keyD),
				CEREAL_NVP(keyE),
				CEREAL_NVP(keyF),
				CEREAL_NVP(keyG),
				CEREAL_NVP(keyH),
				CEREAL_NVP(keyI),
				CEREAL_NVP(keyJ),
				CEREAL_NVP(keyK),
				CEREAL_NVP(keyL),
				CEREAL_NVP(keyM),
				CEREAL_NVP(keyN),
				CEREAL_NVP(keyO),
				CEREAL_NVP(keyP),
				CEREAL_NVP(keyQ),
				CEREAL_NVP(keyR),
				CEREAL_NVP(keyS),
				CEREAL_NVP(keyT),
				CEREAL_NVP(keyU),
				CEREAL_NVP(keyV),
				CEREAL_NVP(keyW),
				CEREAL_NVP(keyX),
				CEREAL_NVP(keyY),
				CEREAL_NVP(keyZ),
				CEREAL_NVP(keyEscape),
				CEREAL_NVP(keySpace),
				CEREAL_NVP(keyDelete)
			);
		}
	};
} // sage
