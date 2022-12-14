#include "crscene.h"
#include <assert.h>
#include <stdlib.h>
#include "crtime.h"

Scene* CreateTestScene(
	const PlayerSettings const* settings,
	float wallHeight,
	float size)
{
	Scene* scene = malloc(sizeof(Scene));
	assert(scene != NULL);

	scene->player =
	(Player){
		.frame = CreateFrame2D(0.0f, 0.0f, 0.0f),
		.settings =
		(PlayerSettings){
			.arrowSize = settings->arrowSize,
			.baseSize = settings->baseSize,
			.transSpeed = settings->transSpeed,
			.rotSpeed = settings->rotSpeed,
			.fov = settings->fov
		}
	};

	scene->walls = CreateDoubleLinkedList();
	scene->wallHeight = wallHeight;

	scene->colors =
	(SceneColors){
		.clearCol = CreateColorRGB(0, 0, 0),
		.wallCol = CreateColorRGB(255, 255, 255),
		.floorCol = CreateColorRGB(52, 52, 52),
		.ceilingColor = CreateColorRGB(52, 52, 128),
		.playerCol = CreateColorRGB(0, 0, 255),
		.rayCol = CreateColorRGB(255, 0, 0),
		.intersectCol = CreateColorRGB(0, 255, 0)
	};

	scene->camera =
	(Frame2D){
		.position = {.x = 0.0f, .y = 0.0f },
		.theta = 0.0f
	};

	float hsize = size / 2.0f;

	Point2D p1 = { .x = -size, .y = -hsize };
	Point2D p2 = { .x = -hsize, .y = hsize };
	Point2D p3 = { .x = size, .y = size };
	Point2D p4 = { .x = size, .y = -hsize };
	Point2D p5 = { .x = hsize, .y = -hsize };
	Point2D p6 = { .x = hsize, .y = -size };
	Point2D p7 = { .x = -hsize, .y = -size };
	Point2D p8 = { .x = -hsize, .y = -hsize };

	LineSegment2D* L1 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L2 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L3 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L4 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L5 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L6 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L7 = malloc(sizeof(LineSegment2D));
	LineSegment2D* L8 = malloc(sizeof(LineSegment2D));

	assert(L1 != NULL);
	assert(L2 != NULL);
	assert(L3 != NULL);
	assert(L4 != NULL);
	assert(L5 != NULL);
	assert(L6 != NULL);
	assert(L7 != NULL);
	assert(L8 != NULL);

	L1->p1 = p1;
	L1->p2 = p2;
	L2->p1 = p2;
	L2->p2 = p3;
	L3->p1 = p3;
	L3->p2 = p4;
	L4->p1 = p4;
	L4->p2 = p5;
	L5->p1 = p5;
	L5->p2 = p6;
	L6->p1 = p6;
	L6->p2 = p7;
	L7->p1 = p7;
	L7->p2 = p8;
	L8->p1 = p8;
	L8->p2 = p1;

	PushDLLNode(&scene->walls, L1);
	PushDLLNode(&scene->walls, L2);
	PushDLLNode(&scene->walls, L3);
	PushDLLNode(&scene->walls, L4);
	PushDLLNode(&scene->walls, L5);
	PushDLLNode(&scene->walls, L6);
	PushDLLNode(&scene->walls, L7);
	PushDLLNode(&scene->walls, L8);

	return scene;
}

void CleanupScene(Scene* scene)
{
	if (scene == NULL)
	{
		return;
	}

	DLLNode* current = scene->walls.head;

	while (current != NULL)
	{
		LineSegment2D* data = (LineSegment2D*)current->data;
		free(data);
		current = current->next;
	}

	ClearDoubleLinkedList(&scene->walls);

	free(scene);
	scene = NULL;
}

void UpdatePlayerPosition(
	Scene* const scene,
	float deltaMS,
	const InputState* const inputState,
	CycleProfile* const profile)
{
	uint64_t updateStartTime = GetTicks();

	const Vector2D worldForward = { .x = 0.0f, .y = -1.0f };

	if (inputState->forwards)
	{
		
		Vector2D lookDir =
			FindLookVector(
				&worldForward,
				scene->player.frame.theta
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, scene->player.settings.transSpeed * deltaMS)
			);
	}

	if (inputState->backwards)
	{
		Vector2D worldForward = { .x = 0.0f, .y = -1.0f };
		Vector2D lookDir =
			FindLookVector(
				&worldForward,
				scene->player.frame.theta
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, -scene->player.settings.transSpeed * deltaMS)
			);
	}

	if (inputState->right)
	{
		Vector2D worldForward = { .x = 0.0f, .y = -1.0f };
		Vector2D lookDir =
			FindLookVector(
				&worldForward,
				scene->player.frame.theta + 90.0f
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, scene->player.settings.transSpeed * deltaMS)
			);
	}

	if (inputState->left)
	{
		Vector2D worldForward = { .x = 0.0f, .y = -1.0f };
		Vector2D lookDir =
			FindLookVector(
				&worldForward,
				scene->player.frame.theta - 90.0f
			);

		lookDir = Vec2DNormalise(lookDir);

		scene->player.frame.position =
			AddVec2DToPoint2D(
				scene->player.frame.position,
				Vec2DMul(lookDir, scene->player.settings.transSpeed * deltaMS)
			);
	}

	if (inputState->rotRight)
	{
		scene->player.frame.theta += scene->player.settings.rotSpeed * deltaMS;
	}

	if (inputState->rotLeft)
	{
		scene->player.frame.theta -= scene->player.settings.rotSpeed * deltaMS;
	}

	profile->updatePlayerTimeMS = GetTimeInMS(GetTicks() - updateStartTime);
}
