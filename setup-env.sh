# execute this script in ~/.bashrc and so on

# configure this part depending on your environment
export HNLL_ENGN=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)
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

# download tiny_gltf
if [ ! -e $HNLL_ENGN/submodules/tiny_gltf/tiny_gltf.h ]; then
  mkdir $HNLL_ENGN/submodules/tiny_gltf
  echo "download tiny gltf"
  curl -o $HNLL_ENGN/submodules/tiny_gltf/tiny_gltf.h https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h
fi
export TINY_GLTF_DIR=$HNLL_ENGN/submodules/tiny_gltf

export MODEL_DIR=$HNLL_ENGN/models

# compile shaders
source ${HNLL_ENGN}/modules/graphics/compile.sh