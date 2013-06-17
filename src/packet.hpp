/*
 * packet.hpp
 *
 *  Created on: 2013Äê6ÔÂ16ÈÕ
 *      Author: jeremy
 */

#ifndef PACKET_HPP_
#define PACKET_HPP_

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <vector>

namespace rtunnel {

class packet {
public:
	const static int HEART_BEAT = 0x00;
	const static int ACK_HEART_BEAT = 0x01;
	const static int CREATE_TCP_SERVER = 0x02;
	const static int ACK_CREATE_TCP_SERVER = 0x03;
	const static int NEW_TCP_SOCKET = 0x04;
	const static int ACK_NEW_TCP_SOCKET = 0x05;
	const static int DATA = 0x06;
	const static int CLOSE_TUNNEL = 0x07;
	const static int DH_KEY = 0x08;
	const static int ACK_DH_KEY = 0x09;
	const static int TUNNEL_MODE = 0x0a;
	const static int ACK_TUNNEL_MODE = 0x0b;

	const static int COMPRESSED = 0x80;
	const static int ENCRYPTED = 0x40;
	const static int HIGH_MASK = 0xc0;

	packet(int size);

	void setProtocol(int protocol);
	bool isProtocol(int protocol);

	int getType();
	bool isCompressed();
	void setCompressed();
	bool isEncrypted();
	void setEncrypted();
	void clearEncrypted();
	void clearCompressed();
	void clearHeader();
	bool isHeartBeat();
	void setHeartBeat();
	bool isAckHeartBeat();
	void setAckHeartBeart();
	void clearBody();
	void clear();
	int extractBytes(std::vector<unsigned char> destVec, int start, int len);
	int extractBytes(std::vector<unsigned char> destVec);
	void feedBytes(std::vector<unsigned char> srcVec, int start, int len);
	void feedBytes(std::vector<unsigned char> srcVec);
	void feedLong(unsigned long v);
	unsigned long extractLong();
	unsigned int extractInt();
	void feedInt(unsigned int v);
	std::vector<unsigned char> toBytes();
	unsigned char byteAt(int index);
	void encode();
	void decode();
	void fillHeader();
	void readHeader();
	void resize(int size);
	void ensureSize(int size);
	int getDataLen();
	void setDataLen(int len);
	std::string toString();
	std::string toDebugString();
	std::vector<unsigned char> getBuffer();
	int getIndex();
	void readPacket(std::vector<unsigned char> buf, int length);
	boost::asio::const_buffer wrapPacket();
	boost::asio::const_buffer wrapPacketData();
	void readData(std::vector<unsigned char> buf);

	static void setControlPacket(packet p, int type, std::vector<unsigned char> resultBytes);
	static void setControlPacket(packet p, int type, int intResult);
	static void setControlPacket(packet p, int type, long longResult);
	static packet fillHeartBeatPacket(packet p);
	static packet fillACKHeartBeatPacket(packet p, std::vector<unsigned char> timeBytes);
	static packet fillCloseTunnelPacket(packet p);
	static packet fillDHKeyPacket(packet p, int protocol, std::vector<unsigned char> keyBytes);

	virtual ~packet();
private:
	static const int PACKET_MAX_SIZE;
	static const int PROTOCOL_BIT_MASK;
	static const int HEAD_SIZE;
	static const bool DEBUG;
	static const int BUFFER_MARGIN;
	static const int CLEAR_PROTOCOL_BIT_MASK;
	std::vector<unsigned char> bufferVec;
	int index;
	int readIndex;
	int type;
	void init(int level);
	void compress();
	void uncompress();
	void encrypt();
	void decrypt();
};

} /* namespace rtunnel */
#endif /* PACKET_HPP_ */
