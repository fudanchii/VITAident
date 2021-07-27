#include <stdint.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <taihen.h>

int ksceKernelSysrootGetSystemSwVersion(void);

int Kernel_GetCurrentFirmware(void) {
    uint32_t state = 0;
    ENTER_SYSCALL(state);
    
    unsigned int current_fw = -1;
    int ret = ksceKernelSysrootGetSystemSwVersion();
    
    if (ret)
        current_fw = ret;
      
    EXIT_SYSCALL(state);
    return current_fw;
}

// Thanks TheOfficialFloW!
uint32_t Kernel_GetFactoryFirmware(void) {
    uint32_t state = 0;
    ENTER_SYSCALL(state);
    
    uint32_t factory_fw = 0;
    void *sysroot = ksceKernelGetSysrootBuffer();
    
    if (sysroot)
        factory_fw = *(unsigned int *)(sysroot + 8);
        
    EXIT_SYSCALL(state);
    return factory_fw;
}

uint32_t Kernel_GetCPU_midr(void) {
    uint32_t state = 0;
    ENTER_SYSCALL(state);

    uint32_t mid = 0;

    asm ("mrc p15, 0, %0, c0, c0, 0" : "=r" (mid));

    EXIT_SYSCALL(state);

    return mid;
}

uint32_t Kernel_GetCPU_mpidr(void) {
    uint32_t state = 0;
    ENTER_SYSCALL(state);

    uint32_t mpid = 0;

    asm ("mrc p15, 0, %0, c0, c0, 5" : "=r" (mpid));

    ENTER_SYSCALL(state);

    return mpid;
}

uint32_t Kernel_GetCPU_count(void) {
    uint32_t state = 0;
    ENTER_SYSCALL(state);

    static SceCorelockContext ctx;

    ksceKernelCorelockContextInitialize(&ctx);

    EXIT_SYSCALL(state);

    return ctx.core_count;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize args, void *argp) {
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
    return SCE_KERNEL_STOP_SUCCESS;
}
