ifndef OBS_INCLUDE
OBS_INCLUDE = /usr/include/obs
endif

CXXFLAGS = -std=c++11 -Wall -g -fPIC -I$(OBS_INCLUDE) -I./src $(shell pkg-config --cflags Qt5WebKitWidgets)
CXX      = g++
RM       = /bin/rm -rf
LDFLAGS  = 
LDLIBS   = -lobs $(shell pkg-config --libs Qt5WebKitWidgets) -lrt

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
