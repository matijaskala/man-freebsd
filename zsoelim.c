/*	$NetBSD: soelim.c,v 1.15 2016/09/05 00:40:29 sevan Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

/*
 * soelim - a filter to process n/troff input eliminating .so's
 *
 * Author: Bill Joy UCB July 8, 1977
 *
 * This program eliminates .so's from a n/troff input stream.
 * It can be used to prepare safe input for submission to the
 * phototypesetter since the software supporting the operator
 * doesn't let him do chdir.
 *
 * This is a kludge and the operator should be given the
 * ability to do chdir.
 *
 * This program is more generally useful, it turns out, because
 * the program tbl doesn't understand ".so" directives.
 */
#include <sys/param.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	STDIN_NAME	"-"

struct path {
	char **list;
	size_t n, c;
};

static void	 process(struct path *, FILE *);
static void	 initpath(struct path *);
static void	 addpath(struct path *,  const char *);
static FILE	*openpath(struct path *, const char *);


static void
initpath(struct path *p)
{
	p->list = NULL;
	p->n = p->c = 0;
}

static void
addpath(struct path *p, const char *dir)
{
	char **n;

	if (p->list == NULL || p->n <= p->c - 2) {
		n = realloc(p->list, (p->n + 10) * sizeof(p->list[0]));
		if (n == NULL)
			err(1, NULL);
		p->list = n;
		p->n += 10;
	}

	if ((p->list[p->c++] = strdup(dir)) == NULL)
		err(1, NULL);

	p->list[p->c] = NULL;
}

static FILE *
openpath(struct path *p, const char *name)
{
	char cmd[MAXPATHLEN + 16];
	const char *f;
	FILE *fp;
	size_t i;
	const char *progname = getprogname();

	if (*name == '/' || p->c == 0) {
		if ((fp = fopen(name, "r")))
			return fp;
		if (*progname != 'z')
			return NULL;
		int save_errno = errno;
		(void)snprintf(cmd, sizeof(cmd), "zcat %s.gz", name);
		if (access(cmd + 5, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "bzcat %s.bz", name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "bzcat %s.bz2", name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "xzcat %s.xz", name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "lzcat %s.lzma", name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "lzip -dc %s.lz", name);
		if (access(cmd + 9, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "brcat %s.br", name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "zstdcat %s.zst", name);
		if (access(cmd + 8, F_OK) != -1)
			return popen(cmd, "r");
		errno = save_errno;
		return NULL;
	}

	for (i = 0; i < p->c; i++) {
		if (p->list[i][0] == '\0')
			f = name;
		else {
			(void)snprintf(cmd, sizeof(cmd), "%s/%s", 
			    p->list[i], name);
			f = cmd;
		}
		if ((fp = fopen(f, "r")))
			return fp;
		if (*progname != 'z')
			continue;
		int save_errno = errno;
		(void)snprintf(cmd, sizeof(cmd), "zcat %s/%s.z", 
		    p->list[i], name);
		if (access(cmd + 5, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "bzcat %s/%s.bz", 
		    p->list[i], name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "bzcat %s/%s.bz2", 
		    p->list[i], name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "xzcat %s/%s.xz", 
		    p->list[i], name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "lzcat %s/%s.lzma", 
		    p->list[i], name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "lzip -dc %s/%s.lz", 
		    p->list[i], name);
		if (access(cmd + 9, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "brcat %s/%s.br", 
		    p->list[i], name);
		if (access(cmd + 6, F_OK) != -1)
			return popen(cmd, "r");
		(void)snprintf(cmd, sizeof(cmd), "zcat %s/%s.zst", 
		    p->list[i], name);
		if (access(cmd + 8, F_OK) != -1)
			return popen(cmd, "r");
		errno = save_errno;
	}
	return NULL;
}

int
main(int argc, char *argv[])
{
	struct path p;
	int c;

	initpath(&p);

	while ((c = getopt(argc, argv, "I:")) != -1)
		switch (c) {
		case 'I':
			addpath(&p, optarg);
			break;
		default:
			(void)fprintf(stderr,
			    "usage: %s [-I<dir>] [files...]\n",
			    getprogname());
			exit(1);
		}

	addpath(&p, ".");

	argc -= optind;
	argv += optind;
			
	if (argc == 0) {
		(void)process(&p, stdin);
		exit(0);
	}
	do {
		if (!strcmp(argv[0], STDIN_NAME))
			(void)process(&p, stdin);
		else {
			FILE *soee = openpath(&p, argv[0]);
			if (soee)
				(void)process(&p, soee);
			else
				warn("Cannot open `%s'", argv[0]);
		}
		argv++;
		argc--;
	} while (argc > 0);
	exit(0);
}

void
process(struct path *p, FILE *file)
{
	char *cp;
	int c;
	char fname[MAXPATHLEN];
	FILE *soee;
	int isfile;

	for (;;) {
		c = getc(file);
		if (c == EOF)
			break;
		if (c != '.')
			goto simple;
		c = getc(file);
		if (c != 's') {
			putchar('.');
			goto simple;
		}
		c = getc(file);
		if (c != 'o') {
			printf(".s");
			goto simple;
		}
		do
			c = getc(file);
		while (c == ' ' || c == '\t');
		cp = fname;
		isfile = 0;
		for (;;) {
			switch (c) {

			case ' ':
			case '\t':
			case '\n':
			case EOF:
				goto donename;

			default:
				if (cp == &fname[sizeof(fname)-1]) {
					*cp = 0;
					printf(".so %s\n", fname);
					goto simple;
				}
				*cp++ = c;
				c = getc(file);
				isfile++;
				continue;
			}
		}
donename:
		if (cp == fname) {
			printf(".so");
			goto simple;
		}
		*cp = 0;
		soee = openpath(p, fname);
		if (soee == NULL) {
			warn("Cannot open `%s'", fname);
			if (isfile)
				printf(".so %s\n", fname);
		} else {
			process(p, soee);
			fclose(soee);
		}
		continue;
simple:
		if (c == EOF)
			break;
		putchar(c);
		if (c != '\n') {
			c = getc(file);
			goto simple;
		}
	}
}
