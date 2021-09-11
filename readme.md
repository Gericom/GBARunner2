GBARunner2
===================
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RSVWHQ3XR2UX6)

GBARunner2 is a hypervisor that runs GBA games on DS/DSi/3DS in DS mode.
## Usage Notes
- If you have a gba folder on the root of your sd, this folder will be opened by default
- Games generally **don't** need to be SRAM patched unless stated in the compatibility list (see below). If it is needed, you can use [gbata](http://www.no-intro.org/gbadat/tools/gbata7a-en.zip).
- If you are using GBARunner2 with TWiLightMenu on a DSi or 3DS with the SD card, make sure you use the dldi on ARM7 build
- The settings are accessed by **pressing R** on the rom selection menu. Settings are saved when leaving the settings menu

GBARunner2 has the open source bios of Normmatt integrated, with some small modifications. A GBA bios is thus optional, but can be used to improve compatibility with some games. Always try with a valid GBA bios before reporting that something doesn't work! Compatibility testing should as such also be done with a bios. You can put the bios in one of the following places: /bios.bin, /gba/bios.bin or /_gba/bios.bin.

### Bios checksums
A valid bios should have the following checksums:
- CRC32: `81977335`
- MD5: `a860e8c0b6d573d191e4ec7db1b1e4f6`
- SHA1: `300c20df6731a33952ded8c436f7f186d25d3492`
- SHA256: `fd2547724b505f487e6dcb29ec2ecff3af35a841a77ab2e85fd87350abd36570`

## Compatibility List
A compatibility list can be found here:
- For DS/DSLite or with flashcards on other consoles: https://wiki.gbatemp.net/wiki/GBARunner2
- For DSi/3DS when ran in DSi mode: https://wiki.gbatemp.net/wiki/GBARunner2/DSi_3DS_Compatibility_List

Many thanks to Dodain47, Zense, Dartz and all other testers for investing so much time in testing games!

## DSP Audio
GBARunner2 now has support for DSP audio on DSi and 3DS through a custom DSP binary. The compiler is a fork of llvm with a teak backend I made. It can be found here: https://github.com/Gericom/teak-llvm

I want to thank wwylele, Normmatt and nocash (please let me know if there was anyone involved I missed) for all the DSP research they did. Without them it would not have been possible!

## Libraries Used
- [FatFS](http://elm-chan.org/fsw/ff/00index_e.html)

## Donation
**GBARunner2 will always be free software and you'll never have to pay to use it!**

If you however like this project and you want to thank me or support me in continuing this project, you can donate using the button below.
There's no minimum amount, I appreciate any small gift.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RSVWHQ3XR2UX6)