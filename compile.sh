COMPILER="/home/honolulu/programs/downloaded_libraries/vulkanSDK/x86_64/bin/glslc"

SHADERDIR="./shader"
OUTPUTDIR="./shader/spv"
mkdir -p ./shader/spv
$COMPILER $SHADERDIR/shader.vert -o $OUTPUTDIR/vert.spv
$COMPILER $SHADERDIR/shader.frag -o $OUTPUTDIR/frag.spv