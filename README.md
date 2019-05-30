# This fork is just for testing/experimenting.
Also most of the new things here are spaghetti-code quality. ![:slowpoke:](https://cdn.discordapp.com/emojis/531734661865668628.png)


[Download binaries here (updated every git commit) ](https://ci.appveyor.com/api/projects/lybxlpsv/TotallyLegitArcadeController/artifacts/release.zip?branch=experimental) [![Build status](https://ci.appveyor.com/api/projects/status/github/lybxlpsv/totallylegitarcadecontroller?branch=experimental&svg=true)](https://ci.appveyor.com/project/lybxlpsv/totallylegitarcadecontroller?branch=experimental)

[Download DVA version here (updated every git commit) ](https://ci.appveyor.com/api/projects/lybxlpsv/TotallyLegitArcadeController/artifacts/release-dva.zip?branch=experimental)(for [DIVA Loader](https://github.com/Rayduxz/DIVA-Loader)) [![Build status](https://ci.appveyor.com/api/projects/status/github/lybxlpsv/totallylegitarcadecontroller?branch=experimental&svg=true)](https://ci.appveyor.com/project/lybxlpsv/totallylegitarcadecontroller?branch=experimental)

Any contributions are welcome, including those not on our todo list.
Currently added features:
 * framebuffer scaling
 * in-game overlay
 * arbitrary resolution

# Description
This program consists of three parts:
##### Prepatch:
Used for applying the static patches from the `patch.txt` file to allow the game to successfully boot past all startup checks.
##### Divahook:
Installs a hook on the game's update routine to update its components.
##### TLAC:
Injects the divahook dll inside the game's process.

## Components
The available components can be toggled inside the `components.ini` config file. Currently available are:
* input_emulator
* touch_slider_emulator
* touch_panel_emulator
* sys_timer
* player_data_manager
* frame_rate_manager
* stage_manager
* fast_loader
* camera_controller
* debug_component

## Controls
Input bindings can be set inside the `keyconfig.ini` config file. Currently supported are keyboard, DualShock 4, and XInput controller input. To use a PlayStation 1/2/3 controller, install ScpToolkit.
Key and button names are listed inside this config.

## Player Data
If enabled the PlayerDataManager writes constant values from the `playerdata.ini` config file to the game's memory.
This allows the user to set their player name, a level plate ID to be shown during the song select, equip a skin to be loaded during gameplay and button / slide sounds to be applied to all songs played.
This config file **must be** encoded using UTF-8.

## Debug Component
This component enables diva's internal dw_gui and makes the hidden DATA_TESTs accessible.
The current game state can be changed using the F4 - F8 keys with F6 being a recreation of the original DATA_TEST_MAIN inside the console which can be controlled using the up / down arrow + enter key.

# Usage
First **once per update** patch a clean `diva.exe` using the `prepatch.exe` program by passing the `diva.exe` path to it as its first command line argument; this can be done by dragging the diva executable on top of the prepatch executable inside the Windows file explorer. It is strongly recommended to always create a backup of the original diva executable first.
**NOTE: The path to the game folder must not contain non-Unicode characters, such as Chinese ones.**

To start DIVA run `diva.exe` like normal, to enable the components start `tlac.exe` **while** diva is running.
Do this **every time** you wish to run diva.
Alternatively, run tlac.exe "diva.exe" as the first argument.

DIVA has a few notable command line arguments including `-w` to start in windowed mode and `-hdtv1080` to increase the resolution to 1920x1080.

All possible resolution arguments:
* `-qvga`    hidden, probably 320x240
* `-vga`    VGA(640x480) Mode
* `-wvga`    hidden, probably 800x480
* `-svga`    hidden, probably 800x600
* `-xga`    XGA(1024x768) Mode
* `-hdtv720`    hidden, probably 1280x720
* `-hdtv720_dbd`    hidden, probably 1280x720 dot by dot mode
* `-wxga`    WXGA(1280x768) Mode
* `-wxga_dbd`    WXGA(1280x768) dot by dot Mode
* `-wxga2`    WXGA(1360x768) Mode
* `-uxga`    hidden, probably 1600x1200
* `-hdtv1080`    hidden, probably 1920x1080
* `-wuxga`    hidden, probably 1920x1200
* `-wqhd`    hidden, probably 2560x1440
* `-wqxga`    hidden, probably 2560x1600

# Arbitrary Resolution
Open "graphics.ini" and set maxWidth and maxHeight. Set customRes to true.
Use "diva_customRes.bat"! Do not run the exe directly and then open TLAC!
It should run with the arbitrary resolution you've set.
You must set customRes to false to be able to run the exe directly.
To disable anti aliasing completely, edit "diva_customRes.bat" and add the argument `-no_aa`.
