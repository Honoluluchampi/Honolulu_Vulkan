COMPILER="/home/honolulu/programs/external_libraries/vulkanSDK/x86_64/bin/glslc"

SHADERDIR=$HVE_DIR/shader
OUTPUTDIR=$HVE_DIR/shader/spv
mkdir -p $HVE_DIR/shader/spv
$COMPILER $SHADERDIR/simple_shader.vert -o $OUTPUTDIR/simple_shader.vert.spv
$COMPILER $SHADERDIR/simple_shader.frag -o $OUTPUTDIR/simple_shader.frag.spv
$COMPILER $SHADERDIR/point_light.vert -o $OUTPUTDIR/point_light.vert.spv
$COMPILER $SHADERDIR/point_light.frag -o $OUTPUTDIR/point_light.frag.spv