#include "windows.h"
#include "gl/gl.h"
#include "math.h"
#include "mmsystem.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "main.h"
#include "sound.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

void Animation_Setup(TInteraction *anm, TObject *obj)
{
    if(anm->obj != NULL)
        return;

    anm->obj = obj;
    anm->counter = 10;
    anm->dx = (camera.x - obj->x) / (float)anm->counter;
    anm->dy = (camera.y - obj->y) / (float)anm->counter;
    anm->dz = ((camera.z - obj->scale - 0.2) - obj->z) / (float)anm->counter;
}

void Animated(TInteraction *anm)
{
    if(anm->obj != NULL)
    {
        anm->obj->x = anm->obj->x + anm->dx;
        anm->obj->y = anm->obj->y + anm->dy;
        anm->obj->z = anm->obj->z + anm->dz;
        anm->counter--;

            if(anm->counter < 1)
            {
                int i;

                for(i = 0; i < SlotSize; i++)
                    if(slot[i].type < 0)
                    {
                        slot[i].type = anm->obj->type;
                        break;
                    }

                if(i < SlotSize)
                {
                    anm->obj->x = rand() % map_width;
                    anm->obj->y = rand() % map_height;
                }
                anm->obj->z = Map_Obtain_Height(anm->obj->x, anm->obj->y);

                anm->obj = NULL;

            }
    }
}

void Tree_Present(TObjectGroup object)
{
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3,GL_FLOAT, 0, cube_tree);
        glColor3f(0.7,0.7,0.7);
        glNormal3f(0, 0, 1);
        glBindTexture(GL_TEXTURE_2D, object.type);
            for(int i = 0; i < object.stuff_counter; i++)
            {
                if(object.stuff[i].type == 1)
                    glTexCoordPointer(2, GL_FLOAT, 0, cube_tree_UV_trunk);
                else
                    glTexCoordPointer(2, GL_FLOAT, 0, cube_tree_UV_leaves);

                glPushMatrix();
                    glTranslatef(object.stuff[i].x, object.stuff[i].y, object.stuff[i].z);
                    glDrawElements(GL_TRIANGLES, cube_tree_counter, GL_UNSIGNED_INT, cube_tree_Index);
                glPopMatrix();
            }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void Tree_Create(TObjectGroup *object, int type, float x, float y)
{
    object->type = type;

    float z_axis = Map_Obtain_Height(x + 0.5, y + 0.5) - 0.5;
    int tree_height = 6;
    int leaves = 5 * 5 * 2 - 2 + 3 * 3 * 2;

    object->stuff_counter = tree_height + leaves;
    object->stuff = malloc(sizeof(TObject) * object->stuff_counter);

    for(int i = 0; i < tree_height; i++)
    {
        object->stuff[i].type = 1;
        object->stuff[i].x = x;
        object->stuff[i].y = y;
        object->stuff[i].z = z_axis + i;
    }

    ///
    int position = tree_height;

    for(int i = 0; i < 2; i++)
        for(int j = x - 2; j <= x + 2; j++)
            for(int k = y - 2; k <= y + 2; k++)
                if((j != x) || (k != y))
                {
                    object->stuff[position].type = 2;
                    object->stuff[position].x = j;
                    object->stuff[position].y = k;
                    object->stuff[position].z = z_axis + tree_height - 2 + i;
                    position++;
                }
    ///

    for(int i = 0; i < 2; i++)
        for(int j = x - 1; j <= x + 1; j++)
            for(int k = y - 1; k <= y + 1; k++)
            {
                object->stuff[position].type = 2;
                object->stuff[position].x = j;
                object->stuff[position].y = k;
                object->stuff[position].z = z_axis + tree_height + i;
                position++;
            }

}

/// part of code

void Loading_Texture(char *file_name, int *aim)
{
    int width, height, counter;

    unsigned char *data = stbi_load(file_name, &width, &height, &counter, 0);
    glGenTextures(1, aim);
    glBindTexture(GL_TEXTURE_2D, *aim);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, counter == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data); /// free the data

}

/// part of code

