// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2018 IBM Corp.

#define _GNU_SOURCE
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <stdlib.h>

#include <linux/aspeed-lpc-ctrl.h>
#include <mtd/mtd-abi.h>

#include "aspeed_lpc.h"

#define DEFAULT_LPC_CTRL_PATH	"/dev/aspeed-lpc-ctrl"

/**
 * @mem: Reserved Memory Region
 * @mem_size: Reserved Mem Size (bytes)
 * @lpc_base: LPC Bus Base Address (bytes)
 * @mtd_info: Actual Flash Info
 * @fd: File descriptor for aspeed-lpc-ctrl device
 * @path: Path to aspeed-lpc-ctrl device
 */
struct astlpc_ctx {
	void *mem;
	uint32_t mem_size;
	uint32_t lpc_base;
	struct mtd_info_user mtd_info;
	int fd;
	const char *path;
};

#ifdef DEBUG
#define debug(...)		\
do {				\
	printf(__VA_ARGS__);	\
} while (0)
#else
#define debug(...) do { } while (0)
#endif

#define min(a, b) \
        ({ \
                typeof(a) _a = (a); \
                typeof(b) _b = (b); \
                __builtin_types_compatible_p(typeof(_a), typeof(_b)); \
                _a < _b ? _a : _b; \
        })

struct astlpc_ctx *astlpc_alloc(void)
{
	return calloc(1, sizeof(struct astlpc_ctx));
}

int astlpc_init(struct astlpc_ctx *ctx, const char *path)
{
	struct aspeed_lpc_ctrl_mapping map = {
		.window_type = ASPEED_LPC_CTRL_WINDOW_MEMORY,
		.window_id = 0, /* There's only one */
		.flags = 0,
		.addr = 0,
		.offset = 0,
		.size = 0
	};

	if (!path) {
		debug("No path set, using default %s\n", DEFAULT_LPC_CTRL_PATH);
		path = DEFAULT_LPC_CTRL_PATH;
	}

	/* Open LPC Device */
	debug("Opening %s\n", path);
	ctx->fd = open(path, O_RDWR | O_SYNC);
	if (ctx->fd < 0) {
		err(1, "%s O_RDWR", path);
		return -errno;
	}

	/* Find Size of Reserved Memory Region */
	debug("Getting buffer size...\n");
	if (ioctl(ctx->fd, ASPEED_LPC_CTRL_IOCTL_GET_SIZE, &map) < 0) {
		err(1, "Couldn't get lpc control buffer size");
		return -errno;
	}

	ctx->mem_size = map.size;
	/* Map at the top of the 28-bit LPC firmware address space-0 */
	ctx->lpc_base = 0x0FFFFFFF & -ctx->mem_size;

	/* mmap the Reserved Memory Region */
	debug("Mapping in 0x%.8x bytes of %s\n", ctx->mem_size, path);
	ctx->mem = mmap(NULL, ctx->mem_size, PROT_READ | PROT_WRITE,
				MAP_SHARED, ctx->fd, 0);
	if (ctx->mem == MAP_FAILED) {
		err(1, "Failed to map %s", path);
		return -errno;
	}

	return 0;
}

void astlpc_free(struct astlpc_ctx *ctx)
{
	if (ctx->mem) {
		munmap(ctx->mem, ctx->mem_size);
	}
	close(ctx->fd);
	free(ctx);
}

/* Point the lpc bus mapping to the actual flash device */
int astlpc_use_flash(struct astlpc_ctx *ctx, uint32_t flash_size)
{
	struct aspeed_lpc_ctrl_mapping map = {
		.window_type = ASPEED_LPC_CTRL_WINDOW_FLASH,
		.window_id = 0, /* Theres only one */
		.flags = 0,
		/*
		 * The mask is because the top nibble is the host LPC FW space,
		 * we want space 0.
		 */
		.addr = 0x0FFFFFFF & -flash_size,
		.offset = 0,
		.size = flash_size
	};

	printf("Pointing HOST LPC bus at the flash\n");
	printf("Assuming %dMB of flash: HOST LPC 0x%08x\n",
		flash_size >> 20, map.addr);

	if (ioctl(ctx->fd, ASPEED_LPC_CTRL_IOCTL_MAP, &map)
			== -1) {
		err(1, "Failed to point the LPC BUS at the actual flash");
		return -errno;
	}

	return 0;
}

/* Point the lpc bus mapping to the reserved memory region */
int astlpc_use_mem(struct astlpc_ctx *ctx, const char *file)
{
	int fd;
	void *mem;
	int size_read, size;

	struct aspeed_lpc_ctrl_mapping map = {
		.window_type = ASPEED_LPC_CTRL_WINDOW_MEMORY,
		.window_id = 0, /* There's only one */
		.flags = 0,
		.addr = ctx->lpc_base,
		.offset = 0,
		.size = ctx->mem_size
	};

	printf("Pointing HOST LPC bus at memory region %p of size 0x%.8x\n",
			ctx->mem, ctx->mem_size);
	printf("LPC address 0x%.8x\n", map.addr);

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		err(1, "%s", file);
		return -errno;
	}
	mem = ctx->mem;
	size = ctx->mem_size;
	do {
		size_read = read(fd, mem, min(64 * 1204, size));
		if (size_read < 0)
			break;
		size -= size_read;
		mem += size_read;
	} while (size && size_read);

	close(fd);
	if (size > 0) {
		err(1, "Failed to read entire file\n");
		return -ENOSPC;
	}

	if (ioctl(ctx->fd, ASPEED_LPC_CTRL_IOCTL_MAP, &map)) {
		err(1, "Failed to point the LPC BUS to memory");
		return -errno;
	}

	return 0;
}
