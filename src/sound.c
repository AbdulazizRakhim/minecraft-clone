#include "sound.h"
#include "stdio.h"
#define LAND_CHANNEL 2

Mix_Chunk* pickupSound = NULL;
Mix_Chunk* walkSound = NULL;
Mix_Chunk* sprintSound = NULL;
Mix_Chunk* landSound = NULL;

const int FOOTSTEP_CHANNEL = 1;

void InitAudio() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer error: %s\n", Mix_GetError());
        return;
    }

    Mix_AllocateChannels(16);

    walkSound   = Mix_LoadWAV("assets/sounds/minecraft-walking.wav");
    sprintSound = Mix_LoadWAV("assets/sounds/minecraft-sprinting.wav");
    pickupSound = Mix_LoadWAV("assets/sounds/minecraft-item-pickup-sound-effect.wav");
    landSound   = Mix_LoadWAV("assets/sounds/minecraft-fall-damage-crack.wav");

    if (!walkSound || !sprintSound || !pickupSound || !landSound) {
            printf("Sound loading error: %s\n", Mix_GetError());
        }
}

void CloseAudio()
{
    Mix_FreeChunk(walkSound);
    Mix_FreeChunk(sprintSound);
    Mix_FreeChunk(pickupSound);
    Mix_FreeChunk(landSound);
    Mix_CloseAudio();
}

void PlayWalkSound() {
    Mix_PlayChannel(1, walkSound, -1); // loop
}

void PlaySprintSound() {
    Mix_PlayChannel(1, sprintSound, -1); // loop
}

void StopFootstepSound(){
    Mix_HaltChannel(1);
}

void PlayPickupSound() {
    Mix_PlayChannel(-1, pickupSound, 0);
}

void PlayLandSound() {
    Mix_HaltChannel(LAND_CHANNEL); // stop if anything was on it
    Mix_PlayChannel(LAND_CHANNEL, landSound, 0);
}
