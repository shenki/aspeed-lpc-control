/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2018 IBM Corp. */

#ifndef ASPEED_LPC_H
#define ASPEED_LPC_H

#include <stdint.h>

struct astlpc_ctx;

struct astlpc_ctx *astlpc_alloc(void);
int astlpc_init(struct astlpc_ctx *context, const char *path);
void astlpc_free(struct astlpc_ctx *context);
int astlpc_use_flash(struct astlpc_ctx *context, uint32_t flash_size);
int astlpc_use_mem(struct astlpc_ctx *context, const char *path);

#endif /* ASPEED_LPC_H */