/// part of code
#define sqr(a) (a)*(a)

void Calculating_Normals(TCell a, TCell b, TCell c, TCell *n)
{
    float wrki;
    TCell v1, v2;

    // Calculate vectors
    v1.x = a.x - b.x;
    v1.y = a.y - b.y;
    v1.z = a.z - b.z;

    v2.x = b.x - c.x;
    v2.y = b.y - c.y;
    v2.z = b.z - c.z;

    // Cross product to find normal
    n->x = (v1.y * v2.z - v1.z * v2.y);
    n->y = (v1.z * v2.x - v1.x * v2.z);
    n->z = (v1.x * v2.y - v1.y * v2.x);

    // Calculate the magnitude of the normal
    wrki = sqrt(sqr(n->x) + sqr(n->y) + sqr(n->z));

    // Normalize the normal vector
    n->x = n->x / wrki;
    n->y = n->y / wrki;
    n->z = n->z / wrki;  // Corrected division by wrki
}

/// part of code

BOOL IsCoordInMap(float x, float y)
{
    return (x >= 0) && (x < map_width) && (y >= 0) && (y < map_height);
}

void Map_Create_Hills(int pos_X, int pos_Y, int radius, int height)
{
    for(int i = pos_X - radius; i <= pos_X + radius; i++)
        for(int j = pos_Y - radius; j <= pos_Y + radius; j++)
            if(IsCoordInMap(i,j))
            {
                float length = sqrt(pow(pos_X - i, 2) + pow(pos_Y - j, 2));

                if(length < radius)
                {
                    length = length / radius * M_PI_2;
                    map[i][j].z = map[i][j].z + cos(length) * height;
                }

            }

}

float Map_Obtain_Height(float x, float y)
{
    if(!IsCoordInMap(x, y))
        return 0;
    int c_X = (int)x; // casting
    int c_Y = (int)y; // casting

    float height1 = ( (1 - (x - c_X)) * map[c_X][c_Y].z      +   (x - c_X) * map[c_X + 1][c_Y].z);
    float height2 = ( (1 - (x - c_X)) * map[c_X][c_Y + 1].z  +   (x - c_X) * map[c_X + 1][c_Y + 1].z);

    return (1 - (y - c_Y)) * height1 + (y - c_Y) * height2;

}


void Map_Create()
{
    for(int i = 0; i < SlotSize; i++)
        slot[i].type = -1;

    Loading_Texture("assets/textures/ground.png",   &texture_ground);
    Loading_Texture("assets/textures/grass.png",    &texture_grass);
    Loading_Texture("assets/textures/flower.png",   &texture_flower);
    Loading_Texture("assets/textures/flower2.png",  &texture_flower2);
    Loading_Texture("assets/textures/mushroom.png", &texture_mushroom);
    Loading_Texture("assets/textures/tree.png",     &texture_tree);
    Loading_Texture("assets/textures/tree2.png",    &texture_tree2);
    Loading_Texture("assets/textures/wood_tree.png",    &texture_wood);

    /// lighting


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    /// ligting

    /// removing the black frame

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.99);

    ///removing the black frame

    for(int i = 0; i < map_width; i++)
        for(int j = 0; j < map_height; j++)
        {
            map[i][j].x = i;
            map[i][j].y = j;
            map[i][j].z = (rand() % 10) * 0.02;

            mapUV[i][j].u = i;
            mapUV[i][j].v = j;
        }

    for(int i = 0; i < map_width - 1; i++)
    {
        int position = i * map_height;
        for(int j = 0; j < map_height - 1; j++)
        {
            mapIndex[i][j][0] = position;
            mapIndex[i][j][1] = position + 1;
            mapIndex[i][j][2] = position + 1 + map_height;

            mapIndex[i][j][3] = position + 1 + map_height;
            mapIndex[i][j][4] = position + map_height;
            mapIndex[i][j][5] = position;

            position++;
        }
    }

    for(int i = 0; i < 10; i++)
        Map_Create_Hills(rand() % map_width, rand() % map_height, rand() % 50, rand() % 10);

    for(int i = 0; i < map_width - 1; i++)
        for(int j = 0; j < map_height - 1; j++)
            Calculating_Normals(map[i][j], map[i + 1][j], map[i][j + 1], &mapImproved[i][j]);

    int grass_quantity = 2000;
    int mushroom_quantity = 30;
    int tree_quantity = 40;

    plant_counter = grass_quantity + mushroom_quantity + tree_quantity;
    plant_array = realloc(plant_array, sizeof(*plant_array) * plant_counter);
        for(int i = 0; i < plant_counter; i++)
        {
            if(i < grass_quantity)
            {
                plant_array[i].type = rand() % 10 != 0 ? texture_grass :
                                     (rand() % 2  == 0  ? texture_flower : texture_flower2);
                plant_array[i].scale = 0.7 + (rand() % 5) * 0.1;
            }
            else if (i < (grass_quantity + mushroom_quantity))
            {
                plant_array[i].type = texture_mushroom;
                plant_array[i].scale = 0.2 + (rand() % 10) * 0.01;
            }
            else
            {
                plant_array[i].type = rand() % 2 == 0 ? texture_tree : texture_tree2;
                plant_array[i].scale = 4 + (rand() % 14);
            }
            plant_array[i].x = rand() % map_width;
            plant_array[i].y = rand() % map_height;
            plant_array[i].z = Map_Obtain_Height(plant_array[i].x, plant_array[i].y);

        }

    tree_counter = 50;
    tree = realloc(tree, sizeof(*tree) * tree_counter);
        for(int i = 0; i < tree_counter; i++)
            Tree_Create(tree + i, texture_wood, rand() % map_width, rand() % map_height);

}

