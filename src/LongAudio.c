#include "LongAudio.h"

#pragma pack(push, 1)
typedef struct
{
    u32 riffID;
    u32 size;
    u32 waveID;
} WAVEHeader;

#define RIFF_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

enum
{
    WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
    WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};

typedef struct
{
    u32 id;
    u32 size;
} WAVEChunk;

typedef struct
{
    u16 wFormatTag;
    u16 nChannels;
    u32 nSamplesPerSec;
    u32 nAvgBytesPerSec;
    u16 nBlockAlign;
    u16 wBitsPerSample;
    u16 cbSize;
    u16 wValidBitsPerSample;
    u32 dwChannelMask;
    u8 subFormat[16];
} WAVEfmt;

#pragma pack(pop)

typedef struct
{
    u8* at;
    u8* stop;
} RiffIterator;

RiffIterator ParseChunkAt(void* at, void* stop)
{
    RiffIterator iterator;
    
    iterator.at = (u8*)at;
    iterator.stop = (u8*)stop;
    
    return iterator;
}

RiffIterator NextChunk(RiffIterator iterator)
{
    WAVEChunk* chunk = (WAVEChunk*)iterator.at;
    u32 size = (chunk->size + 1) & ~1;
    iterator.at += sizeof(WAVEChunk) + size;
    return iterator;
}

bool IsValid(RiffIterator iterator)
{
    return (iterator.at < iterator.stop);
}

void* GetChunkData(RiffIterator iterator)
{
    return (iterator.at + sizeof(WAVEChunk));
}

u32 GetType(RiffIterator iterator)
{
    WAVEChunk* chunk = (WAVEChunk*)iterator.at;
    return chunk->id;
}

u32 GetChunkDataSize(RiffIterator iterator)
{
    WAVEChunk* chunk = (WAVEChunk*)iterator.at;
    return chunk->size;
}

Sound LoadSound(char* soundName)
{
    Sound result = {0};
    Buffer buffer = LoadData(soundName);
    
    if (buffer.data && buffer.count)
    {
        WAVEHeader* header = (WAVEHeader*)buffer.data;
        assert(header->riffID == WAVE_ChunkID_RIFF);
        assert(header->waveID == WAVE_ChunkID_WAVE);
        
        u32 channelCount = 0;
        u32 sampleDataSize = 0;
        i16* sampleData = 0;
        
        for (RiffIterator iterator = ParseChunkAt(header + 1, (u8*)(header + 1) + header->size - 4); IsValid(iterator); iterator = NextChunk(iterator))
        {
            switch (GetType(iterator))
            {
                case WAVE_ChunkID_fmt:
                {
                    WAVEfmt* fmt = (WAVEfmt*)GetChunkData(iterator);
                    assert(fmt->wFormatTag == 1); // NOTE: Only support PCM
                    assert(fmt->nSamplesPerSec == 48000);
                    assert(fmt->wBitsPerSample == 16);
                    assert(fmt->nBlockAlign == sizeof(i16)*fmt->nChannels);
                    channelCount = fmt->nChannels;
                } break;
                case WAVE_ChunkID_data:
                {
                    sampleData = (i16*)GetChunkData(iterator);
                    sampleDataSize = GetChunkDataSize(iterator);
                } break;
            }
        }
        
        assert(channelCount && sampleData && sampleDataSize);
        assert(channelCount < 3);
        
        result.sample = sampleData;
        result.sampleCount = sampleDataSize / (channelCount*sizeof(i16));
    }
    
    return result;
}

enum
{
    SOUND_GENERATION,
    SOUND_LOOPING,
    SOUND_PAUSING,
    SOUND_VALID,
};

#define MAX_SOUNDS 1024
typedef struct PlayingSoundSlot PlayingSoundSlot;
struct PlayingSoundSlot
{
    u32 generation;
    union
    {
        PlayingSound sound;
        u32 next;
    };
};
static PlayingSoundSlot allSounds[MAX_SOUNDS];

SoundHandle PlaySound(Sound sound)
{
    SoundHandle result = PlaySoundEx(sound, 1, 1, false);
    return result;
}

SoundHandle PlaySoundEx(Sound sound, f32 volume, f32 pitch, bool looping)
{
    SoundHandle result = {0};
    
    if ((sound.sampleCount > 0) && (sound.sample))
    {
        u32 next = allSounds[0].next;
        if (next)
        {
            allSounds[0].next = allSounds[next].next;
            allSounds[next].sound = (PlayingSound){
                .sound = sound,
                .volume = volume,
                .pitch = pitch,
            };
            
            allSounds[next].generation |= SOUND_LOOPING|SOUND_PAUSING|SOUND_VALID;
            result = (SoundHandle){
                allSounds[next].generation << SOUND_GENERATION,
                next
            };
        }
    }
    
    return result;
}

bool IsSoundHandleValid(SoundHandle sound)
{
    bool result = ((sound.sound) &&
                   (sound.sound < MAX_SOUNDS) &&
                   ((allSounds[sound.sound].generation << SOUND_GENERATION) == sound.generation) &&
                   (allSounds[sound.sound].generation & SOUND_VALID));
    return result;
}

PlayingSound* GetPlayingSound(SoundHandle sound)
{
    if (IsSoundHandleValid(sound))
        return &allSounds[sound.sound].sound;
    return 0;
}

void StopSound(SoundHandle sound)
{
    if (IsSoundHandleValid(sound))
    {
        allSounds[sound.sound].generation &=  ~SOUND_VALID;
    }
}

void StopAllSounds(void)
{
    // TODO: Reset allSounds
}

bool IsPaused(SoundHandle sound)
{
    bool result = false;
    
    if (IsSoundHandleValid(sound))
        result = allSounds[sound.sound].generation & SOUND_PAUSING;
    
    return result;
}

void SetPaused(SoundHandle sound, bool paused)
{
    if (IsSoundHandleValid(sound))
        allSounds[sound.sound].generation = (allSounds[sound.sound].generation & ~SOUND_PAUSING) | SOUND_PAUSING;
}

bool IsLooping(SoundHandle sound)
{
    if (IsSoundHandleValid(sound))
        return allSounds[sound.sound].generation & SOUND_LOOPING;
    return false;
}

void SetLooping(SoundHandle sound, bool looping)
{
    if (IsSoundHandleValid(sound))
        allSounds[sound.sound].generation = (allSounds[sound.sound].generation & ~SOUND_LOOPING) | looping;
}