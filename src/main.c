// See this project's readme.md for more information.

#include "../include/raylib.h"
#include "../include/raymath.h"

#define G 600

// Structs for in-game objects:
typedef struct Player {
    Vector2 position;
    float speed;
    float movementSpeed;
    float jumpSpeed;
    bool canJump;
    float width;
    float height;
} Player;

typedef struct DeathRay {
  Vector2 position;
  float speed;
  float width;
  float height;
} DeathRay;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

// Starting time and current time before the Death Ray spawns in.
int deathRaySpawnTime = 0;

// Module functions declaration:
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta);
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

// Main game function:
int main(void)
{
    // Window stuff:
    const int screenWidth = 900;
    const int screenHeight = 500;

    // Enable VSYNC.
    SetConfigFlags(FLAG_VSYNC_HINT);
  
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  
    InitWindow(screenWidth, screenHeight, "Death Ray");

    // player definition.
    Player player = { 0 };
    player.position = (Vector2){ 400, 280 };
    player.speed = 0;
    player.movementSpeed = 200;
    player.jumpSpeed = 350;
    player.canJump = false;
    player.width = 30;
    player.height = 30;
  
    // deathRay definition.
    DeathRay deathRay = { 0 };
    deathRay.position = (Vector2){ 1000, 400 };
    deathRay.speed = 50;
    deathRay.width = 75;
    deathRay.height = 150;
  
    // Environment definition.
    EnvItem envItems[] = {
        {{ 0, 0, 1000, 400 }, 0, LIGHTGRAY },
        {{ 0, 400, 1000, 200 }, 1, GRAY },
        {{ 300, 200, 400, 10 }, 1, GRAY },
        {{ 250, 300, 100, 10 }, 1, GRAY },
        {{ 650, 300, 100, 10 }, 1, GRAY }
    };

    int envItemsLength = sizeof(envItems)/sizeof(envItems[0]);

    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Store pointers to the multiple update camera functions:
    void (*cameraUpdaters[])(Camera2D*, Player*, EnvItem*, int, float, int, int) = {
        UpdateCameraCenter,
        UpdateCameraCenterInsideMap,
        UpdateCameraCenterSmoothFollow,
        UpdateCameraEvenOutOnLanding,
        UpdateCameraPlayerBoundsPush
    };

    int cameraOption = 2;
    int cameraUpdatersLength = sizeof(cameraUpdaters)/sizeof(cameraUpdaters[0]);

    char *cameraDescriptions[] = {
        "Follow player center",
        "Follow player center, but clamp to map edges",
        "Follow player center; smoothed",
        "Follow player center horizontally; update player center vertically after landing",
        "Player push camera on getting too close to screen edge"
    };

    // SetTargetFPS(60);

    // Main game loop:
    while (!WindowShouldClose())
    {
        // Update:
        float deltaTime = GetFrameTime();

        // Calls UpdatePlayer to calculate gravity and player-world collision.
        UpdatePlayer(&player, envItems, envItemsLength, deltaTime);

        camera.zoom += ((float)GetMouseWheelMove()*0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            player.position = (Vector2){ 400, 280 };
        }

        if (IsKeyPressed(KEY_C)) cameraOption = (cameraOption + 1)%cameraUpdatersLength;

        // Call update camera function by its pointer.
        cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

        // Check player-deathRay collision in the x-axis, then y-axis. Account for player and deathRay size.
        if (player.position.x + player.width >= deathRay.position.x && player.position.x <= deathRay.position.x + deathRay.width && player.position.y - player.height <= deathRay.position.y && player.position.y >= deathRay.position.y - deathRay.height)
        {
          CloseWindow(); 
        }
      
        // Make the deathRay move.
        if (deathRaySpawnTime == 0)
        {
          deathRay.position.x -= deathRay.speed * GetFrameTime();
        }
      
        // Draw:
        BeginDrawing();

            ClearBackground(LIGHTGRAY);

            BeginMode2D(camera);
                
                // Draw all the platforms and stuff.
                for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

                // Draw the player's visual.
                Rectangle playerRect = { player.position.x, player.position.y - player.height, player.width, player.height };
                DrawRectangleRec(playerRect, DARKBLUE);
      
                // Draw the deathRay's visual.
                Rectangle deathRayRect = { deathRay.position.x, deathRay.position.y - deathRay.height, deathRay.width, deathRay.height };
                DrawRectangleRec(deathRayRect, MAROON);
                
                // Draw the player's hitbox.
                DrawCircleV(player.position, 3, GOLD);
      
                // Draw the deathRay's hitbox.
                DrawCircleV(deathRay.position, 3, GOLD);

            EndMode2D();

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
            DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom and player position", 40, 80, 10, DARKGRAY);
            DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
            DrawText("Current camera mode:", 20, 120, 10, BLACK);
            DrawText(cameraDescriptions[cameraOption], 40, 140, 10, DARKGRAY);
            DrawText("Current player positions:", 20, 160, 10, BLACK);
            DrawText(TextFormat ("True X Position: %12.3f", player.position.x), 40, 180, 10, DARKGRAY);
            DrawText(TextFormat ("True Y Position: %12.3f", player.position.y), 40, 200, 10, DARKGRAY);

        EndDrawing();
    }

    // De-initialization.
    CloseWindow();

    return 0;
}

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta)
{
    if (IsKeyDown(KEY_LEFT)) player -> position.x -= player -> movementSpeed * delta;
  
    if (IsKeyDown(KEY_RIGHT)) player -> position.x += player -> movementSpeed * delta;
  
    if (IsKeyDown(KEY_SPACE) && player -> canJump)
    {
        player -> speed = -player -> jumpSpeed;
        player -> canJump = false;
    }

    bool hitObstacle = false;
  
    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player -> position);
        if (ei -> blocking &&
            ei -> rect.x - player -> width <= p -> x &&
            ei -> rect.x + ei -> rect.width >= p -> x &&
            ei -> rect.y >= p -> y &&
            ei -> rect.y <= p -> y + player -> speed * delta)
        {
            hitObstacle = true;
            player -> speed = 0.0f;
            p -> y = ei -> rect.y;
            break;
        }
    }

    if (!hitObstacle)
    {
        player -> position.y += player -> speed * delta;
        player -> speed += G * delta;
        player -> canJump = false;
    }
    else player -> canJump = true;
}

