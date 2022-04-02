# config VULKAN_DIR, TINY_OBJ_LOADER_DIR in your .env

LIB_NAME		=libhve.a

SRCROOT			=./src
SRCDIRS		 :=$(shell find $(SRCROOT) -type d)
SRCS 				=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))
OBJROOT			=./obj
OBJS 				=$(subst $(SRCROOT), $(OBJROOT), $(SRCS:.cpp=.o))
INCDIRS 		=$(subst $(SRCROOT), ./include, $(SRCDIRS))
INCS 				=$(foreach dir, $(INCDIRS), $(wildcard $(INCDIRS)/*.hpp))
DEPENDS 		=$(OBJS:.o=.d)
BIN_DIR			=./bin
LIB_DIR			=./lib
TARGET_LIB 	=$(LIB_DIR)/$(LIB_NAME)
CXX 				=g++-10
CFLAGS 			= -std=c++17 -g3

HGE =/home/honolulu/programs/honolulu_engine/honolulu_game_engine/include

LDFLAGS 		=$(VULKAN_DIR)/lib/libvulkan.so.1.3.204 $(VULKAN_DIR)/lib/libvulkan.so.1 /usr/lib/x86_64-linux-gnu/libglfw.so.3.3
INCFLAGS 		=-I$(HGE) -I/usr/include -I$(VULKAN_DIR)/include $(addprefix -I, $(INCDIRS))

VK_ICD_FILENAMES 	:=$(VULKAN_DIR)/etc/vulkan/icd.d/MoltenVK_icd.json
VK_LAYER_PATH 		:=$(VULKAN_DIR)/etc/vulkan/explicit_layer.d

# build the lib
$(TARGET_LIB): $(OBJS) $(INCS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	$(AR) rcs $(TARGET_LIB) $(OBJS)

# compile object files
$(OBJROOT)/%.o: $(SRCROOT)/%.cpp
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -o $@ -c $< $(INCFLAGS)

.PHONY: default
default: $(TARGET_LIB)

.PHONY: clean
clean:
	-rm -f $(OBJS)
	-rm -f $(TARGET_LIB)

.PHONY: all
all:
	make clean
	sh compile.sh
	make default

-include $(DEPENDS)