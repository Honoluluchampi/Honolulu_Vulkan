COMPILER="/home/honolulu/programs/external_libraries/vulkanSDK/x86_64/bin/glslc"

SHADERDIR="./shader"
OUTPUTDIR="./shader/spv"
mkdir -p ./shader/spv
$COMPILER $SHADERDIR/simple_shader.vert -o $OUTPUTDIR/simple_shader.vert.spv
$COMPILER $SHADERDIR/simple_shader.frag -o $OUTPUTDIR/simple_shader.frag.spv
$COMPILER $SHADERDIR/point_light.vert -o $OUTPUTDIR/point_light.vert.spv
$COMPILER $SHADERDIR/point_light.frag -o $OUTPUTDIR/point_light.frag.spv
$COMPILER $SHADERDIR/line_shader.vert -o $OUTPUTDIR/line_shader.vert.spv
$COMPILER $SHADERDIR/line_shader.frag -o $OUTPUTDIR/line_shader.frag.spv