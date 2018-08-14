/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2018 IBM Corp. */

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "aspeed_lpc.h"
#include "config.h"

static struct option options[] = {
	{"size",	required_argument,	0, 's' },
	{"version",	no_argument,		0, 'V' },
	{"verbose",	no_argument,		0, 'v' },
	{"help",	no_argument,		0, 'h' },
	{0,		0,			0,  0  },
};

static void show_version(void)
{
	printf("%s\n", PACKAGE_STRING);
}

static void show_help(const char *name)
{
	printf("Usage: %s [--file|-f FILE] [-v|--version] [-V|--verbose] [-h|--help]\n\n", name);
	printf("  -f, --file\tFILE is the PNOR to be loaded into memory\n");
	printf("  -V, --version\tdisplay version information and exit\n");
	printf("  -v, --verbose\tbe verbose in output\n");
	printf("  -h, --help\tdisplay this help text and exit\n");
}

int main(int argc, char **argv)
{
	struct astlpc_ctx *ctx;
	const char *file;
	unsigned char c;
	char *endptr;
	int rc;

	do {
		c = getopt_long(argc, argv, "f:vVh", options, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			file = optarg;
			break;
		case 'v':
			astlpc_set_verbose();
			break;
		case 'V':
			show_version();
			return 0;
		case 'h':
		default:
			show_help(basename(argv[0]));
			if (c == 'h')
				return 0;
			exit(EXIT_FAILURE);
		};
	} while (c != EOF);

	if (!file) {
		show_help(basename(argv[0]));
		exit(EXIT_FAILURE);
	}

	ctx = astlpc_alloc();
	assert(ctx);

	rc = astlpc_init(ctx, NULL);
	if (rc < 0)
		return rc;

	rc = astlpc_use_mem(ctx, file);
	if (rc < 0)
		return rc;

	astlpc_free(ctx);

	return 0;
}
