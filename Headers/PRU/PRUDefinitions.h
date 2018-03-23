/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef PRUDEFINITIONS_H
#define PRUDEFINITIONS_H

// PRU Definitions
#define PRU_UNBIND_PATH	"/sys/bus/platform/drivers/pru-rproc/unbind"
#define PRU_BIND_PATH	"/sys/bus/platform/drivers/pru-rproc/bind"
#define PRU0_NAME		"4a334000.pru0"
#define PRU1_NAME		"4a338000.pru1"
#define PRU0_UEVENT		"/sys/bus/platform/drivers/pru-rproc/4a334000.pru0/uevent"
#define PRU1_UEVENT		"/sys/bus/platform/drivers/pru-rproc/4a338000.pru1/uevent"

#define PRU_ADDR		0x4A300000		// Start of PRU memory Page 184 am335x TRM
#define PRU_LEN			0x80000			// Length of PRU memory
#define PRU_SHAREDMEM	0x10000			// Offset to shared memory
#define CNT_OFFSET 		64

#endif // PRUDEFINITIONS_H
