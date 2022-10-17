COMPILER=${VULKAN_DIR}/bin/glslc

SHADERDIR=${HNLL_ENGN}/modules/graphics/shader
OUTPUTDIR=${HNLL_ENGN}/modules/graphics/shader/spv
mkdir -p ${OUTPUTDIR}

for FILE in ${SHADERDIR}/*.*
  do 
    FILENAME=$(basename ${FILE})
    ${COMPILER} $FILE -o ${OUTPUTDIR}/${FILENAME}.spv
done