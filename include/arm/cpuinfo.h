#ifndef H_ARM_CPUINFO

#define H_ARM_CPUINFO

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CPUINFO_ARM_MIDR_IMPLEMENTOR_MASK    0xFF000000
#define CPUINFO_ARM_MIDR_PART_MASK           0x0000FFF0
#define CPUINFO_ARM_MIDR_VARIANT_MASK        0x00F00000
#define CPUINFO_ARM_MIDR_REVISION_MASK       0x0000000F

#define CPUINFO_ARM_MIDR_IMPLEMENTOR_OFFSET  24
#define CPUINFO_ARM_MIDR_PART_OFFSET          4
#define CPUINFO_ARM_MIDR_VARIANT_OFFSET      20

#define CPUINFO_ARM_MPIDR_MP_OK_MASK         0x80000000
#define CPUINFO_ARM_MPIDR_UNIPROC_MASK       0x40000000
#define CPUINFO_ARM_MPIDR_MT_MP_MASK         0x01000000
#define CPUINFO_ARM_MPIDR_AFF2_MASK          0x00FF0000
#define CPUINFO_ARM_MPIDR_AFF1_MASK          0x0000FF00
#define CPUINFO_ARM_MPIDR_AFF0_MASK          0x000000FF

#define CPUINFO_ARM_MPIDR_AFF2_OFFSET        16
#define CPUINFO_ARM_MPIDR_AFF1_OFFSET        8
#define CPUINFO_ARM_MPIDR_AFF0_OFFSET        0

#define __implementor(midr) ((midr & CPUINFO_ARM_MIDR_IMPLEMENTOR_MASK) >> CPUINFO_ARM_MIDR_IMPLEMENTOR_OFFSET)
#define __part(midr) ((midr & CPUINFO_ARM_MIDR_PART_MASK) >> CPUINFO_ARM_MIDR_PART_OFFSET)
#define __variant(midr) ((midr & CPUINFO_ARM_MIDR_VARIANT_MASK) >> CPUINFO_ARM_MIDR_VARIANT_OFFSET)
#define __revision(midr) (midr & CPUINFO_ARM_MIDR_REVISION_MASK)

#define __implement_mp_ext(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_MP_OK_MASK) == CPUINFO_ARM_MPIDR_MP_OK_MASK)
#define __is_uniprocessor(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_UNIPROC_MASK) == CPUINFO_ARM_MPIDR_UNIPROC_MASK)
#define __is_multiprocessor(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_UNIPROC_MASK) == 0x00000000)

#define __is_multithreadprocessor(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_MT_MP_MASK) == CPUINFO_ARM_MPIDR_MT_MP_MASK)
#define __is_multicoreprocessor(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_MT_MP_MASK) == 0x00000000)

#define __aff2(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_AFF2_MASK) >> CPUINFO_ARM_MPIDR_AFF2_OFFSET)
#define __aff1(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_AFF1_MASK) >> CPUINFO_ARM_MPIDR_AFF1_OFFSET)
#define __aff0(mpidr) ((mpidr & CPUINFO_ARM_MPIDR_AFF0_MASK) >> CPUINFO_ARM_MPIDR_AFF0_OFFSET)

void cpuinfo_getVendorString(uint32_t, char *, uint8_t);
void cpuinfo_getCpuFamily(uint32_t, char *, uint8_t);
int cpuinfo_getVariant(uint32_t);
int cpuinfo_getRevision(uint32_t);

uint8_t cpuinfo_getMPSupport(uint32_t);
uint8_t cpuinfo_isUniProcessor(uint32_t);
uint8_t cpuinfo_isMultiThreadedProcessor(uint32_t);
uint8_t cpuinfo_getAff2(uint32_t);
uint8_t cpuinfo_getAff1(uint32_t);
uint8_t cpuinfo_getAff0(uint32_t);

uint8_t cpuinfo_getNumCores(uint32_t);

void cpuinfo_getVendorString(uint32_t midr, char *buff, uint8_t buflen) {
    char *vName;

    switch (__implementor(midr)) {
    case 'A':
        vName = "ARM Limited";
        break;
    case 'D':
        vName = "Digital Equipment Corporation";
        break;
    case 'M':
        vName = "Motorola - Freescale Semiconductor Inc.";
        break;
    case 'Q':
        vName = "Qualcomm Inc.";
        break;
    case 'V':
        vName = "Marvell Semiconductor Inc.";
    case 'a':
        vName = "Apple Inc.";
        break;
    case 'i':
        vName = "Intel ARM parts.";
        break;
    default:
        vName = "Unregistered vendor";
        break;
    }

    memset(buff, 0, buflen);
    if (strlen(vName) < buflen - 1) {
        strcpy(buff, vName);
    } else {
        memcpy(buff, vName, buflen - 1);
    }
}

void cpuinfo_getCpuFamily(uint32_t midr, char *buff, uint8_t buflen) {
    char *fName;

    switch(__part(midr)) {
    case 0xC05:
        fName = "Cortex-A5";
        break;
    case 0xC07:
        fName = "Cortex-A7";
        break;
    case 0xC08:
        fName = "Cortex-A8";
        break;
    case 0xC09:
        fName = "Cortex-A9";
        break;
    default:
        fName = "";
        break;
    }

    memset(buff, 0, buflen);
    if (strlen(fName) == 0 && buflen > 7) {
        sprintf(buff, "ARM %X", __part(midr));
        return;
    }

    if (strlen(fName) < buflen - 1) {
        strcpy(buff, fName);
    } else {
        memcpy(buff, fName, buflen - 1);
    }
}

int cpuinfo_getVariant(uint32_t midr) {
    return __variant(midr);
}

int cpuinfo_getRevision(uint32_t midr) {
    return __revision(midr);
}

uint8_t cpuinfo_getMPSupport(uint32_t mpidr) {
    return __implement_mp_ext(mpidr);
}

uint8_t cpuinfo_isUniProcessor(uint32_t mpidr) {
    return __is_uniprocessor(mpidr);
}

uint8_t cpuinfo_isMultiThreadedProcessor(uint32_t mpidr) {
    return __is_multithreadprocessor(mpidr);
}

uint8_t cpuinfo_getAff2(uint32_t mpidr) {
    return __aff2(mpidr);
}

uint8_t cpuinfo_getAff1(uint32_t mpidr) {
    return __aff1(mpidr);
}

uint8_t cpuinfo_getAff0(uint32_t mpidr) {
    return __aff0(mpidr);
}

uint8_t cpuinfo_getNumCores(uint32_t scu) {
    return (scu & 0x00000003) + 1;
}

#endif
