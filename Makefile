LIB_NAME		=libhve.a
BIN_NAME		=hve_app

SRCDIR 			=./src
SRCS 				=$(wildcard $(SRCDIR)/*.cpp)
OBJDIR 			=./obj
OBJS 				=$(addprefix $(OBJDIR)/, $(notdir $(SRCS:.cpp=.o)))
INCDIR 			=./include
INCS 				=$(wildcard $(INCDIR)/*.hpp)
MAINSRC			=./main.cpp
DEPENDS 		= $(OBJS:.o=.d)
BIN_DIR			=./bin
TARGET_BIN 	=$(BIN_DIR)/$(BIN_NAME)
LIB_DIR			=./lib
TARGET_LIB 	=$(LIB_DIR)/$(LIB_NAME)
COMPILER 		= g++-10
CFLAGS 			= -std=c++17 -g3

# only support ubuntu for now
VULKANDIR 	= /home/honolulu/programs/downloaded_libraries/vulkanSDK/x86_64
LDFLAGS 		= $(VULKANDIR)/lib/libvulkan.so.1.3.204 $(VULKANDIR)/lib/libvulkan.so.1 /usr/lib/x86_64-linux-gnu/libglfw.so.3.3
INCFLAGS 		= -I/usr/include -I$(VULKANDIR)/include -I./include

VK_ICD_FILENAMES 	:= $(VULKANDIR)/etc/vulkan/icd.d/MoltenVK_icd.json
VK_LAYER_PATH 		:= $(VULKANDIR)/etc/vulkan/explicit_layer.d

# build an app
$(TARGET_BIN): $(MAINSRC) $(TARGET_LIB) $(INCS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(COMPILER) $(CFLAGS) -o $@ $(MAINSRC) $(INCFLAGS) $(TARGET_LIB) $(LDFLAGS) 

# build the lib
$(TARGET_LIB): $(OBJS) $(INCS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	$(AR) rcs $(TARGET_LIB) $(OBJS)

# compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) -o $@ -c $< $(INCFLAGS)

.PHONY: default
default: $(TARGET_BIN)

.PHONY: lib
lib: $(TARGET_LIB)

.PHONY: test
test:
	$(TARGET_BIN)

.PHONY: clean
clean:
	-rm -f $(OBJS)
	-rm -f $(TARGET_LIB)
	-rm -f $(TARGET_BIN)

.PHONY: all
all:
	make clean
	make default
	make test

-include $(DEPENDS)