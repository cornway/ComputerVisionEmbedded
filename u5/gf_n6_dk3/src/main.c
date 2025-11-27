/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>

#include "application.h"

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
	init();

	while (true) {
		loop();
	}
	return 0;
}
