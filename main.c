/*
 *  Program entry and command line parsing
 *
 *  Copyright (C) 2007-2008 Xavier Carcelle <xavier.carcelle@gmail.com>
 *		    	    Florian Fainelli <florian@openwrt.org>
 *			    Nicolas Thill <nico@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL.  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so.  If you
 *  do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "faifa.h"
#include "faifa_compat.h"

#ifndef FAIFA_PROG
#define FAIFA_PROG "faifa"
#endif

/* Command line arguments storing */
int opt_help = 0;
int opt_interactive = 0;
int opt_key = 0;
extern FILE *err_stream;
extern FILE *out_stream;
extern FILE *in_stream;

/**
 * error - display error message
 */
static void error(char *message)
{
	fprintf(stderr, "%s: %s\n", FAIFA_PROG, message);
}

/**
 * usage - show the program usage
 */
static void usage(void)
{
	fprintf(stderr, "-i : interface\n"
			"-m : show menu (no option required)\n"
			"-a : station MAC address\n"
			"-k : network key\n"
			"-v : be verbose (default: no)\n"
			"-e : error stream (default: stderr)\n"
			"-o : output stream (default: stdout)\n"
			"-s : input stream (default: stdin)\n"
			"-h : this help\n");
}

extern void menu(faifa_t *faifa);
extern void set_key(char *macaddr);

/**
 * main - main function of faifa
 * @argc:	number of arguments
 * @argv:	array of arguments
 */
int main(int argc, char **argv)
{
	faifa_t *faifa;
	char *opt_ifname = NULL;
	char *opt_macaddr = NULL;
	char *opt_err_stream = NULL;
	char *opt_out_stream = NULL;
	char *opt_in_stream = NULL;
	int opt_verbose = 0;
	int c;
	int ret = 0;
	u_int8_t addr[ETHER_ADDR_LEN] = { 0 };

	fprintf(stdout, "Faifa for HomePlug AV (GIT revision %s)\n\n", GIT_REV);

	if (argc < 2) {
		usage();
		return -1;
	}

	while ((c = getopt(argc, argv, "i:ma:k:ve:o:s:h")) != -1) {
		switch (c) {
			case 'i':
				opt_ifname = optarg;
				break;
			case 'm':
				opt_interactive = 1;
				break;
			case 'a':
				opt_macaddr = optarg;
				break;
			case 'k':
				opt_key = 1;
				break;
			case 'v':
				opt_verbose = 1;
				break;
			case 'e':
				opt_err_stream = optarg;
				break;
			case 'o':
				opt_out_stream = optarg;
				break;
			case 's':
				opt_in_stream = optarg;
				break;
			case 'h':
			default:
				opt_help = 1;
				break;
		}
	}

	if (opt_help) {
		usage();
		return -1;
	}

	if (opt_ifname == NULL)
		opt_ifname = "eth0";

	if (opt_err_stream == NULL)
		err_stream = stderr;
	else {
		err_stream = fopen(opt_err_stream, "w+");
		if (!err_stream) {
			perror("err_stream");
			return -1;
		}
	}

	if (opt_out_stream == NULL)
		out_stream = stdout;
	else {
		out_stream = fopen(opt_out_stream, "w+");
		if (!out_stream) {
			perror("out_stream");
			return -1;
		}
	}

	if (opt_in_stream == NULL)
		in_stream = stdin;
	else {
		in_stream = fopen(opt_in_stream, "rb");
		if (!in_stream) {
			perror("in_stream");
			return -1;
		}
	}

	faifa = faifa_init();
	if (faifa == NULL) {
		error("can't initialize Faifa library");
		return -1;
	}

	if (faifa_open(faifa, opt_ifname) == -1) {
		error(faifa_error(faifa));
		faifa_free(faifa);
		return -1;
	}

	faifa_set_verbose(faifa, opt_verbose);

	if (opt_macaddr) {
		ret = faifa_parse_mac_addr(faifa, opt_macaddr, addr);
		if (ret < 0) {
			error(faifa_error(faifa));
			goto out_error;
		}

		faifa_set_dst_addr(faifa, addr);
	}

	if (opt_interactive)
		menu(faifa);

out_error:
	faifa_close(faifa);
	faifa_free(faifa);

	return ret;
}
