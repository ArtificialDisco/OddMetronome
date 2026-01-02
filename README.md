# OddMetronome
A metronome that can do odd time signatures, written in C and SDL.

For building on Ubuntu, you need to

sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-ttf-dev
sudo apt-get install libsdl2-mixer-dev
sudo apt-get install libsdl2-dev

This should be similar for other package managers on other systems. SDL is pretty universal.
Afterwards just type make.

For building on Windows things get a little trickier. I recommend reading Lazy Foo's tutorial on how to set it up.
There is a Visual Studio 19 community edition project file included in the repository with my setup which assumes
the SDL includes and SDL libraries are set up in c:\vclib. You might still need to tweak the version numbers.

The DLL files are shipped as well, and the VS project includes a build step that copies them to the Release folder.
