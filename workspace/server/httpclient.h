
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

#include "httpserver.h"

class HttpClient
{
	public:
		HttpClient(const std::string& server, int port);
		HttpServer::Response *get(HttpServer::Request *request, FILE *fhd = nullptr);

	private:
		int receiveBlock(int sock, int& state, std::string& line, HttpServer::Response *retv, long& count, FILE *fhd);
		void setProtocol(const std::string& line);

	private:
		std::string m_host;
		int m_port;
};

#endif	// HTTPCLIENT_H

