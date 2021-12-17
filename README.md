# Two Tribes Engine

We, [Two Tribes](https://twotribes.com), have used this engine for over a decade. The engine was initially conceived as a Nintendo DS game engine when creating games such as [Worms Open Warfare 2](https://en.wikipedia.org/wiki/Worms:_Open_Warfare_2). It has since been used on many games for Nintendo Wii, Playstation 4 and PC among others.

It offers basic 3D functionality, but the main focus for Two Tribes has always been 2D games, so this is where it really shines! Our games always strived to run at 60 frames per second while offering great layered 2D visuals.

We decided to release the PC version of the engine on Github (see license below) and can't wait to see what other developers are able to create with it!

For an example of what can be created, feel free to watch this video of our last game [RIVE](https://rivethegame.com) in action: https://youtu.be/0fVr3bBKJzY?t=33

## Test Level

Included in this repository is a fully working test level of RIVE. It demonstrates the capabilities of the engine. Feel free to experiment with it, change it or make something with it yourself.

While the test level works without any issues, please do note that there is no guarantee that everything will work under different circumstances. Only a small part of the RIVE assets are included in this distribution. So there is a high chance that features are missing, or are generating asserts when fiddling around with different settings. If that happens, feel free to open an Issue on github and we might be able to help out. But as this software is provided "as-is", it might take a while before we are able to help out.

# Licenses

## Engine License

The engine is licensed under the [GNU GPLv2 license](https://www.gnu.org/licenses/gpl-2.0-standalone.html). A copy of the license is included in the repository. Should you require a different license because of the nature of your project, feel free to contact us.

## Assets License

All assets contained in the `.\assets` folder are copyrighted. All rights are reserved. You are allowed to personally use them to experiment with the engine and discover its possibilities.

Without our written permission you are not allowed to use them (nor any derivate work) in a product (free or commercially). Nor are you allowed to redistribute them in any way.

# Directory Structure

```
├── assets
│   ├── conversion              -> contains platform conversion scripts
│   │   ├── config              -> contains main conversion scripts
│   │   ├── intermediate        -> temp directory for asset conversion
│   ├── output                  -> will contain converted assets
│   ├── source
│   │   ├── shared              -> source assets shared across all platforms
│   │   ├── win                 -> windows only source assets
│   ├── LICENSE                 -> the more restrictive license that applies to the assets
│   ├── scripts.code-workspace  -> the script VS Code workspace file
│   ├── windows.xml             -> asset database for the windows version of the game
├── build
│   ├── win                     -> contains the windows executable build
├── config                      -> contains various configuration files for asset conversion and gamelauncher
├── src
│   ├── engine                  -> contains the Two Tribes engine. The main engine upon which our games were built
│   ├── game                    -> contains game specific code. This code was used in our latest projects: Toki Tori 2+ and RIVE.
│   ├── ttgame.sln              -> the Visual Studio 2017 solution
├── tools                       -> contains various tools required for the asset conversion
└── .gitattributes
└── .gitignore
└── LICENSE                     -> a copy of the GNU GPLv2 license, under which this engine, except for the assets, is licensed
└── README.md                   -> this readme file
```

## Spaces and dots

Ensure the entire path does not contain any spaces nor dots. Some asset converters don't work well with those. Especially take care of that when downloading a release zipfile from github, as those zip filenames can contain dots.

## Virus & Threat Protection

After downloading this repository Windows Defender might interfere with asset conversion, as it can flag asset conversion executables for a threat scan. When that happens the conversion might stall for a while, waiting for Windows Defender to scan the executable. Asset conversion might not resume after such a stall, resulting in a permanent stall.

In that case simply run the conversion multiple times, until it is finished. After a successful run, Windows Defender has flagged the asset conversion executables as safe and a conversion stall won't happen again.

# Designers Instructions

## Installing RIVE font

First ensure to install the `RIVE.ttf` font located in `.\assets\fonts`, otherwise the conversion step will fail.

## Converting assets

All converters run on Windows. To convert all assets, simply run `.\assets\conversion\convert_win.bat` or `.\assets\conversion\convert_win_rebuild.bat`

Alternatively you can convert assets by using the TTTray and/or TTAssetMonitor applications.

## TT Tray

Two Tribes developed a small tray app that you can use to easily convert assets or boot up the game.

To use it first register the project configuration by running: `.\config\register_tray.bat`. This registers the information contained in `.\config\ttgame.tttray` with the `TTTray` application.

The TTTray application itself is located in `.\tools\tttray`.

To exit the app or reload the configuration file(s), hold shift while clicking on the tray icon and then select the now visible options.

## TT Asset Monitor

In addition to the TTTray application, Two Tribes developed a file watcher that can quickly convert modified assets. The game can load some modified assets on the fly, or by pressing `F5` ingame. That way you never need to close and reopen the game to see your changes.

To use it first register the project configuration by running: `.\config\register_filewatcher.bat`. This registers the information contained in `.\config\ttgame.ttassmon` with the `TTAssetMonitor` application.

The TTAssetMonitor application itself is located in `.\tools\ttassetmonitor`.

To exit the app, hold shift while clicking on the tray icon and then select the now visible exit option.

## Scripting

Most of the high level features of the game are programmed in [Squirrel](http://www.squirrel-lang.org). Just open the Visual Studio Code project in `.\assets\source\` to see what's possible. There are `nut` file extensions in the VS Code Marketplace to help you with syntax highlighting.

### Prerequisites

Any text editor would do fine, but VS Code is highly recommended.

# Coders Instructions

# Prerequisites

## Visual Studio 2017

-   Get and install VS 2017:

https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2017-and-other-products

## DirectX June 2010 SDK

-   Get the following DirectX SDK:

https://www.microsoft.com/en-us/download/details.aspx?id=6812

-   Only "Install DirectX Runtime" and "DirectX Headers and Libs" are required for installation. Ensure to uninstall any previous 2010 runtime version of DirectX, otherwise the installation might end up in a `S1023` error

-   Verify that the `DXSDK_DIR` environment variable is set. This should've been done by the installer.

### Modify property sheets

Now we need to let Visual Studio know the location of this DirectX version.

-   Go to `C:\Users\<username>\AppData\Local\Microsoft\MSBuild\v4.0`

-   Open `Microsoft.Cpp.x64.user.props`

-   Append `$(DXSDK_DIR)Include;` to `IncludePath`. Be sure to append at the END of the string.

-   Append `$(DXSDK_DIR)Lib\x64;` to `LibraryPath`. Be sure to append at the END of the string.

`Microsoft.Cpp.x64.user.props` now looks something like this:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets">
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <IncludePath>$(VC_IncludePath);$(WindowsSDK_IncludePath);$(DXSDK_DIR)Include;</IncludePath>
    <LibraryPath>$(VC_LibraryPath_x64);$(WindowsSDK_LibraryPath_x64);$(DXSDK_DIR)Lib;</LibraryPath>
  </PropertyGroup>
  <ItemDefinitionGroup />
  <ItemGroup />
</Project>
```

### Win32

Win32 support in the engine has been deprecated.

## Windows SDK

-   Go to https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/

-   Locate and install Windows 10 SDK, version 1809 (10.0.17763.0). Newer versions can be used too, but then you need to retarget the SDK version of all projects.
