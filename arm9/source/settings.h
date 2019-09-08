#pragma once

extern u32 gEmuSettingUseBottomScreen;
//extern u32 gEmuSettingAutoSave;
extern u32 gEmuSettingCenterMask;
extern u32 gEmuSettingMainMemICache;
//extern u32 gEmuSettingWramICache;
extern u32 gEmuSettingSkipIntro;

void settings_initialize();
bool settings_save();