COMPILER=${VULKAN_DIR}/bin/glslc

SHADERDIR=${HNLL_ENGN}/modules/graphics/shader
OUTPUTDIR=${HNLL_ENGN}/modules/graphics/shader/spv
mkdir -p ${OUTPUTDIR}

for FILE in ${SHADERDIR}/*.*
  do 
    FILENAME=$(basename ${FILE})
    ${COMPILER} $FILE -o ${OUTPUTDIR}/${FILENAME}.spv
done
# $COMPILER ${SHADERDIR}/simple_shader.vert -o ${OUTPUTDIR}/simple_shader.vert.spv
# $COMPILER ${SHADERDIR}/simple_shader.frag -o ${OUTPUTDIR}/simple_shader.frag.spv
# $COMPILER ${SHADERDIR}/point_light.vert -o ${OUTPUTDIR}/point_light.vert.spv
# $COMPILER ${SHADERDIR}/point_light.frag -o ${OUTPUTDIR}/point_light.frag.spv
# $COMPILER ${SHADERDIR}/wire_frustum_shader.vert -o ${OUTPUTDIR}/wire_frustum_shader.vert.spv
# $COMPILER ${SHADERDIR}/wire_frustum_shader.frag -o ${OUTPUTDIR}/wire_frustum_shader.frag.spv