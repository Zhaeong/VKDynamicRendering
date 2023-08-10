#!/bin/bash


#export VK_ICD_FILENAMES=/opt/amdgpu-pro/etc/vulkan/icd.d/amd_icd64.json
# #export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json


# export REPLAY_DELAY=50

# #GMSharing Portion
export VK_LAYER_PATH="/usr/share/vulkan/explicit_layer.d:/opt/GfxCloudService/settings"

export ENABLE_GMSHARING_LAYER=1
export GM_ANALYSIS_MODE=0
export GM_ENABLE_VRS=0 
export GM_DISABLE_SHARED=0 

export GM_DISABLE_FILE_OUTPUT=0

rm ./GCSLayerLogs.txt
export GM_LOGGER_NAME=./GCSLayerLogs.txt

export GM_SHARE_COMPRESSED_ONLY=0

export VK_INSTANCE_LAYERS="VK_LAYER_cloud_service:VK_LAYER_MESA_overlay"

./bin/vkGame