/*
 * clientbootstrap.cpp
 *
 *  Created on: 2013年6月16日
 *      Author: jeremy
 */

#include "clientbootstrap.hpp"
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <string>

namespace rtunnel {

log4cpp::Category& client_bootstrap::logger = log4cpp::Category::getInstance(std::string("rtunnel.client_bootstrap"));

client_bootstrap::client_bootstrap(int ac, char* av[]):mainKeepRunning(false), keepRunning(false), clientConfig() {
	clientConfig.init(ac, av);
}

void client_bootstrap::start(){
	this->mainKeepRunning = true;
	while(this->mainKeepRunning){
		this->p_clientLogicThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&rtunnel::client_bootstrap::runClientLogic, this)));
		try{
			this->p_clientLogicThread->join();
			boost::this_thread::sleep(boost::posix_time::seconds(5));
		} catch(boost::thread_exception & e){
			client_bootstrap::logger.debug("thread join error.");
		}
	}
}

void client_bootstrap::runClientLogic(){
	this->keepRunning = true;
	client_bootstrap::logger.info(str(boost::format("begin to establish tunnel with transit server(forwardPort=%1%).") % this->clientConfig.forwardPort));
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(this->clientConfig.rtunnelServerHost, boost::lexical_cast<std::string>(this->clientConfig.rtunnelServerPort));
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	this->p_socket = boost::shared_ptr<tcp::socket>(new tcp::socket(io_service));
	boost::system::error_code ec;
	boost::asio::connect(*(this->p_socket.get()), endpoint_iterator, ec);
	if(ec){
		client_bootstrap::logger.info("connect to transit server fails, will try to reestablish it.");
		this->cleanup();
		return;
	}
}

void client_bootstrap::stop(){
	this->mainKeepRunning = false;
	if(this->p_clientLogicThread.get() != NULL && !this->p_clientLogicThread->interruption_requested()){
		this->p_clientLogicThread->interrupt();
	}
	this->cleanup();
	boost::this_thread::sleep(boost::posix_time::seconds(5));
}

void client_bootstrap::cleanup(){
	client_bootstrap::logger.debug("start cleanup.");
	keepRunning = false;
	if(this->p_socket.get() != NULL){
		this->p_socket->close();
	}
}

client_bootstrap::~client_bootstrap() {
	this->stop();
	log4cpp::Category::shutdown();
}

} /* namespace rtunnel */
