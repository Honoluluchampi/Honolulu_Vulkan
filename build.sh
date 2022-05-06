# compile shader
cd honolulu_vulkan_engine
sh compile.sh
cd ..

# build
mkdir -p build
cd build
cmake ..
make
cd ..
sh copy_libs.sh