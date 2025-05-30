#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

typedef struct
{
    float x, y, z;

}TCell;

typedef struct
{
    float red, green, blue;

} TColor;

typedef struct
{
    float u,v;

} TUV;

typedef struct
{
    float x, y, z;
    int type;
    float scale;

} TObject;

/// T = triangle, UV = texture coordinates

typedef struct
{
    TObject *stuff;
    int stuff_counter;
    int type; // type of tree

} TObjectGroup;

#define map_width 100
#define map_height 100

TCell map[map_width][map_height];
//TColor mapImproved[map_width][map_height];
TCell mapImproved[map_width][map_height];
TUV mapUV[map_width][map_height];

GLuint mapIndex[map_width - 1][map_height - 1][6];

int mapIndexCount = sizeof(mapIndex) / sizeof(GLuint);

/// coordinates for objects

    float plant[] = {-0.5,0,0, 0.5,0,0, 0.5,0,1, -0.5,0,1,
                    0, -0.5,0, 0, 0.5,0, 0,0.5,1, 0,-0.5,1};
    float plantUV[] = {0,1, 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0};
    GLuint plant_index[] = {0,1,2, 2,3,0, 4,5,6, 6,7,4};

    int plant_index_counter = sizeof(plant_index) / sizeof(GLuint);


/// coordinates for objects


int texture_flower, texture_flower2, texture_grass, texture_ground, texture_mushroom, texture_tree, texture_tree2;

TObject *plant_array = NULL;
int plant_counter = 0;

int texture_wood;
float cube_tree[] = {0,0,0, 1,0,0, 1,1,0, 0,1,0,
                     0,0,1, 1,0,1, 1,1,1, 0,1,1,
                     /// for other texture coordinates
                     0,0,0, 1,0,0, 1,1,0, 0,1,0,
                     0,0,1, 1,0,1, 1,1,1, 0,1,1};

float cube_tree_UV_trunk[] = {0.5,0.5, 1,0.5, 1,0, 0.5,0,
                      0.5,0.5, 1,0.5, 1,0, 0.5,0,
                      0,0.5, 0.5,0.5, 0,0.5, 0.5,0.5,
                      0,0,   0.5,0,   0,0,   0.5,0};

float cube_tree_UV_leaves[] = {0,1,   0.5,1,   0.5,0.5, 0,0.5,
                               0,1,   0.5,1,   0.5,0.5, 0,0.5,
                               0,0.5, 0.5,0.5, 0,0.5,   0.5,0.5,
                               0,1,   0.5,1,   0,1,     0.5,1};

GLuint cube_tree_Index[] = {0,1,2,   2,3,0,    4,5,6,    6,7,4,    8,9,13,   13,12,8,
                            9,10,14, 14,13,9,  10,11,15, 15,14,10, 11,8,12,  12,15,11};

int cube_tree_counter = sizeof(cube_tree_Index) / sizeof(GLuint);

TObjectGroup *tree = NULL;
int tree_counter = 0;

float sun_shape[] = {-1,-1,0, 1,-1,0, 1,1,0, -1,1,0};

BOOL selectingMode = FALSE;

#define ObjectListCount 255
typedef struct{
    int planted_object_index;
    int color_index;
} TSelectingObject;

TSelectingObject select_object[ObjectListCount];
int selecting_object_counter = 0;

//  Animation code starts here
///
    typedef struct{
        TObject *obj;
        float dx, dy, dz;
        int counter;
    } TInteraction;

TInteraction animation = {0, 0, 0, 0, 0};
///

//  Interface code starts here
///

POINT screenSize;
float screenKoef;

///

//  Free-slots
///

typedef struct{
    int type;
}TSlot;

// 16
#define SlotSize 9
TSlot slot[SlotSize];

float slotRectangle[]   = {0,0, 1,0, 1,1, 0,1};
float slotRectangleUV[] = {0,0, 1,0, 1,1, 0,1};

int selectedSlotIndex = 0;
///

///
#define SLOT_SIZE 50
///

// Health bars
///
int health = 9;
int health_maximum = 15;

float heart_shape[] = {0.5,0.25, 0.25,0, 0,0.25, 0.5,1, 1,0.25, 0.75,0};

BOOL mouseBind = TRUE;

typedef struct{
    int time;
    int time_maximum;
} TBuff;

struct{
    TBuff speed;
    TBuff eye;

} buffs = {0,0,0,0};

int icon_running, icon_search;

float Map_Obtain_Height(float x, float y);


#endif // MAIN_H_INCLUDED
