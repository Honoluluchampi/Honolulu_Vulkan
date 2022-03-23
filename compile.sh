COMPILER="/home/honolulu/programs/downloaded_libraries/vulkan1.3.204.0/x86_64/bin/glslc"

SHADERDIR="./shader"
OUTPUTDIR="./spv"
mkdir -p spv
$COMPILER $SHADERDIR/shader.vert -o $OUTPUTDIR/vert.spv
$COMPILER $SHADERDIR/shader.frag -o $OUTPUTDIR/frag.spv