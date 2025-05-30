#include "GL/gl.h"
#include "windows.h"
#include "map.h"
#include "math.h"
#include "camera.h"
#include "time.h"
#include "sound.h"

struct Structure_Camera camera = {
    0, 0, 1.7f,     // x, y, z
    0, 0,           // right_left, raise_lower
    0.0f,           // verticalVelocity
    0,              // isJumping
    1,              // jumpReleased (important to allow first jump)
    0,              // jumpKeyPrevState
    0,              // isSprinting
    1.7f,            // previousZ
    0.0f            // eyeOffsetZ
};

void Camera_Implement()
{
    glRotatef(-camera.raise_lower, 1, 0, 0);
    glRotatef(-camera.right_left,  0, 0, 1);
    glTranslatef(-camera.x, -camera.y, -(camera.z + camera.eyeOffsetZ));
}

void Camera_Rotating(float X_angle, float Z_angle)
{
    camera.right_left += Z_angle;
    if (camera.right_left < 0) camera.right_left += 360;
    if (camera.right_left > 360) camera.right_left -= 360;

    camera.raise_lower += X_angle;
    if (camera.raise_lower < 0) camera.raise_lower = 0;
    if (camera.raise_lower > 180) camera.raise_lower = 180;
}

void Camera_Mouse(int center_X, int center_Y, float sensitivity)
{
    POINT cursor;
    POINT base = {center_X, center_Y};
    GetCursorPos(&cursor);

    float deltaX = base.x - cursor.x;
    float deltaY = base.y - cursor.y;

    Camera_Rotating(deltaY * sensitivity, deltaX * sensitivity);
    SetCursorPos(base.x, base.y);
}

void Camera_Moving(int forwardMoving, int rightMoving, int jumpPressed, float speed)
{
    static int isPlaying = 0;
    static int lastSprintState = -1;

    // Crouch check
    int isCtrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
    if (isCtrlDown && !camera.isCrouching) {
        camera.eyeOffsetZ = -0.8f; // Lower camera view
        camera.isCrouching = 1;
    }
    else if (!isCtrlDown && camera.isCrouching) {
        camera.eyeOffsetZ = 0.0f; // Return to normal view
        camera.isCrouching = 0;
    }

    // Apply crouch penalty to speed
    if (camera.isCrouching)
        speed *= 0.4f;

    // Set sprinting flag
    camera.isSprinting = (speed > 0.1f);

    float playerHeight = camera.isCrouching ? 0.9f : 1.7f;
    float terrainHeight = Map_Obtain_Height(camera.x, camera.y);
    float groundHeight = terrainHeight + playerHeight;

    // Movement direction
    float angle = -camera.right_left / 180.0f * M_PI;

    if (forwardMoving > 0) {
        if (rightMoving > 0) angle += M_PI_4;
        else if (rightMoving < 0) angle -= M_PI_4;
    } else if (forwardMoving < 0) {
        angle += M_PI;
        if (rightMoving > 0) angle -= M_PI_4;
        else if (rightMoving < 0) angle += M_PI_4;
    } else {
        if (rightMoving > 0) angle += M_PI_2;
        else if (rightMoving < 0) angle -= M_PI_2;
        else speed = 0;
    }

    // Move character
    if (speed != 0) {
        camera.x += sin(angle) * speed;
        camera.y += cos(angle) * speed;
    }

    // Footstep/sprint sounds
    if (speed != 0 && camera.z <= groundHeight && !camera.isJumping) {
        int currentSprintState = camera.isSprinting;

        if (!isPlaying || currentSprintState != lastSprintState) {
            StopFootstepSound();
            if (currentSprintState) PlaySprintSound();
            else PlayWalkSound();

            isPlaying = 1;
            lastSprintState = currentSprintState;
        }
    } else {
        if (isPlaying) {
            StopFootstepSound();
            isPlaying = 0;
            lastSprintState = -1;
        }
    }

    // Gravity and jump
    float gravity = -0.003f;
    float jumpStrength = 0.18f;

    if (camera.z > groundHeight || camera.isJumping) {
        camera.verticalVelocity += gravity;
        camera.z += camera.verticalVelocity;

        if (camera.z <= groundHeight) {
            camera.z = groundHeight;
            camera.verticalVelocity = 0;

            if (camera.isJumping) {
                camera.isJumping = 0;
                camera.jumpReleased = 1;
                PlayLandSound();
            }
        }
    }

    // Jump input
    int isKeyDown = (GetAsyncKeyState(VK_SPACE) & 0x8000);
    if (isKeyDown && !camera.jumpKeyPrevState && !camera.isJumping && camera.jumpReleased) {
        camera.verticalVelocity = jumpStrength;
        camera.isJumping = 1;
        camera.jumpReleased = 0;
    }

    camera.jumpKeyPrevState = isKeyDown;
}
