/*
 * packet.cpp
 *
 *  Created on: 2013年6月16日
 *      Author: jeremy
 */

#include "packet.hpp"
#include <exception>
#include <math.h>

namespace rtunnel {

const int packet::PACKET_MAX_SIZE = 102400;
const int packet::PROTOCOL_BIT_MASK = 0x0f;
const int packet::HEAD_SIZE = 5;
const bool packet::DEBUG = false;
const int packet::BUFFER_MARGIN = 16 + packet::HEAD_SIZE + 32;
const int packet::CLEAR_PROTOCOL_BIT_MASK = 0xff - packet::PROTOCOL_BIT_MASK;

packet::packet(int size):type(0), index(HEAD_SIZE), readIndex(HEAD_SIZE){
	if (size >= PACKET_MAX_SIZE) {
		throw new std::invalid_argument(str(boost::format("packet size %1% larger than %2%") % size % PACKET_MAX_SIZE));
	}
	if (size < 0) {
		throw new std::invalid_argument(str(boost::format("packet size %1% must not be negtive.") % size));
	}
	this -> bufferVec = std::vector<unsigned char>(size + BUFFER_MARGIN);
}

void packet::setProtocol(int protocol){
	if (protocol < 0 || protocol > PROTOCOL_BIT_MASK) {
		throw new std::invalid_argument(str(boost::format("Unsupported protocol %1%, protocol type must be [0, %2%]") % protocol % PROTOCOL_BIT_MASK));
	}
	// clean previous protocol
	this->type &= CLEAR_PROTOCOL_BIT_MASK;
	this->type |= protocol;
	this->bufferVec[0] = (unsigned char) this->type;
}

bool packet::isProtocol(int protocol){
	if (protocol < 0 || protocol > PROTOCOL_BIT_MASK) {
		throw new std::invalid_argument(str(boost::format("Unsupported protocol %1%, protocol type must be [0, %2%]") % protocol % PROTOCOL_BIT_MASK));
	}
	return (this->type & PROTOCOL_BIT_MASK) == protocol;
}

void packet::init(int level){
	// not implemented!
}

/**
 * 一个byte的协议头
 *
 * @return
 */
int packet::getType() {
	return this->type;
}

/**
 *
 * @return true if this packet's data is compressed.
 */
bool packet::isCompressed() {
	return (this->type & COMPRESSED) != 0;
}

/**
 * set to compress this packet's data when invoke {@link #encode()} method
 */
void packet::setCompressed() {
	this->init(6);
	this->type |= COMPRESSED;
}

/**
 *
 * @return true if this packet's data is encrypted.
 */
bool packet::isEncrypted() {
	return (this->type & ENCRYPTED) != 0;
}

/**
 * set to encrypt this packet's data when invoke {@link #encode()} method
 */
void packet::setEncrypted() {
	type |= ENCRYPTED;
}

/**
 * set not to encrypt this packet's data when invoke {@link #encode()}
 * method
 */
void packet::clearEncrypted() {
	type &= 0xBF;
}

/**
 * set not to compress this packet's data when invoke {@link #encode()}
 * method
 */
void packet::clearCompressed() {
	type &= 0x7F;
}

/**
 * clear header information
 */
void packet::clearHeader() {
	type = 0;
	unsigned char zero = 0;
	for (int i = 0; i < HEAD_SIZE; i++) {
		this->bufferVec[i] = zero;
	}
}

/**
 *
 * @return true if this is a HEART_BEAT packet
 */
// type & 0011
bool packet::isHeartBeat() {
	return isProtocol(HEART_BEAT);
}

/**
 * set this packet for HEART_BEAT
 */
void packet::setHeartBeat() {
	setProtocol(HEART_BEAT);
}

/**
 *
 * @return true if this is a ACK_HEART_BEAT packet
 */
// type & 0011
bool packet::isAckHeartBeat() {
	return isProtocol(ACK_HEART_BEAT);
}

/**
 * set this packet for ACK_HEART_BEAT
 */
void packet::setAckHeartBeart() {
	setProtocol(ACK_HEART_BEAT);
}

/**
 * clear packet body
 */
void packet::clearBody() {
	index = HEAD_SIZE;
	readIndex = HEAD_SIZE;
}

/**
 * clear this packet for reusing.
 */
void packet::clear() {
	clearHeader();
	clearBody();
}

/**
 * extract bytes from this buffer to a byte array,increasing readIndex
 *
 * @param dest
 * @param start
 * @param len
 * @return data length actually read or 0 if no bytes available.
 */
int packet::extractBytes(std::vector<unsigned char> destVec, int start, int len) {
	int rlen = std::min(index - readIndex, len);
	if (rlen > 0) {
		std::copy(bufferVec.begin() + readIndex, bufferVec.begin() + readIndex + rlen, destVec.begin());
		readIndex += rlen;
		return rlen;
	} else {
		return 0;
	}
}

/**
 * extract bytes from this buffer to a byte array,increasing readIndex
 *
 * @see #extractBytes(byte[], int, int)
 * @param dest
 * @return data length actually read or 0 if no bytes available.
 */
int packet::extractBytes(std::vector<unsigned char> destVec) {
	return extractBytes(destVec, 0, destVec.size());
}

/**
 * feed bytes from src to this buffer,increasing index
 *
 * @param src
 * @param start
 * @param len
 */
void packet::feedBytes(std::vector<unsigned char> srcVec, int start, int len) {
	// TODO buffer full check.
	ensureSize(getDataLen() + len);
	std::copy(srcVec.begin() + start, srcVec.begin() + start + len, bufferVec.begin() + index);
	index += len;
}

/**
 * 把一个byte数组填入数据区
 *
 * @see #feedBytes(byte[], int, int)
 * @param src
 */
void packet::feedBytes(std::vector<unsigned char> srcVec) {
	feedBytes(srcVec, 0, srcVec.size());
}

/**
 * 把一个long整数填入数据区
 *
 * @param v
 */
void packet::feedLong(unsigned long v) {
	ensureSize(getDataLen() + 8);
	bufferVec[index++] = (unsigned char) (v >> 56);
	bufferVec[index++] = (unsigned char) (v >> 48);
	bufferVec[index++] = (unsigned char) (v >> 40);
	bufferVec[index++] = (unsigned char) (v >> 32);
	bufferVec[index++] = (unsigned char) (v >> 24);
	bufferVec[index++] = (unsigned char) (v >> 16);
	bufferVec[index++] = (unsigned char) (v >> 8);
	bufferVec[index++] = (unsigned char) (v >> 0);
}

/**
 * 从数据区抽出一个long（8 bytes），增加readIndex
 *
 * @return
 * @throws IOException
 *             如果数据区的长度小于8
 */
unsigned long packet::extractLong(){
	if (index - readIndex < 8)
		throw new std::out_of_range("Insufficient data for long");
	return (unsigned long) ((unsigned long) (0xff & bufferVec[readIndex++]) << 56
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 48
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 40
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 32
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 24
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 16
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 8
			| (unsigned long) (0xff & bufferVec[readIndex++]) << 0);
}

/**
 * 从数据区抽出一个int（4 bytes），增加readIndex
 *
 * @return
 * @throws IOException
 *             如果数据区的长度小于4
 */
unsigned int packet::extractInt(){
	if (index - readIndex < 4)
		throw new std::out_of_range("Insufficient data for int");
	return (unsigned int) ((unsigned int)((bufferVec[readIndex++] & 0xff) << 24)
			| (unsigned int)((bufferVec[readIndex++] & 0xff) << 16)
			| (unsigned int)((bufferVec[readIndex++] & 0xff) << 8)
			| (unsigned int)(bufferVec[readIndex++] & 0xff));
}

/**
 * 把一个int整数填入数据区，增加index
 *
 * @param v
 */
void packet::feedInt(unsigned int v) {
	ensureSize(getDataLen() + 4);
	bufferVec[index++] = (unsigned char) (v >> 24);
	bufferVec[index++] = (unsigned char) (v >> 16);
	bufferVec[index++] = (unsigned char) (v >> 8);
	bufferVec[index++] = (unsigned char) (v >> 0);
}

/**
 * test use
 *
 * @return
 */
std::vector<unsigned char> packet::toBytes() {
	std::vector<unsigned char> bytes(index - HEAD_SIZE);
	std::copy(bufferVec.begin() + HEAD_SIZE, bufferVec.begin() + index, bytes.begin());
	return bytes;
}

/**
 * 数据区特定位置上的一个byte的数据
 *
 * @param index
 * @return
 */
unsigned char packet::byteAt(int index) {
	return bufferVec[HEAD_SIZE + index];
}

/**
 * 根据设置的标识位进行压缩、加密等等
 */
void packet::encode() {
	// 先压缩后加密
	if (isCompressed()) {
		compress();
	}
	if (isEncrypted()) {
		encrypt();
	}
}

/**
 * 根据标识位进行解密和解压
 */
void packet::decode() {
	// 先解密后解压
	if (isEncrypted()) {
		decrypt();
		clearEncrypted();
	}
	if (isCompressed()) {
		uncompress();
		clearCompressed();
	}
}

void packet::fillHeader() {
	int len = index - HEAD_SIZE;
	bufferVec[0] = (unsigned char) (unsigned int)type;
	bufferVec[1] = (unsigned char) ((unsigned int)len >> 24);
	bufferVec[2] = (unsigned char) ((unsigned int)len >> 16);
	bufferVec[3] = (unsigned char) ((unsigned int)len >> 8);
	bufferVec[4] = (unsigned char) ((unsigned int)len);
}

void packet::readHeader() {
	this->type = bufferVec[0];
	int len = (((unsigned int)bufferVec[1] << 24) & 0xff000000)
			| (((unsigned int)bufferVec[2] << 16) & 0x00ff0000)
			| (((unsigned int)bufferVec[3] << 8) & 0x0000ff00) | (((unsigned int)bufferVec[4]) & 0x000000ff);
	index = len + HEAD_SIZE;
}

/**
 * resize this packet's buffer to size.
 *
 * @param size
 */
void packet::resize(int size) {
	std::vector<unsigned char> newBufferVec(size);
	std::copy(bufferVec.begin(), bufferVec.begin()+index, newBufferVec.begin());
	this->bufferVec = newBufferVec;
}

/**
 * ensure size for packet buffer
 *
 * @param size
 */
void packet::ensureSize(int size) {
	// TODO check buffer full,i.e. reach PACKET_MAX_SIZE
	if (bufferVec.size() < size + BUFFER_MARGIN) {
		int resize = std::min(PACKET_MAX_SIZE + BUFFER_MARGIN,
				std::max((int)bufferVec.size() << 1, size + BUFFER_MARGIN));
		if (resize < size) {
			throw new std::invalid_argument(str(boost::format("packet size %1%,exceed maximum packet size is %2%") % size % PACKET_MAX_SIZE));
		}
		this->resize(resize);
	}
}

/**
 *
 * @return effective data length in this packet,not include head
 *         information.
 */
int packet::getDataLen() {
	return index - HEAD_SIZE;
}

void packet::setDataLen(int len) {
	index = len + HEAD_SIZE;
}

/**
 * compress data from HEAD_SIZE to index
 */
void packet::compress() {
	/*
	int* len = new int[1];
	len[0] = index - HEAD_SIZE;
	buffer = deflater.compress(buffer, HEAD_SIZE, len);
	index = len[0] + HEAD_SIZE;
	*/
	//not implemented!
}

/**
 * encrypt data from HEAD_SIZE to index
 */
void packet::encrypt() {
	/*
	if (cipher != null) {
		int padding = cipher.pad(getDataLen());
		ensureSize(padding + index);
		try {
			int l = cipher.encrypt(buffer, HEAD_SIZE, getDataLen(), buffer,
					HEAD_SIZE);
			if (l == 0) {
				// encryption fail
				clearEncrypted();
			} else {
				index += padding;
			}
		} catch (CryptoException e) {
			throw new RuntimeException("encryption failed", e);
		}
	} else {
		// cipher not set ,do not encrypt it.
		logger.warn("Cipher not set,unable to encrypt");
		clearEncrypted();
	}
	*/
	//not implemented!
}

/**
 * decrypt data from HEAD_SIZE to index
 */
void packet::decrypt() {
	/*
	if (cipher != null) {
		try {
			// len: actual data length
			int len = cipher.decrypt(buffer, HEAD_SIZE, getDataLen(),
					buffer, HEAD_SIZE);
			index = HEAD_SIZE + len;
		} catch (CryptoException e) {
			throw new RuntimeException(e.getMessage(), e);
		}
	} else {
		throw new RuntimeException("AESChiper not set,unable to decrypt.");
	}
	*/
	//not implemented!
}

/**
 * uncompress data from HEAD_SIZE to index
 */
void packet::uncompress() {
	/*
	int[] len = new int[1];
	len[0] = index - HEAD_SIZE;
	buffer = inflater.uncompress(buffer, Packet.HEAD_SIZE, len);
	index = len[0] + HEAD_SIZE;
	*/
	//not implemented!
}

std::string packet::toString() {
	if (DEBUG)
		return toDebugString();
	return str(boost::format("Packet{compressed=%1%, encrypted=%2%, type=%3%, datalen=%4%}") % isCompressed() % isEncrypted() % getType() % getDataLen());

}

std::string packet::toDebugString() {
	/*
	StringBuilder sb = new StringBuilder();
	sb.append("[");
	for (int i = HEAD_SIZE; i < index; i++) {
		sb.append(buffer[i]);
		sb.append(",");
	}
	if (getDataLen() > 0) {
		sb.setLength(sb.length() - 1);
	}
	sb.append("]");
	return "Packet{compressed=" + isCompressed() + ",encrypted="
			+ isEncrypted() + ",type=" + getTypeDesc() + ",datalen="
			+ getDataLen() + ",data=" + sb + "}";
	*/
	//not implemented!
	return str(boost::format("Packet{compressed=%1%, encrypted=%2%, type=%3%, datalen=%4%}") % isCompressed() % isEncrypted() % getType() % getDataLen());
}

std::vector<unsigned char> packet::getBuffer() {
	fillHeader();
	return bufferVec;
}

int packet::getIndex() {
	return index;
}

/**
 * 将ChannelBuffer的长度为length的内容当成一个完整的Packet读入当前包的头部和数据区。
 * 注意ChannelBuffer可能读到了下一个包的数据
 *
 * @param in
 * @param length
 *            包的长度（包括HEAD_SIZE）
 * @throws IOException
 */
void packet::readPacket(std::vector<unsigned char> buf, int length){
	ensureSize(length - HEAD_SIZE);
	std::copy(buf.begin(), buf.begin() +length, bufferVec.begin());
	readHeader();
}

/**
 * 将整个Packet包装成ChannelBuffer,zero copy
 *
 * @return
 */
boost::asio::const_buffer packet::wrapPacket() {
	fillHeader();
	return boost::asio::buffer(this->bufferVec, index);
}

/**
 * 将包的数据区包装成ChannelBuffer,zero copy
 *
 * @return
 */
boost::asio::const_buffer packet::wrapPacketData() {
	return boost::asio::buffer(std::vector<unsigned char>(this->bufferVec.begin() + HEAD_SIZE, this->bufferVec.end()), index - HEAD_SIZE);
}

/**
 * 将ChannelBuffer的所有内容读到packet的数据区
 *
 * @param in
 * @throws IOException
 */
void packet::readData(std::vector<unsigned char> buf){
	int len = buf.size();
	ensureSize(len);
	std::copy(buf.begin(), buf.end(), bufferVec.begin()+HEAD_SIZE);
	index += len;
}

void packet::setControlPacket(packet p, int type, std::vector<unsigned char> resultBytes) {
	p.clear();
	p.setProtocol(type);
	p.feedBytes(resultBytes);
}

void packet::setControlPacket(packet p, int type, int intResult) {
	p.clear();
	p.setProtocol(type);
	p.feedInt(intResult);
}

void packet::setControlPacket(packet p, int type, long longResult) {
	p.clear();
	p.setProtocol(type);
	p.feedLong(longResult);
}

packet packet::fillHeartBeatPacket(packet p) {
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration duration( time.time_of_day() );
	setControlPacket(p, HEART_BEAT, (long)duration.total_milliseconds());
	return p;
}

packet packet::fillACKHeartBeatPacket(packet p, std::vector<unsigned char> timeBytes) {
	setControlPacket(p, ACK_HEART_BEAT, timeBytes);
	return p;
}

packet packet::fillCloseTunnelPacket(packet p) {
	std::vector<unsigned char> resultBytes(0);
	setControlPacket(p, CLOSE_TUNNEL, resultBytes);
	return p;
}

packet packet::fillDHKeyPacket(packet p, int protocol, std::vector<unsigned char> keyBytes) {
	setControlPacket(p, protocol, keyBytes);
	return p;
}

packet::~packet() {
}

} /* namespace rtunnel */
