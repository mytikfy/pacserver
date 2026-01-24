

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>

#include "httpclient.h"

HttpClient::HttpClient(const std::string& host, int port)
	: m_host(host), m_port(port)
{
}

int HttpClient::receiveBlock(int sock, int& state, std::string& line, HttpServer::Response *retv, long& count, FILE *fhd)
{
	auto&& store = [fhd, retv, &count](int c) {
		if (fhd == 0) {
			retv->addBody(c);
		}
		else {
			int written = fwrite(&c, 1, 1, fhd);
			if (written != 1) {
				perror("file write failed");
				exit(67);
			}
		}

		if ((count != -1) && (--count <= 0)) {
			return true;
		}

		return false;
	};

	static uint8_t buf[512 * 1024] = {};
	size_t bytes = recv(sock, buf, sizeof(buf), 0);

	if (bytes <= 0) {
 		perror("rceiv");
		exit(32);

		return 0;
	}

	for (size_t i = 0; i < bytes; i++) {
		int c = buf[i];

		if (c == '\n') {
			if (state == 0) {
				// Protocol line
				state = 1;
				retv->setProtocol(line);
				line = "";
			}
			else if (state == 1) {
				// header lines
				if (line.empty()) {
					state = 2;

					HttpServer::HeaderLine::Base *hlb = retv->findHeader("content-length");

					if ((hlb != nullptr) && (hlb->type() == HttpServer::HeaderLine::HEADER_CONTENTLENGTH)) {
						count = ((HttpServer::HeaderLine::ContentLength *)hlb)->contentLength();
						printf("expected filelength :%ld\n", count);
					}
					else {
						return true;
					}
				}
				else {
					retv->addHeader(HttpServer::HeaderLine::Base::instance(line));
					line = "";
				}
			}
			else {
				if (store(c)) {
					return true;
				}
			}
		}
		else if (c == '\r') {
			// skip
			if (state == 2) {
				if ( store(c)) {
					return true;
				}
			}
		}
		else {
			if (state != 2) {
				line += (char)c;
			}
			else {
				if (store(c)) {
					return true;
				}
			}
		}
	}

	return 0;
}

HttpServer::Response *HttpClient::get(HttpServer::Request *request, FILE *fhd)
{
	std::string url = m_host + (m_port != 80 ? ":" + std::to_string(m_port) : "" ) + "/";

	HttpServer::Response *retv = new HttpServer::Response(request);
	std::vector<std::string> headers = request->headers();

	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(6);
	}

	// Connect to the API server
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(m_port);

	hostent *he = gethostbyname(m_host.c_str());

	if (he == nullptr) {
		printf("he failed \n");
		exit(9);
	}

	memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("connect failed");
		exit(8);
	}

	// Send the request to the server
	std::string data = request->toString();

	size_t written = ::send(sock, data.c_str(), data.size(), 0);

	if (written != data.size()) {
		perror("write to server vailed");
		exit(54);
	}

	std::string line;
	int state = 0;
	long count = -1;

	while (true) {
		if (receiveBlock(sock, state, line, retv, count, fhd)) {
			break;
		}
	}

	close(sock);

	return retv;
}

