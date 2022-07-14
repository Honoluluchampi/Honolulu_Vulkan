# execute this script in ~/.bashrc and so on

VULKAN_DIR=~/programs/external_libraries/vulkanSDK
source $VULKAN_DIR/setup-env.sh
# configure this part depending on your environment
if [ "$(uname)" == 'Darwin' ]; then
  export VULKAN_DIR=$VULKAN_DIR/macOS
elif [ "$(expr substr $(uname -s) 1 5)" == 'Linux' ]; then
  export VULKAN_DIR=$VULKAN_DIR/x86_64
fi

# git submodules
export IMGUI_DIR=$PWD/submodules/imgui

# download imgui font
if [ ! -e include/imgui/roboto_regular.embed ]; then
  curl -o include/imgui/roboto_regular.embed https://raw.githubusercontent.com/TheCherno/Walnut/master/Walnut/src/Walnut/ImGui/Roboto-Regular.embed
fi

# download tiny_obj_loader
if [ ! -e submodules/tiny_obj_loader/tiny_obj_loader.h ]; then
  mkdir submodules/tiny_obj_loader
  echo "download tiny obj loader"
  curl -o submodules/tiny_obj_loader/tiny_obj_loader.h https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/master/tiny_obj_loader.h
fi
export TINY_OBJ_LOADER_DIR=$PWD/submodules/tiny_obj_loader

# hnll
export HNLL_ENGN=$PWD
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