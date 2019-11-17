#include "jerry/znode/znode.h"
#include <iostream>
#include <zsystem/zswait.h>

int main(int a_argc, char **a_argv) {
	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION


	opterr = 0;
	int c;
	while((c = getopt (a_argc, a_argv, "dc:")) != -1) {
		switch(c) {
		case 'd':
			jerry::znode::zn__is_daemon = true;
			break;
		case 'c':
			std::cout << "test." << std::endl;
			jerry::znode::zn__config_file = optarg;
			break;
		case '?':
			if (optopt == 'c')
				std::cerr << "Option -" << (char) optopt << " requires an argument." << std::endl;
			else if (isprint (optopt))
				std::cerr << "Unknown option `-" << (char) optopt << "'." << std::endl;
			else
				std::cerr << "Unknown option character `-" << (unsigned int) optopt << "'." << std::endl;
			return 1;
		default:
			abort ();
		}
	}

	if(jerry::znode::zn__is_daemon) {
		daemon(0, 0);
	}
	return zs__execute(0, jerry::znode::Main);
}
