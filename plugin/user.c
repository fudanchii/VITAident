#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

#include "plugin.h"

int User_GetCurrentFirmware(void) {
    return Kernel_GetCurrentFirmware();
}

// Thanks TheOfficialFloW!
uint32_t User_GetFactoryFirmware(void) {
    return Kernel_GetFactoryFirmware();
}

uint32_t User_GetCPU_midr(void) {
    return Kernel_GetCPU_midr();
}

uint32_t User_GetCPU_mpidr(void) {
    return Kernel_GetCPU_mpidr();
}

uint32_t User_GetCPU_count(void) {
    return Kernel_GetCPU_count();
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
    return SCE_KERNEL_STOP_SUCCESS;
}