void Map_Present()
{
    ///
    float size = 0.1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-screenKoef * size, screenKoef * size, -size, size, size * 2, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ///

    glEnable(GL_DEPTH_TEST);


    static float alfa = 0;
    alfa = alfa + 0.001; // time of day and night cycle

    if(alfa > 180)
        alfa = alfa - 360;

    #define abs(a) ((a) > 0 ? (a) : -(a))
    float color_changing = 1 - (abs(alfa) / 180);

    #define sunset 40
    float sunset_time = 90 - abs(alfa);
    sunset_time = (sunset - abs(sunset_time));

    if(sunset_time < 0)
        sunset_time = 0;
    else
        sunset_time = sunset_time / sunset;

    //sunset_time = sunset_time < 0 ? 0 : sunset_time / sunset;

    ///
            ///
                if(selectingMode)
                    glClearColor(0,0,0,0);
            ///
                else
                    glClearColor(0.6f * color_changing, 0.8f * color_changing, 1.0f * color_changing, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if(selectingMode)
            {
              glDisable(GL_TEXTURE_2D);
              glDisable(GL_LIGHTING);
            }
            else
            {
              glEnable(GL_LIGHTING);
              glEnable(GL_TEXTURE_2D);
            }

        Animated(&animation);

        glPushMatrix();

            if(!selectingMode) // dealing with selecting object
            {
            glPushMatrix();

                glRotatef(-camera.raise_lower, 1, 0, 0);
                glRotatef(-camera.right_left,  0, 0, 1);
                glRotatef(alfa, 0, 1, 0);
                glTranslatef(0,0,20);
                glDisable(GL_DEPTH_TEST);

                    glDisable(GL_TEXTURE_2D);
                    glColor3f(1,1 - sunset_time * 0.8 ,0 - sunset_time);
                    glEnableClientState(GL_VERTEX_ARRAY);
                        glVertexPointer(3,GL_FLOAT,0,sun_shape);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glEnable(GL_TEXTURE_2D);

                glEnable(GL_DEPTH_TEST);

            glPopMatrix();
              }

            Camera_Implement();

            ///
            glPushMatrix();

                glRotatef(alfa, 0, 1, 0);
                GLfloat position[] = {0, 0, 1, 0};
                glLightfv(GL_LIGHT0, GL_POSITION, position);

                float sunset_beams[] = {1 + sunset_time * 2, 1, 1, 0};
                glLightfv(GL_LIGHT0, GL_DIFFUSE, sunset_beams);

                float colour = color_changing * 0.15 + 0.05;
                float ambient_light_intensity[] = {colour, colour, colour, 0};
                 glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light_intensity);

            glPopMatrix();
            ///
            if(!selectingMode) // dealing with selecting object
            {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
                glVertexPointer(3, GL_FLOAT, 0, map);
                glTexCoordPointer(2, GL_FLOAT,  0, mapUV);
                glColor3f(0.7, 0.7, 0.7);
                glNormalPointer(GL_FLOAT, 0, mapImproved);
                glBindTexture(GL_TEXTURE_2D, texture_ground);
                glDrawElements(GL_TRIANGLES, mapIndexCount, GL_UNSIGNED_INT, mapIndex);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            }
            /// ========================================================================
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glVertexPointer(3, GL_FLOAT, 0, plant);
                glTexCoordPointer(2, GL_FLOAT,  0, plantUV);
                glColor3f(0.7, 0.7, 0.7);
                glNormal3f(0,0,1);
                selecting_object_counter = 0;
                int selecting_colour = 1;

                    for(int i = 0; i < plant_counter; i++)
                    {
                        if(selectingMode)
                        {
                            if((plant_array[i].type == texture_tree) || (plant_array[i].type == texture_tree2))
                                continue;

                            static int radius = 3;
                            if((plant_array[i].x > camera.x - radius) && (plant_array[i].x < camera.x + radius)
                                                                      &&
                               (plant_array[i].y > camera.y - radius) && (plant_array[i].y < camera.y + radius))
                            {
                                glColor3ub(0, selecting_colour, 0);
                                select_object[selecting_object_counter].color_index = selecting_colour;
                                select_object[selecting_object_counter].planted_object_index = i;
                                selecting_object_counter++;
                                selecting_colour++;

                                if(selecting_colour >= 255)
                                    break;

                            }
                            else
                                continue;

                        }

                      glBindTexture(GL_TEXTURE_2D, plant_array[i].type);
                      glPushMatrix();
                        glTranslatef(plant_array[i].x, plant_array[i].y, plant_array[i].z);
                        glScalef( plant_array[i].scale, plant_array[i].scale, plant_array[i].scale);
                        glDrawElements(GL_TRIANGLES, plant_index_counter, GL_UNSIGNED_INT, plant_index);
                      glPopMatrix();
                    }

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            if(!selectingMode) // dealing with selecting object
                for(int i = 0; i < tree_counter; i++)
                    Tree_Present(tree[i]);

        glPopMatrix();
    ///


}

