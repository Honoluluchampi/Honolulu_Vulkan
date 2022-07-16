# setup env
source setup-env.sh

# compile shader
sh modules/graphics/compile.sh

# build
cmake -S . -B build
cmake --build build -j 10
# sh copy_libs.sh