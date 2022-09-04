#include "crscene.h"
#include "crconsts.h"
#include "crtime.h"

Player CreateDefaultPlayer()
{
	Player player =
	{
		.frame = CreateFrame2D(0.0, 0.0, 0.0),
		30.0
	};

	return player;
}

Scene CreateDefaultScene()
{
	Scene scene =
	{
		.player = CreateDefaultPlayer(),
		.walls = CreateDoubleLinkedList(),
		.colors =
		{
			.clearCol = CreateColorRGB(0, 0, 0),
			.wallCol = CreateColorRGB(255, 255, 255),
			.floorCol = CreateColorRGB(52, 52, 52),
			.ceilingColor = CreateColorRGB(52, 52, 128),
			.playerCol = CreateColorRGB(0, 0, 255),
			.rayCol = CreateColorRGB(255, 0, 0),
			.intersectCol = CreateColorRGB(0, 255, 0)
		}
	};

	return scene;
}

void UpdatePlayerPosition(
	Scene* const scene,
	const InputState* const inputState,
	CycleProfile* const profile)
{
	uint64_t updateStartTime = GetTicks();

	if (inputState->forwards)
	{
		Vector2D worldForward = { .x = 0.0, .y = -1.0 };
		Vector2D lookDir =
			FindLookVector(
				worldForward,
				scene->player.frame.theta
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, TRANS_SPEED)
			);
	}

	if (inputState->backwards)
	{
		Vector2D worldForward = { .x = 0.0, .y = -1.0 };
		Vector2D lookDir =
			FindLookVector(
				worldForward,
				scene->player.frame.theta
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, -TRANS_SPEED)
			);
	}

	if (inputState->right)
	{
		Vector2D worldForward = { .x = 0.0, .y = -1.0 };
		Vector2D lookDir =
			FindLookVector(
				worldForward,
				scene->player.frame.theta + 90.0
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, TRANS_SPEED)
			);
	}

	if (inputState->left)
	{
		Vector2D worldForward = { .x = 0.0, .y = -1.0 };
		Vector2D lookDir =
			FindLookVector(
				worldForward,
				scene->player.frame.theta - 90.0
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, TRANS_SPEED)
			);
	}

	if (inputState->rotRight)
	{
		scene->player.frame.theta += ROT_SPEED;
	}

	if (inputState->rotLeft)
	{
		scene->player.frame.theta -= ROT_SPEED;
	}

	profile->updatePlayerTimeMS = GetTimeInMS(GetTicks() - updateStartTime);
}