// Shit I haven't looked at to understand yet:

// Camera logic:
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    camera->target = player->position;
}

void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->target = player->position;
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

    if (max.x < width) camera->offset.x = width - (max.x - width/2);
    if (max.y < height) camera->offset.y = height - (max.y - height/2);
    if (min.x > 0) camera->offset.x = width/2 - min.x;
    if (min.y > 0) camera->offset.y = height/2 - min.y;
}

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static float minSpeed = 30;
    static float minEffectLength = 10;
    static float fractionSpeed = 0.8f;

    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    Vector2 diff = Vector2Subtract(player->position, camera->target);
    float length = Vector2Length(diff);

    if (length > minEffectLength)
    {
        float speed = fmaxf(fractionSpeed*length, minSpeed);
        camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
    }
}

void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static float evenOutSpeed = 700;
    static int eveningOut = false;
    static float evenOutTarget;

    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    camera->target.x = player->position.x;

    if (eveningOut)
    {
        if (evenOutTarget > camera->target.y)
        {
            camera->target.y += evenOutSpeed*delta;

            if (camera->target.y > evenOutTarget)
            {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
        else
        {
            camera->target.y -= evenOutSpeed*delta;

            if (camera->target.y < evenOutTarget)
            {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
    }
    else
    {
        if (player->canJump && (player->speed == 0) && (player->position.y != camera->target.y))
        {
            eveningOut = 1;
            evenOutTarget = player->position.y;
        }
    }
}

void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static Vector2 bbox = { 0.2f, 0.2f };

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, *camera);
    camera->offset = (Vector2){ (1 - bbox.x)*0.5f * width, (1 - bbox.y)*0.5f*height };

    if (player->position.x < bboxWorldMin.x) camera->target.x = player->position.x;
    if (player->position.y < bboxWorldMin.y) camera->target.y = player->position.y;
    if (player->position.x > bboxWorldMax.x) camera->target.x = bboxWorldMin.x + (player->position.x - bboxWorldMax.x);
    if (player->position.y > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (player->position.y - bboxWorldMax.y);
}
