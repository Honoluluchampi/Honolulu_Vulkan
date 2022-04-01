# confit ~/.bachrc and so on to call 'source (this directory)/setup-env.sh

BIN_NAME =he_app
MAIN_SRC =main.cpp
BIN_DIR =./bin
TARGET_BIN 	=$(BIN_DIR)/$(BIN_NAME)
CXX 				=g++-10
CFLAGS 			=-std=c++17 -g3
HGE_DIR			=./honolulu_game_engine
HVE_DIR			=$(HGE_DIR)/honolulu_vulkan_engine
HGE_LIB			=$(HGE_DIR)/lib/libhge.a
HVE_LIB			=$(HVE_DIR)/lib/libhve.a
LDFLAGS		  =$(HGE_LIB) $(HVE_LIB) 
LDFLAGS 	 +=$(VULKAN_DIR)/lib/libvulkan.so.1.3.204 $(VULKAN_DIR)/lib/libvulkan.so.1 /usr/lib/x86_64-linux-gnu/libglfw.so.3.3
INCFLAGS		=-I$(HGE_DIR)/include -I$(HVE_DIR)/include 

# build an app
$(TARGET_BIN): $(MAIN_SRC) $(LDFLAGS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $(MAIN_SRC) $(INCFLAGS) $(LDFLAGS)

.PHONY: default
default: $(TARGET_BIN)

.PHONY: hge
hge:
	cd honolulu_game_engine && make

.PHONY: hve
hve:
	cd honolulu_game_engine/honolulu_vulkan_engine && make

.PHONY: test
test:
	$(TARGET_BIN)

.PHONY: clean
clean:
	-rm -f $(TARGET_BIN)

.PHONY: all
all:
	make clean
# make all recursively
	cd honolulu_game_engine && make all
	make default
	make test

-include $(DEPENDS)