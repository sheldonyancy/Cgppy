#include "YVulkanRenderingSystem.h"
#include "YDeveloperConsole.hpp"


void yRenderDeveloperConsole(YsVkCommandUnit* command_unit,
                             u32 command_buffer_index,
                             u32 current_frame, 
                             u32 image_index) {
    YDeveloperConsole::instance()->cmdDraw(command_unit,
                                           command_buffer_index,
                                           current_frame, 
                                           image_index);
}