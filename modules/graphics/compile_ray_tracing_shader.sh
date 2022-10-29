COMPILER=${VULKAN_DIR}/bin/glslangValidator
TARGET_ENV=vulkan1.3

SHADERDIR=${HNLL_ENGN}/modules/graphics/ray_tracing_shader
OUTPUTDIR=${HNLL_ENGN}/modules/graphics/ray_tracing_shader/spv
mkdir -p ${OUTPUTDIR}

extensions=("rgen" "rmiss" "rchit" "rahit" "rint" "comp")

for extension in ${extensions[@]}
  do
    for FILE in ${SHADERDIR}/*.${extension}
      do
        FILENAME=$(basename ${FILE})
        ${COMPILER} -S ${extension} ${FILE} --target-env ${TARGET_ENV} -o ${OUTPUTDIR}/${FILENAME}.spv 
    done
done