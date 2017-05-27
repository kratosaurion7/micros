#ifndef KERNEL_FEATURES_H
#define KERNEL_FEATURES_H

#include <stddef.h>
#include <stdint.h>

struct kernel_info_block;

#include "common.h"
#include "multiboot.h"


// Private Members
enum graphical_modes {
    TEXT,
    VGA_GRAPHICS,
    NONE
};

struct kernel_features{
    enum graphical_modes current_graphic_mode;
    
    BOOL isDebugBuild;
    
    size_t kernel_options_size;
    unsigned char** kernel_options;
};

struct kernel_features* features;

void init_module_kernel_features(struct kernel_info_block* kinfo);

void parse_commandline(unsigned char* cmdline);

// Public Members
void kfDetectFeatures(multiboot_info_t* info);
BOOL kfSupportGraphics();
BOOL kfDebugMode();

#endif