void Player_Move()
{
    int forward = 0;
    int right = 0;
    int jump = 0;

    if (GetAsyncKeyState('W') & 0x8000) forward += 1;
    if (GetAsyncKeyState('S') & 0x8000) forward -= 1;
    if (GetAsyncKeyState('D') & 0x8000) right += 1;
    if (GetAsyncKeyState('A') & 0x8000) right -= 1;
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) jump = 1;

    Camera_Moving(forward, right, jump);

    if(mouseBind)
        Camera_Mouse(400, 400, 0.2);

    if (!camera.isJumping)
        camera.z = Map_Obtain_Height(camera.x, camera.y) + 1.7;
}

void Player_Lifting_Object(HWND hwnd)
{
    selectingMode = TRUE;
    Map_Present();
    selectingMode = FALSE;

    RECT rectangle;
    GLubyte color[3];
    GetClientRect(hwnd, &rectangle);
    glReadPixels(rectangle.right / 2.0, rectangle.bottom / 2.0, 1, 1, GL_RGB,
                 GL_UNSIGNED_BYTE,color);

    if(color[1] > 0)
    {
        for(int i = 0; i < selecting_object_counter; i++)
            if(select_object[i].color_index == color[1])
            {
                Animation_Setup(&animation,plant_array + select_object[i].planted_object_index);

                // play a sound
                PlayPickupSound();
            }
    }


}


