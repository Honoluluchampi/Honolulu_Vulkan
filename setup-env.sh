# execute this script in ~/.bashrc and so on

# configure this part depending on your environment
export HNLL_ENGN=~/programs/honolulu_engine
VULKAN_DIR=~/programs/external_libraries/vulkanSDK

source $VULKAN_DIR/setup-env.sh
if [ "$(uname)" == 'Darwin' ]; then
  export VULKAN_DIR=$VULKAN_DIR/macOS
elif [ "$(expr substr $(uname -s) 1 5)" == 'Linux' ]; then
  export VULKAN_DIR=$VULKAN_DIR/x86_64
fi

# git submodules
export IMGUI_DIR=$HNLL_ENGN/submodules/imgui

# download imgui font
if [ ! -e $HNLL_ENGN/include/gui/roboto_regular.embed ]; then
  curl -o $HNLL_ENGN/include/gui/roboto_regular.embed https://raw.githubusercontent.com/TheCherno/Walnut/master/Walnut/src/Walnut/ImGui/Roboto-Regular.embed
fi

# download tiny_obj_loader
if [ ! -e $HNLL_ENGN/submodules/tiny_obj_loader/tiny_obj_loader.h ]; then
  mkdir $HNLL_ENGN/submodules/tiny_obj_loader
  echo "download tiny obj loader"
  curl -o $HNLL_ENGN/submodules/tiny_obj_loader/tiny_obj_loader.h https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/master/tiny_obj_loader.h
fi
export TINY_OBJ_LOADER_DIR=$HNLL_ENGN/submodules/tiny_obj_loader

export MODEL_DIR=$HNLL_ENGN/models

# glfw glm
# for OS X
if [ "$(uname)" == 'Darwin' ]; then
  BREW_PREFIX=`brew --prefix`
  # glfw
  GLFW_VERSION=`ls ${BREW_PREFIX}/Cellar/glfw`
  export GLFW_DIR=${BREW_PREFIX}/Cellar/glfw/${GLFW_VERSION}
  # glm
  GLM_VERSION=`ls ${BREW_PREFIX}/Cellar/glm`
  export GLM_DIR=${BREW_PREFIX}/Cellar/glm/${GLM_VERSION}
  # x11
  X11_VERSION=`ls ${BREW_PREFIX}/Cellar/libx11`
  export X11_DIR=${BREW_PREFIX}/Cellar/libx11/${X11_DIR}
fi