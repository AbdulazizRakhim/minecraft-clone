#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

#include "SDL_mixer.h"

void InitAudio();
void CloseAudio();

void PlayWalkSound();
void PlaySprintSound();
void StopFootstepSound();
void PlayLandSound();
void PlayPickupSound();

#endif // SOUND_H_INCLUDED
