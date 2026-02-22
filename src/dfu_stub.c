/*
 * Stub for DFU target MCUboot functions.
 * Required because the Matter GN build compiles OTAImageProcessorImpl.cpp
 * even when OTA/MCUboot is disabled.
 */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

int dfu_target_mcuboot_set_buf(uint8_t *buf, size_t len)
{
    (void)buf;
    (void)len;
    return -ENOTSUP;
}
