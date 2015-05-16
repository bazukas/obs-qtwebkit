# About

This is a browser source plugin for obs-studio (https://github.com/jp9000/obs-studio) based
on QtWebkit library. This plugin is Linux only.

Currently does not support interaction with the web page, but it might be added later.

# Building

Make sure you have necessary dependencies installed, which are obs-studio and qt5 and qt5-webkit
development packages. You may need to set `OBS_INCLUDE` and `OBS_LIB` env variables (see Makefile).

Run `make` in the obs-qtwebkit directory to build the plugin.

# Installing

Run `make install` to copy plugin binaries into $HOME/.obs-studio/plugins.

# Design choices

Rendering web pages is done using QWebFrame, which only works in the main thread. Because of that,
rendering is done in a child process. Currently, the process is restarted every time user changes
source's properties, it would be nice to avoid that.
