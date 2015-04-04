OBS_INCLUDE = /usr/include/obs
QT5_INCLUDE = /usr/include/qt

CXXFLAGS = -std=c++11 -Wall -g -fPIC -I$(OBS_INCLUDE) -I$(QT5_INCLUDE) -I./src
CXX      = g++
RM       = /bin/rm -rf
LDFLAGS  = 
LDLIBS   = -lobs -lQt5Widgets -lQt5Network -lQt5Gui -lQt5WebKitWidgets -lQt5Core -lrt

LIB = build/qtwebkit-browser.so
LIB_OBJ = build/qtwebkit-main.o build/qtwebkit-source.o build/qtwebkit-manager.o

RENDERER = build/renderer
RENDERER_SRC = src/qtwebkit-renderer.cpp
RENDERER_OBJ = build/qtwebkit-renderer.o

PLUGIN_BUILD_DIR = build/qtwebkit-browser
PLUGIN_INSTALL_DIR = ~/.obs-studio/plugins

all: plugin

.PHONY: plugin
plugin: $(LIB) $(RENDERER)
	mkdir -p $(PLUGIN_BUILD_DIR)/bin
	cp $(LIB) $(RENDERER) $(PLUGIN_BUILD_DIR)/bin

.PHONY: install
install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp -r $(PLUGIN_BUILD_DIR) $(PLUGIN_INSTALL_DIR)

$(RENDERER): $(RENDERER_OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(LIB): $(LIB_OBJ)
	$(CXX) -shared $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(LIB_OBJ) $(LIB) $(RENDERER_OBJ) $(RENDERER) $(PLUGIN_BUILD_DIR)
