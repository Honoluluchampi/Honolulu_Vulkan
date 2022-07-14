# setup env
source setup-env.sh

# compile shader
sh modules/vulkan/compile.sh

# build
cmake -S . -B build
cmake --build build
# sh copy_libs.sh