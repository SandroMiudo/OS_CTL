// Cross-platform GPU count using OpenCL

#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "global_consts.h"

const char *SOCKET_PATH = MERGE(RUN_TGT_DIR, display.sock);

uint32_t get_usable_gpu_count(void) {
    cl_uint platformCount = 0;
    cl_int ret = clGetPlatformIDs(0, NULL, &platformCount);
    if (ret != CL_SUCCESS || platformCount == 0) {
        return 0; // No OpenCL platforms found
    }

    cl_platform_id *platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * platformCount);
    if (!platforms) return 0;

    ret = clGetPlatformIDs(platformCount, platforms, NULL);
    if (ret != CL_SUCCESS) {
        free(platforms);
        return 0;
    }

    cl_uint total_gpus = 0;

    for (cl_uint i = 0; i < platformCount; i++) {
        cl_uint deviceCount = 0;
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceCount);
        if (ret == CL_SUCCESS && deviceCount > 0) {
            total_gpus += deviceCount;
        }
    }

    free(platforms);
    return total_gpus;
}