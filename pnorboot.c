/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2018 IBM Corp. */

#include <assert.h>
#include <stddef.h>

#include "aspeed_lpc.h"

int main(int argc, char **argv)
{
	int rc;
	struct astlpc_ctx *ctx = astlpc_alloc();
	assert(ctx);

	rc = astlpc_init(ctx, NULL);
	if (rc < 0)
		return rc;

	rc = astlpc_use_flash(ctx, 32 * 1024 * 1024);
	if (rc < 0)
		return rc;

	astlpc_free(ctx);

	return 0;
}
