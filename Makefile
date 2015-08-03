ifndef OBS_INCLUDE
OBS_INCLUDE = /usr/include/obs
endif
ifndef OBS_LIB
OBS_LIB = /usr/lib
endif

CXXFLAGS = -std=c++11 -Wall -g -fPIC -I$(OBS_INCLUDE) -I./src $(shell pkg-config --cflags Qt5WebKitWidgets)
CXX      = g++
RM       = /bin/rm -rf
LDFLAGS  = -L$(OBS_LIB)
LDLIBS_LIB   = -lobs -lrt
LDLIBS_RENDERER   = $(shell pkg-config --libs Qt5WebKitWidgets) -lrt

LIB = build/qtwebkit-browser.so
LIB_OBJ = build/qtwebkit-main.o build/qtwebkit-source.o build/qtwebkit-manager.o

RENDERER = build/renderer
RENDERER_SRC = src/qtwebkit-renderer.cpp
RENDERER_OBJ = build/qtwebkit-renderer.o

PLUGIN_BUILD_DIR = build/qtwebkit-browser
PLUGIN_INSTALL_DIR = ~/.config/obs-studio/plugins
ifdef OBS_PLUGIN_INSTALL_OLD
PLUGIN_INSTALL_DIR = ~/.obs-studio/plugins
endif
PLUGIN_DATA_DIR = data

ARCH = $(shell getconf LONG_BIT)
PLUGIN_BIN_DIR = $(PLUGIN_BUILD_DIR)/bin/$(ARCH)bit
ifdef OBS_PLUGIN_BUILD_OLD
PLUGIN_BIN_DIR = $(PLUGIN_BUILD_DIR)/bin
endif

all: plugin

.PHONY: plugin
plugin: $(LIB) $(RENDERER)
	mkdir -p $(PLUGIN_BIN_DIR)
	cp $(LIB) $(RENDERER) $(PLUGIN_BIN_DIR)
	cp -r $(PLUGIN_DATA_DIR) $(PLUGIN_BUILD_DIR)

.PHONY: install
install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp -r $(PLUGIN_BUILD_DIR) $(PLUGIN_INSTALL_DIR)

$(RENDERER): $(RENDERER_OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS_RENDERER) -o $@

$(LIB): $(LIB_OBJ)
	$(CXX) -shared $(LDFLAGS) $^ $(LDLIBS_LIB) -o $@

build/%.o: src/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(LIB_OBJ) $(LIB) $(RENDERER_OBJ) $(RENDERER) $(PLUGIN_BUILD_DIR)
