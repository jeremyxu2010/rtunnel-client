/*
 * clientconfig.hpp
 *
 *  Created on: 2013Äê6ÔÂ16ÈÕ
 *      Author: jeremy
 */

#ifndef CLIENTCONFIG_HPP_
#define CLIENTCONFIG_HPP_

#include <string>
#include <boost/program_options.hpp>

using namespace std;

namespace rtunnel {

class client_config {
public:
	client_config();
	void init(int ac, char* av[]);
	virtual ~client_config();
	string rtunnelServerHost;
	int rtunnelServerPort;
	string tcpHost;
	int tcpPort;
	int forwardPort;
};

}  // namespace rtunnel

#endif /* CLIENTCONFIG_HPP_ */
