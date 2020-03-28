#pragma once

#define DSP_DSP1_MAGIC                  0x31505344

enum DspDsp1SegmentType : u8
{
    DSP_DSP1_SEG_TYPE_PROG0 = 0,
    DSP_DSP1_SEG_TYPE_PROG1 = 1,
    DSP_DSP1_SEG_TYPE_DATA = 2
};

#define DSP_DSP1_FLAG_SYNC_LOAD         1
#define DSP_DSP1_FLAG_LOAD_FILTER_SEG   2

struct dsp_dsp1_header_t
{
    u8 rsaSignature[0x100];
    u32 magic;
    u32 fileSize;
    u16 memoryLayout;
    u16 padding;
    u8 unknown;
    DspDsp1SegmentType filterSegType;
    u8 nrSegments;
    u8 flags;
    u32 filterSegAddr;
    u32 filterSegSize;
    u64 zero;
};

static_assert(sizeof(dsp_dsp1_header_t) == 0x120);

struct dsp_dsp1_segment_t
{
    u32 offset;
    u32 address;
    u32 size;
    u8 padding[3];
    DspDsp1SegmentType segmentType;
    u8 sha256[32];
};

static_assert(sizeof(dsp_dsp1_segment_t) == 0x30);

struct dsp_dsp1_t
{
    dsp_dsp1_header_t header;
    dsp_dsp1_segment_t segments[10];
};