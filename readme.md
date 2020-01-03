# GBARunner2
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RSVWHQ3XR2UX6)

GBARunner2 is a hypervisor that runs GBA games on DS/DSi/3DS in DS mode.

## Usage Notes
  * Place a GBA BIOS on your SD card. Either `/bios.bin`, `/gba/bios.bin` or `/_gba/bios.bin` will work.
    * If you have a `gba` folder on the root of your SD card it will be used by default.
  * Most games should run without patches with the latest versions, but make sure your games are SRAM patched with [GBATA](https://www.no-intro.org/gbadat/tools/gbata7a-en.zip) if necessary.
  * If you are using GBARunner2 with TWiLightMenu on a DSi or 3DS with the SD card, make sure you use the dldi on ARM7 build.
  * The settings are accessed by pressing R on the rom selection menu. Settings are saved when leaving the settings menu.

### Valid BIOS checksums:
|        | A valid BIOS should have the following checksums: |
| ------ | :------------------------------------------------ |
| CRC32  | `81977335`
| MD5    | `a860e8c0b6d573d191e4ec7db1b1e4f6`
| SHA1   | `300c20df6731a33952ded8c436f7f186d25d3492`
| SHA256 | `fd2547724b505f487e6dcb29ec2ecff3af35a841a77ab2e85fd87350abd36570`

## Libraries Used
  * [FatFS](http://elm-chan.org/fsw/ff/00index_e.html)

## Donation
**GBARunner2 will always be free software and you'll never have to pay to use it!**

If you like this project and want to thank me or support me in continuing this project, you can donate using the button below.
There's no minimum amount, I appreciate any small gift.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RSVWHQ3XR2UX6)
