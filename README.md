# Merge log

Scroll down for the original README.md!

Base revision: eadd8b91b2bd2dd8a782c80fce261e2cd475d79b

|Pull Request|Commit|Title|Author|Merged?|
|----|----|----|----|----|
|[6](https://github.com/citra-emu/citra-canary/pull/6)|[a53a8d3](https://github.com/citra-emu/citra-canary/pull/6/files/)|Canary Base (MinGW Test)|[liushuyu](https://github.com/liushuyu)|Yes|
|[4344](https://github.com/citra-emu/citra/pull/4344)|[61fd20c](https://github.com/citra-emu/citra/pull/4344/files/)|SeedDB: replace seek(tell, set) with seek(cur)|[wwylele](https://github.com/wwylele)|Yes|
|[4341](https://github.com/citra-emu/citra/pull/4341)|[751ebe5](https://github.com/citra-emu/citra/pull/4341/files/)|Kernel: make a kernel instance and remove object ID counter from global state|[wwylele](https://github.com/wwylele)|Yes|
|[4337](https://github.com/citra-emu/citra/pull/4337)|[bf71bf0](https://github.com/citra-emu/citra/pull/4337/files/)|nfc: Add Amiibo support (REOPENED)|[FearlessTobi](https://github.com/FearlessTobi)|Yes|
|[4336](https://github.com/citra-emu/citra/pull/4336)|[380b83e](https://github.com/citra-emu/citra/pull/4336/files/)|Only redefine some 64-bit file operation for MSVC|[wwylele](https://github.com/wwylele)|Yes|
|[4279](https://github.com/citra-emu/citra/pull/4279)|[deaed38](https://github.com/citra-emu/citra/pull/4279/files/)|applets: stub mii selector to always return a standard mii|[FearlessTobi](https://github.com/FearlessTobi)|Yes|
|[4270](https://github.com/citra-emu/citra/pull/4270)|[3970d86](https://github.com/citra-emu/citra/pull/4270/files/)|Threaded CPU vertex shader processing|[jroweboy](https://github.com/jroweboy)|Yes|
|[4235](https://github.com/citra-emu/citra/pull/4235)|[111b7db](https://github.com/citra-emu/citra/pull/4235/files/)|Port various game_list changes from yuzu|[FearlessTobi](https://github.com/FearlessTobi)|Yes|
|[4089](https://github.com/citra-emu/citra/pull/4089)|[80bdc4a](https://github.com/citra-emu/citra/pull/4089/files/)|[NOT FOR MERGE YET] Ignore format reinterpretation hack|[jroweboy](https://github.com/jroweboy)|Yes|


End of merge log. You can find the original README.md below the break.

------

**BEFORE FILING AN ISSUE, READ THE RELEVANT SECTION IN THE [CONTRIBUTING](https://github.com/citra-emu/citra/blob/master/CONTRIBUTING.md#reporting-issues) FILE!!!**

Citra
==============
[![Travis CI Build Status](https://travis-ci.org/citra-emu/citra.svg?branch=master)](https://travis-ci.org/citra-emu/citra)
[![AppVeyor CI Build Status](https://ci.appveyor.com/api/projects/status/sdf1o4kh3g1e68m9?svg=true)](https://ci.appveyor.com/project/bunnei/citra)

Citra is an experimental open-source Nintendo 3DS emulator/debugger written in C++. It is written with portability in mind, with builds actively maintained for Windows, Linux and macOS.

Citra emulates a subset of 3DS hardware and therefore is useful for running/debugging homebrew applications, and it is also able to run many commercial games! Some of these do not run at a playable state, but we are working every day to advance the project forward. (Playable here means compatibility of at least "Okay" on our [game compatibility list](https://citra-emu.org/game).)

Citra is licensed under the GPLv2 (or any later version). Refer to the license.txt file included. Please read the [FAQ](https://citra-emu.org/wiki/faq/) before getting started with the project.

Check out our [website](https://citra-emu.org/)!

For development discussion, please join us at #citra-dev on freenode.

### Development

Most of the development happens on GitHub. It's also where [our central repository](https://github.com/citra-emu/citra) is hosted.

If you want to contribute please take a look at the [Contributor's Guide](CONTRIBUTING.md) and [Developer Information](https://github.com/citra-emu/citra/wiki/Developer-Information). You should as well contact any of the developers in the forum in order to know about the current state of the emulator because the [TODO list](https://docs.google.com/document/d/1SWIop0uBI9IW8VGg97TAtoT_CHNoP42FzYmvG1F4QDA) isn't maintained anymore.

If you want to contribute to the user interface translation, please checkout [citra project on transifex](https://www.transifex.com/citra/citra). We centralize the translation work there, and periodically upstream translation.

### Building

* __Windows__: [Windows Build](https://github.com/citra-emu/citra/wiki/Building-For-Windows)
* __Linux__: [Linux Build](https://github.com/citra-emu/citra/wiki/Building-For-Linux)
* __macOS__: [macOS Build](https://github.com/citra-emu/citra/wiki/Building-for-macOS)


### Support
We happily accept monetary donations or donated games and hardware. Please see our [donations page](https://citra-emu.org/donate/) for more information on how you can contribute to Citra. Any donations received will go towards things like:
* 3DS consoles for developers to explore the hardware
* 3DS games for testing
* Any equipment required for homebrew
* Infrastructure setup
* Eventually 3D displays to get proper 3D output working

We also more than gladly accept used 3DS consoles, preferably ones with firmware 4.5 or lower! If you would like to give yours away, don't hesitate to join our IRC channel #citra on [Freenode](http://webchat.freenode.net/?channels=citra) and talk to neobrain or bunnei. Mind you, IRC is slow-paced, so it might be a while until people reply. If you're in a hurry you can just leave contact details in the channel or via private message and we'll get back to you.
