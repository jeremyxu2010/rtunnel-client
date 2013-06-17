/*
 * main.cpp
 *
 *  Created on: 2013Äê6ÔÂ16ÈÕ
 *      Author: jeremy
 */

#include "clientbootstrap.hpp"
#include <log4cpp/PropertyConfigurator.hh>

int main(int ac, char* av[]) {

	std::string initFileName = "log4cpp.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);

	rtunnel::client_bootstrap bootstrap(ac, av);
	bootstrap.start();

	return 0;
}


