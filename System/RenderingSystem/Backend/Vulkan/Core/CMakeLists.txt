target_sources(Cgppy PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanContext.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanDevice.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanSwapchain.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanResource.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanBuffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanImage.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanRenderStage.c
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanPipeline.c
)