# GUI sample

![GitHub commit activity](https://img.shields.io/github/commit-activity/m/Core-2-Extreme/GUI_sample?color=darkgreen&style=flat-square)

## Index
* [What is this?????](https://github.com/Core-2-Extreme/GUI_sample#what-is-this)
* [What sample code does it have?????](https://github.com/Core-2-Extreme/GUI_sample#what-sample-code-does-it-have)
* [Build](https://github.com/Core-2-Extreme/GUI_sample#build)
* [License](https://github.com/Core-2-Extreme/GUI_sample#license)

## What is this?????
This is a sample application for 3ds homebrew (aka SDK). \
Homebrew application for 3ds such as [Video player for 3DS](https://github.com/Core-2-Extreme/Video_player_for_3DS) and [Battery mark for 3DS](https://github.com/Core-2-Extreme/Battery_mark_for_3DS) are based on this sample code.

## What sample code does it have?????
Currently this sample application contains following samples : 
* [Draw image sample](https://github.com/Core-2-Extreme/GUI_sample#draw-image-sample)
* [File explorer sample](https://github.com/Core-2-Extreme/GUI_sample#file-explorer-sample)
* [Hardware settings sample](https://github.com/Core-2-Extreme/GUI_sample#hardware-settings-sample)
* [Camera and mic sample](https://github.com/Core-2-Extreme/GUI_sample#camera-and-mic-sample)
* [Speaker sample](https://github.com/Core-2-Extreme/GUI_sample#speaker-sample)

### Draw image sample
This is a sample code to draw images such as .png and .jpg.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_0.png" width="400" height="480">

Application source code : [`source/sub_app0.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app0.cpp) \
Used APIs : 
* [`Draw_texture*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/draw/draw.h)
* [`Util_converter_convert_color()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.h)
* [`Util_curl_dl_data()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/curl.h)
* [`Util_decoder_image_decode*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/decoder.h)

### File explorer sample
This is a sample code to use file explorer.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_1.png" width="400" height="480">

Application source code : [`source/sub_app1.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app1.cpp) \
Used APIs : 
* [`Util_expl*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/expl.h)

### Hardware settings sample
This is a sample code to change hardware settings such as wifi state, screen brightness and sleep.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_2.png" width="400" height="480">

Application source code : [`source/sub_app2.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app2.cpp) \
Used APIs : 
* [`Util_hw_config*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/hw_config.h)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.h)

### Camera and mic sample
This is a sample code to take a picture by using camera and record sound by using mic.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_3.png" width="400" height="480">

Application source code : [`source/sub_app3.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app3.cpp) \
Used APIs : 
* [`Util_cam*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/cam.h)
* [`Util_converter_convert_color()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.h)
* [`Util_encoder*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/encoder.h)
* [`Util_mic*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/mic.h)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.h)

### Speaker sample
This is a sample code to playback audio.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_4.png" width="400" height="480">

Application source code : [`source/sub_app4.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app4.cpp) \
Used APIs : 
* [`Util_converter_convert_audio()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.h)
* [`Util_decoder*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/decoder.h)
* [`Util_speaker*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/speaker.h)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.h)

## Build
You need : 
* [devkitpro](https://devkitpro.org/wiki/Getting_Started) ([install guide](library/00_devkitpro_install.md))

If you want to build .cia, then you also need : 
* [bannertool](https://github.com/diasurgical/bannertool/releases/tag/1.2.0)
  * Broken links? Try our [backup](https://github.com/Core-2-Extreme/bannertool_fork/releases/tag/1.2.0).
* [makerom](https://github.com/3DSGuy/Project_CTR/releases/tag/makerom-v0.18.4)
  * Broken links? Try our [backup](https://github.com/Core-2-Extreme/Project_CTR_fork/releases/tag/makerom-v0.18.4).

(Copy them in your path e.g. in `{devkitPro_install_dir}\tools\bin`).

As of this writing, we use these packages to build this project. \
Note : Not all of them are necessary e.g. `devkitARM-gdb` is not required to build and we don't use \
standard `libctru` and `citro2(3)d`, but we just document all of them in case someone need these information. \
For more information, see [here](library/00_devkitpro_install.md#install-devkitpro).
```
$ dkp-pacman -Q
3ds-cmake 1.5.1-1
3ds-examples 20230610-1
3ds-pkg-config 0.28-5
3dslink 0.6.2-1
3dstools 1.3.1-3
catnip 0.1.0-1
citro2d 1.6.0-1
citro3d 1.7.1-2
devkit-env 1.0.1-2
devkitARM r64-2
devkitARM-gdb 14.1-2
devkitarm-cmake 1.2.2-1
devkitarm-crtls 1.2.6-1
devkitarm-rules 1.5.1-1
devkitpro-keyring 20180316-1
dkp-cmake-common-utils 1.5.2-1
general-tools 1.4.4-1
libctru 2.3.1-1
pacman 6.0.1-7
picasso 2.7.2-3
tex3ds 2.3.0-4
```

For .cia build
```
bannertool 1.2.0
makerom v0.18.4
```

If you want to make changes to the libraries, then follow [this guide](library/00_devkitpro_install.md#build-libraries-usually-optional).

After having all dependencies, do :
* Clone this repository (`git clone https://github.com/core-2-extreme/{project_name}`).
  * On windows run `build_3dsx.bat` for `.3dsx` only build or `build.bat` for `.3dsx`+`.cia` build.
  * On other system, run `make 3dsx` for `.3dsx` only build or `make all` for `.3dsx`+`.cia` build.

## License
This software is licensed under GNU General Public License v3.0 or later.

Note : As of this writing, `zlib` is not used in this project \
(once it was used), however it may be used in the future.

Third party libraries are licensed under :

| Library                                                                     | License                                |
| --------------------------------------------------------------------------- | -------------------------------------- |
| [libctru](https://github.com/devkitPro/libctru#license)                     | zlib License                           |
| [citro3d](https://github.com/devkitPro/citro3d/blob/master/LICENSE)         | zlib License                           |
| [citro2d](https://github.com/devkitPro/citro2d/blob/master/LICENSE)         | zlib License                           |
| [x264](https://github.com/mirror/x264/blob/master/COPYING)                  | GNU General Public License v2.0        |
| [libmp3lame](https://github.com/gypified/libmp3lame/blob/master/COPYING)    | GNU Lesser General Public License v2.0 |
| [dav1d](https://github.com/videolan/dav1d/blob/master/COPYING)              | BSD 2-Clause                           |
| [ffmpeg](https://github.com/FFmpeg/FFmpeg/blob/master/COPYING.GPLv3)        | GNU General Public License v3.0        |
| [zlib](https://github.com/madler/zlib/blob/master/LICENSE)                  | zlib License                           |
| [mbedtls](https://github.com/Mbed-TLS/mbedtls/blob/development/LICENSE)     | Apache License 2.0                     |
| [nghttp2](https://github.com/nghttp2/nghttp2/blob/master/COPYING)           | MIT License                            |
| [curl](https://github.com/curl/curl/blob/master/COPYING)                    | The curl license                       |
| [stb_image](https://github.com/nothings/stb/blob/master/LICENSE)            | Public Domain                          |
