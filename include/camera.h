#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

struct Structure_Camera{

    float x, y, z;
    float raise_lower, right_left;

    // jumping mechanics
    float verticalVelocity;
    int isJumping;
    int jumpReleased;
    int jumpKeyPrevState;
    float previousZ;

    // sprinting by pressing SHIFT
    int isSprinting;

    // crouching
    int isCrouching;
    float eyeOffsetZ;
    float targetOffsetZ;
};

extern struct Structure_Camera camera;

void Camera_Implement();
void Camera_Rotating(float X_angle, float Z_angle);
void Camera_Mouse(int center_X, int center_Y, float speed);
void Camera_Moving(int forwardMoving, int rightMoving, int jumpPressed, float speed);


#endif // CAMERA_H_INCLUDED
