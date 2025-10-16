# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

# Out-of-tree drivers for existing driver classes STM32 NEMA 2DGPU
add_subdirectory_ifdef(CONFIG_NEMA nema)
add_subdirectory_ifdef(CONFIG_STM32_JPEG jpeg)