void Windows_Resize(int x, int y)
{
    glViewport(0, 0, x, y);
    screenSize.x = x;
    screenSize.y = y;

    screenKoef = x / (float)y;
    float size = 0.1;

    ///
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-screenKoef * size, screenKoef * size, -size, size, size * 2, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ///

}

void Slot_Present(int x, int y, int scale)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, slotRectangle);
    glTexCoordPointer(2, GL_FLOAT, 0, slotRectangleUV);

    for(int i = 0; i < SlotSize; i++)
    {
        glPushMatrix();
            glTranslatef(x + i * scale, y, 0);
            glScalef(scale, scale, 1);
            glColor3ub(61, 61, 41);
            glDisable(GL_TEXTURE_2D);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            if (i == selectedSlotIndex)
            {
            glColor4ub(255, 255, 255, 100);  // soft white
            glLineWidth(3);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
            }

            if(slot[i].type > 0)
            {
                glColor3f(1,1,1);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, slot[i].type);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }

            glColor3ub(160, 160, 160);
            glLineWidth(3);
            glDisable(GL_TEXTURE_2D);
            glDrawArrays(GL_LINE_LOOP, 0, 4);

        glPopMatrix();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_DEPTH_TEST);
}

void DrawHotbar() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int totalHotbarWidth = SlotSize * SLOT_SIZE;

    int totalWidth = SlotSize * SLOT_SIZE;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    int startY = SCREEN_HEIGHT - SLOT_SIZE - 20; // 20 px from bottom

    Slot_Present(startX, startY, SLOT_SIZE);
}

void Slot_Selecting(int x, int y, int scale, int mx, int my)
{
    if((my < y) || (my > y + scale)) return;

    for(int i = 0; i < SlotSize; i++)
    {
        if((mx > x + i * scale) && (mx < x + (i + 1) * scale))
        {
            if(slot[i].type == texture_mushroom)
            {
                health++;
                    if(health > health_maximum)
                        health = health_maximum;
            }
            slot[i].type = -1;
        }
    }
}

void Health_Present(int x, int y, int scale)
{
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, heart_shape);
        for(int i = 0; i < health_maximum; i++)
        {
            glPushMatrix();
                glTranslatef(x + i * scale, y, 0);
                glScalef(scale, scale, 1);
                if(i < health)
                    glColor3f(1,0,0);
                else
                    glColor3f(0,0,0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
            glPopMatrix();
        }
    glDisableClientState(GL_VERTEX_ARRAY);
}


void Menu_Present()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenSize.x, screenSize.y, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    Slot_Present(540,790,50);
    Health_Present(540, 750, 30);
    //DrawHotbar();
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_POPUP | WS_VISIBLE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          SCREEN_WIDTH,
                          SCREEN_HEIGHT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    //WS_OVERLAPPEDWINDOW  WS_POPUP | WS_VISIBLE
    ShowWindow(hwnd, nCmdShow);
    SetCursor(wcex.hCursor);
    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    RECT rectangle;
    GetClientRect(hwnd, &rectangle);
    Windows_Resize(rectangle.right, rectangle.bottom);
    Map_Create();

    glEnable(GL_DEPTH_TEST);

    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }

    InitAudio();

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            if(GetForegroundWindow() == hwnd)
                    Player_Move();

            Map_Present();
            Menu_Present();

            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

     CloseAudio();
     SDL_Quit();

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_LBUTTONDOWN:
            if(mouseBind)
                Player_Lifting_Object(hwnd);
            else
                Slot_Selecting(540,790,50, LOWORD(lParam), HIWORD(lParam));
        break;

        case WM_SIZE:
            Windows_Resize(LOWORD(lParam), HIWORD(lParam));
        break;

        case WM_SETCURSOR:
            ShowCursor(!mouseBind);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;

                case 'E':
                    mouseBind = !mouseBind;

                        if(mouseBind)
                        {

                            while (ShowCursor(FALSE) >= 0);
                            SetCapture(hwnd);
                            SetCursor(NULL);
                        }
                        else
                        {

                            while (ShowCursor(TRUE) <= 0);
                            ReleaseCapture();
                            SetCursor(LoadCursor(NULL, IDC_ARROW));
                        }


                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
