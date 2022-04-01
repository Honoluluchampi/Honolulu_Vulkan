# config VULKAN_DIR in .env
# config HVE_DIR in ~/.bachrc and so on

LIB_NAME 		=libhge.a
BIN_NAME 		=hge_app

SRCROOT			=./src
SRCDIRS		 :=$(shell find $(SRCROOT) -type d)
SRCS 				=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))
OBJROOT			=./obj
OBJS 				=$(subst $(SRCROOT), $(OBJROOT), $(SRCS:.cpp=.o))
INCROOT			=./include
INCDIRS 		=$(subst $(SRCROOT), $(INCROOT), $(SRCDIRS))
INCS 				=$(foreach dir, $(INCDIRS), $(wildcard $(INCDIRS)/*.hpp))
MAINSRC			=./main.cpp
DEPENDS 		=$(OBJS:.o=.d)
BIN_DIR			=./bin

LIB_DIR			=./lib
TARGET_LIB 	=$(LIB_DIR)/$(LIB_NAME)
CXX 				=g++-10
CFLAGS 			=-std=c++17 -g3

HVE_LIB			=$(HVE_DIR)/lib/libhve.a
INCFLAGS 		=-I./honolulu_vulkan_engine/include -I/usr/include -I$(VULKAN_DIR)/include $(addprefix -I, $(INCDIRS))

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

# build both engine
.PHONY: default
default: $(TARGET_LIB)

# build hve
.PHONY: hve
hve:
	cd ../honolulu_vulkan_engine && make

.PHONY: clean
clean:
	-rm -f $(OBJS)
	-rm -f $(TARGET_LIB)

.PHONY: all
all:
	make clean
	cd honolulu_vulkan_engine && make all
	make default

-include $(DEPENDS)