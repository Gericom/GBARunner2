GBARunner2
===================
GBARunner2 is a hypervisor that runs GBA games on DS/DSi/3DS in DS mode.
## Usage Notes
- Place a GBA bios on your sd card. Either /bios.bin, /gba/bios.bin or /_gba/bios.bin will work.
- If you have a gba folder on the root of your sd, this folder will be opened by default
- Make sure your games are SRAM patched if needed, this is most likely only the case for FLASH1M_V103 (use [gbata](http://www.no-intro.org/gbadat/tools/gbata7a-en.zip))
- If you are using GBARunner2 with TWiLightMenu on a DSi or 3DS with the SD card, make sure you use the dldi on ARM7 build
- The settings are accessed by pressing R on the rom selection menu. Settings are saved when leaving the settings menu

### Bios checksums
A valid bios should have the following checksums:
- CRC32: `81977335`
- MD5: `a860e8c0b6d573d191e4ec7db1b1e4f6`
- SHA1: `300c20df6731a33952ded8c436f7f186d25d3492`
- SHA256: `fd2547724b505f487e6dcb29ec2ecff3af35a841a77ab2e85fd87350abd36570`

## Libraries Used
- [FatFS](http://elm-chan.org/fsw/ff/00index_e.html)
