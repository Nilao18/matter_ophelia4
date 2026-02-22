/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 *    @file
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once

/* No MCUboot bootloader - provide fallback for OTA code that references MCUboot config */
#ifndef CONFIG_UPDATEABLE_IMAGE_NUMBER
#define CONFIG_UPDATEABLE_IMAGE_NUMBER 1
#endif
