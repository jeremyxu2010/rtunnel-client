/*
 * clientbootstrap.hpp
 *
 *  Created on: 2013Äê6ÔÂ16ÈÕ
 *      Author: jeremy
 */

#ifndef CLIENTBOOTSTRAP_HPP_
#define CLIENTBOOTSTRAP_HPP_

#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <log4cpp/Category.hh>
#include "clientconfig.hpp"

using boost::asio::ip::tcp;

namespace rtunnel {

class client_bootstrap {
public:
	client_bootstrap(int ac, char* av[]);
	void start();
	void stop();
	virtual ~client_bootstrap();
private:
	void runClientLogic();
	void cleanup();
	rtunnel::client_config clientConfig;
	bool mainKeepRunning;
	bool keepRunning;
	static log4cpp::Category& logger;
	boost::shared_ptr<tcp::socket> p_socket;
	boost::shared_ptr<boost::thread> p_clientLogicThread;
	boost::asio::io_service io_service;
};
} /* namespace rtunnel */
#endif /* CLIENTBOOTSTRAP_HPP_ */
