/*
 * clientconfig.cpp
 *
 *  Created on: 2013年6月16日
 *      Author: jeremy
 */

#include "clientconfig.hpp"

using namespace std;


namespace rtunnel {

namespace po = boost::program_options;

client_config::client_config():rtunnelServerHost(), rtunnelServerPort(0), tcpHost(), tcpPort(0), forwardPort(0){
}

void client_config::init(int ac, char* av[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
			("help", "print this help message")
			("rtunnelServerHost,h", po::value<string>(), "rtunnel server host")
			("rtunnelServerPort,p", po::value<int>(), "rtunnel server port")
			("tcpHost", po::value<string>(), "tcp host")
			("tcpPort", po::value<int>(), "tcp port")
			("forwardPort", po::value<int>(), "forward port");

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << "\n";
		exit(1);
	}

	if (vm.count("rtunnelServerHost")) {
		this->rtunnelServerHost = vm["rtunnelServerHost"].as<string>();
	} else {
		cout << "rtunnelServerHost was not set." << endl;
		exit(1);
	}

	if (vm.count("rtunnelServerPort")) {
		this->rtunnelServerPort = vm["rtunnelServerPort"].as<int>();
	} else {
		cout << "rtunnelServerPort was not set." << endl;
		exit(1);
	}

	if (vm.count("tcpHost")) {
		this->tcpHost = vm["tcpHost"].as<string>();
	} else {
		cout << "tcpHost was not set." << endl;
		exit(1);
	}

	if (vm.count("tcpPort")) {
		this->tcpPort = vm["tcpPort"].as<int>();
	} else {
		cout << "tcpPort was not set." << endl;
		exit(1);
	}

	if (vm.count("forwardPort")) {
		this->forwardPort = vm["forwardPort"].as<int>();
	} else {
		cout << "forwardPort was not set." << endl;
		exit(1);
	}
}

client_config::~client_config() {
	// TODO Auto-generated destructor stub
}

}  // namespace rtunnel


