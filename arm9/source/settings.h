#pragma once

extern u32 gEmuSettingUseBottomScreen;
//extern u32 gEmuSettingAutoSave;
extern u32 gEmuSettingUseSavesDir;
extern u32 gEmuSettingCenterMask;
extern u32 gEmuSettingFrame;
extern u32 gEmuSettingMainMemICache;
extern u32 gEmuSettingWramICache;
extern u32 gEmuSettingGbaColors;
extern u32 gEmuSettingSkipIntro;

struct settings_input
{
    u32 buttonA;
    u32 buttonB;
    u32 buttonL;
    u32 buttonR;
    u32 buttonStart;
    u32 buttonSelect;
};

extern settings_input gInputSettings;

void settings_initialize();
bool settings_save();