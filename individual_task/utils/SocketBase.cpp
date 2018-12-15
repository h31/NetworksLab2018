#include "SocketBase.h"

SocketBase::SocketBase() = default;

SocketBase::~SocketBase(){
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	BOOST_LOG_TRIVIAL(debug) << "ServerBase destruction";
}

void SocketBase::throwException(string errorMsg){
    BOOST_LOG_TRIVIAL(error) << errorMsg;
	throw runtime_error( errorMsg );
}

char* SocketBase::readAll(){
	BOOST_LOG_TRIVIAL(debug) << "Begin reading";
	char* buffer = new char[BUF_SIZE]();
	char strLength[4];

	ssize_t n = read(sockfd, strLength, 4);
	if (n <= 0) {
		throwException("ERROR reading from socket");
	}
	int length = atoi(strLength);

	int received = 0;
	while(received < length){
		n = read(sockfd, buffer, length - received);
		received += n;
		if (n <= 0) {
			throwException("ERROR reading from socket");
		}
	}

	BOOST_LOG_TRIVIAL(debug) << "End reading: " << buffer;
	return buffer;
}

void SocketBase::sendAll(const char* buffer){
	BOOST_LOG_TRIVIAL(debug) << "Begin writing: " << buffer << " " << strlen(buffer);
	size_t messageLength = strlen(buffer);
	char toSend[4 + BUF_SIZE];

	snprintf(toSend, 4 + BUF_SIZE, "%04lu%s", messageLength, buffer);

	ssize_t n = write(sockfd, toSend, strlen(toSend));
	if (n <= 0) {
		throwException("ERROR writing to socket");
	}
	BOOST_LOG_TRIVIAL(debug) << "End writing";
}

void SocketBase::sendAll(string buffer){
    char *cstr = new char[buffer.length() + 1];
    strcpy(cstr, buffer.c_str());
    sendAll(cstr);
    delete [] cstr;
}

void SocketBase::closeSocket() {
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}