/* date = November 26th 2021 3:48 pm */

#ifndef _LONG_AUDIO_H
#define _LONG_AUDIO_H

typedef struct Sound Sound;
struct Sound
{
    i16* sample;
    u32 sampleCount;
};

typedef struct PlayingSound PlayingSound;
struct PlayingSound
{
    Sound sound;
    f32 volume;
    f32 desiredVolume;
    f32 dVolume;
    f32 pitch;
    u32 samplesPlayed;
};

typedef struct SoundHandle SoundHandle;
struct SoundHandle
{
    u32 generation;
    u32 sound;
};

Sound LoadSound(char* soundName);
SoundHandle PlaySound(Sound sound);
SoundHandle PlaySoundEx(Sound sound, f32 volume, f32 pitch, bool looping);
SoundHandle PlaySoundRange(Sound sound, f32 startTime, f32 duration);
SoundHandle PlaySoundRangeEx(Sound sound, f32 startTime, f32 duration, f32 volume, f32 pitch, bool looping);

bool IsSoundHandleValid(SoundHandle sound);
PlayingSound* GetPlayingSound(SoundHandle sound);

void StopSound(SoundHandle sound);
void StopAllSounds(void);

bool IsPaused(SoundHandle sound);
void SetPaused(SoundHandle sound, bool paused);

bool IsLooping(SoundHandle sound);
void SetLooping(SoundHandle sound, bool looping);

#endif //_LONG_AUDIO_H
