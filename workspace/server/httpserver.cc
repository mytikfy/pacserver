#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <linux/tcp.h>
#include <arpa/inet.h>

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <iostream>

#include "httpserver.h"
#include "httpclient.h"

// Trim from the start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline std::string fromUrlEncoding(const std::string& from)
{
	std::string p;

	for (size_t i = 0; i < from.length(); i++) {
		int c = from[i];

		if (c == '%') {
			std::string num;
			num += from[++i];
			num += from[++i];

			int value = ::strtol(num.c_str(), nullptr, 16);

			p += (char)value;
		}
		else {
			p += (char)c;
		}
	}

	return p;
}

void HttpServer::Request::addHeader(HeaderLine::Base *headerline)
{
	m_headers.insert(std::pair<std::string, HeaderLine::Base *>(headerline->key(), headerline));
}
void HttpServer::Request::addHeader(const std::string& key, const std::string& value, uint32_t flags)
{
	HeaderLine::Base *hlb = new HeaderLine::Base(key, value, flags);
	if (hlb) {
		m_headers.insert(std::pair<std::string, HeaderLine::Base *>(key, hlb));
	}
}

void HttpServer::Request::setMethod(const std::string& method)
{
	m_method = method;
}

void HttpServer::Request::setPath(const std::string& path)
{
	m_path = path;
}

HttpServer::HeaderLine::Base *HttpServer::Request::findHeader(const std::string& key)
{
	try {
		return m_headers.at(key);
	} catch (std::out_of_range& e) {
		printf("exception %s\n", e.what());
	}

	return nullptr;
}

std::string HttpServer::Request::userAgent()
{
	auto&& ua = findHeader("user-agent");

	if ((ua != nullptr) && (ua->type() == HeaderLine::HEADER_USERAGENT)) {
		HeaderLine::UserAgent *uai = (HeaderLine::UserAgent *)ua;

		return uai->product();
	}

	return "";
}

void HttpServer::Request::parsePath()
{
	size_t pos = m_path.find("#");

	if (pos != std::string::npos) {
		m_fragment = m_path.substr(pos + 1);
		m_path = m_path.substr(0, pos);
	}

	pos = m_path.find("?");

	if (pos != std::string::npos) {
		m_query = m_path.substr(pos + 1);
		m_path = m_path.substr(0, pos);
	}

	m_path = fromUrlEncoding(m_path);
}

void HttpServer::Request::parseMethod(const std::string& line)
{
	char *buf = new char[line.size() + 1]();
	strcpy(buf, line.c_str());

	char *tok = ::strtok(buf, " ");
	int pos = 0;

	while (tok != nullptr) {
		if (pos == 0) {
			m_method = tok;
		}
		else if (pos == 1) {
			m_path = tok;
		}
		else {
			m_protocol = tok;
		}

		pos++;
		tok = ::strtok(nullptr, " ");
	}

	parsePath();

	delete[] buf;
}

std::string HttpServer::Request::method() const
{
	return m_method;
}

std::string HttpServer::Request::path() const
{
	return m_path;
}

std::string HttpServer::Request::url() const
{
	return "http://" + m_path;
}

std::vector<std::string> HttpServer::Request::headers() const
{
	std::vector<std::string> retv;
	return retv;
}

void HttpServer::Request::print() const
{
	printf("  method : %s\n", m_method.c_str());
	printf("  path : %s\n", m_path.c_str());
	printf("  protocol : %s\n", m_protocol.c_str());
	printf("  show headers (%d)\n", (int)m_headers.size());
	for (auto&& header : m_headers) {
		printf("    header : %s = %s\n", header.first.c_str(), header.second->value().c_str());
	}
}

std::string HttpServer::Request::toString() const
{
	std::string retv;

	retv = m_method + " " + m_path + " " + m_protocol + "\r\n";

	for (auto&& header : m_headers) {
		retv += header.second->key() + ": " + header.second->value() + "\r\n";
	}

	// header / block sep
	retv += "\r\n";

	return retv;
}

HttpServer::HeaderLine::Base::Base(const std::string& key, const std::string& value, uint32_t flags)
	: m_key(key), m_value(value), m_flags(flags)
{
}

std::string HttpServer::HeaderLine::Base::key() const
{
	return m_key;
}

std::string HttpServer::HeaderLine::Base::value() const
{
	return m_value;
}

int HttpServer::HeaderLine::Base::type() const
{
	return HEADER_BASE;
}

std::string HttpServer::HeaderLine::Base::toString() const
{
	return m_key + ": " + value();
}

HttpServer::HeaderLine::UserAgent::UserAgent(const std::string& value)
	: super("user-agent", value)
{
	parse(value);
}

void HttpServer::HeaderLine::UserAgent::parse(const std::string& line)
{
	int state = 0;
	std::string tok;

	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/User-Agent

	for (size_t i = 0; i < line.size(); i++) {
		int c = line[i];

		if (state == 0) {
			if (c == '/') {
				m_product = tok;
				tok.clear();

				state = 1;
			}
			else {
				tok += (char)c;
			}
		}
		else if (state == 1) {
			if (c == ' ') {
				m_productVersion = tok;
				tok.clear();

				state = 2;
			}
			else {
				tok += (char)c;
			}
		}
		else if (state == 2) {
			if (c == '(') {
				tok.clear();

				state = 3;
			}
		}
		else if (state == 3) {
			if (c == ')') {
				m_system = tok;
				tok.clear();

				state = 4;
				break;
			}
			else {
				tok += (char)c;
			}
		}
	}
}

std::string HttpServer::HeaderLine::UserAgent::product() const
{
	return m_product;
}

std::string HttpServer::HeaderLine::UserAgent::productVersion() const
{
	return m_productVersion;
}

std::string HttpServer::HeaderLine::UserAgent::system() const
{
	return m_system;
}

int HttpServer::HeaderLine::UserAgent::type() const
{
	return HEADER_USERAGENT;
}

HttpServer::HeaderLine::LastModified::LastModified(const std::string& value)
	: super("last-modified", value)
{
	parse(value);
}

int HttpServer::HeaderLine::LastModified::type() const
{
	return HEADER_LASTMODIFIED;
}

void HttpServer::HeaderLine::LastModified::parse(const std::string& line)
{
	m_lastModified = Tools::fromHttpDate(line);
}

time_t HttpServer::HeaderLine::LastModified::lastModified() const
{
	return m_lastModified;
}

HttpServer::HeaderLine::ContentLength::ContentLength(const std::string& value)
	: super("content-length", value)
{
	parse(value);
}

size_t HttpServer::HeaderLine::ContentLength::contentLength() const
{
	return m_contentLength;
}

void HttpServer::HeaderLine::ContentLength::parse(const std::string& line)
{
	m_contentLength = ::strtol(line.c_str(), nullptr, 10);
}

int HttpServer::HeaderLine::ContentLength::type() const
{
	return HttpServer::HeaderLine::HEADER_CONTENTLENGTH;
}

std::string HttpServer::HeaderLine::ContentLength::value() const
{
	return std::to_string(m_contentLength);
}

HttpServer::HeaderLine::TestHL::TestHL(const std::string& value)
	: super("test", value)
{
}

HttpServer::HeaderLine::Base *HttpServer::HeaderLine::Base::instance(const std::string& line)
{
	std::string key;

	for (size_t i = 0; i < line.size(); i++) {
		int c = line[i];

		if (c == ':') {
			std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
			std::string value = line.substr(i+1);

			rtrim(key);
			rtrim(value);
			ltrim(key);
			ltrim(value);

			if (key.empty()) {
				printf("mallformed line : %s\n", line.c_str());
				return nullptr;
			}
			else if (key == "test") {
				return new TestHL(value);
			}
			else if (key == "user-agent") {
				return new UserAgent(value);
			}
			else if (key == "last-modified") {
				return new LastModified(value);
			}
			else if (key == "content-length") {
				return new ContentLength(value);
			}
			else {
				return new HeaderLine::Base(key, value);
			}
		}
		else {
			key += (char)c;
		}
	}

	if (!key.empty()) {
		printf("mallformed line : %s\n", line.c_str());
	}

	return nullptr;
}

time_t HttpServer::Tools::fromHttpDate(const std::string& date)
{
	struct tm tm = {};
	char *status = strptime(date.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	(void) status;

	time_t t = mktime(&tm);
	t += tm.tm_gmtoff;

	return t;
}

bool HttpServer::Tools::endsWith(const std::string& line, const std::string& ending)
{
    if (line.length() >= ending.length()) {
        return (0 == line.compare(line.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

HttpServer::HttpServer(int port)
	: m_port(port)
{
}

HttpServer::~HttpServer()
{
}

HttpServer::Request* HttpServer::parse(const char* data, size_t size)
{
	Request *retv = new Request();

	std::string line;
	bool first = true;

	for (size_t i = 0; i < size; i++) {
		int c = data[i];

		if (c == '\n') {
			if (line.empty()) {
				// now rest shall be body
			}
			else {
				if (first) {
					// http GET/POST line
					retv->parseMethod(line);
					first = false;
				}
				else {
					retv->addHeader(HeaderLine::Base::instance(line));
				}

				line.clear();
			}
		}
		else if (c == '\r') {
			// skip
		}
		else {
			line += (char)c;
		}
	}

	return retv;
}

void HttpServer::accept(int serverSocket)
{
	// Accept an incoming connection:
	int sock = ::accept(serverSocket, nullptr, nullptr);

	struct sockaddr_in peerAddr;
    socklen_t peerAddrSize = sizeof(peerAddr);

    ::getpeername(sock, (struct sockaddr*)&peerAddr, &peerAddrSize);

	char ip[INET_ADDRSTRLEN] = {};
	::inet_ntop(AF_INET, &peerAddr.sin_addr, ip, INET_ADDRSTRLEN);

	printf("incoming request from %s ...\n", ip);

	// Read the request message from the client:
	char buffer[BUFFER_SIZE] = {};
	size_t size = ::recv(sock, buffer, BUFFER_SIZE - 1, 0);

	Request *request = parse(buffer, size);
	Response *response = process(request);

	response->prepare();

	int flag = 1;
	::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

	std::string headers = response->headers();

	size_t written = ::send(sock, headers.c_str(), headers.size(), 0);

	if (written != headers.size()) {
		perror("header response write failed");
		exit(11);
	}

	std::vector<uint8_t> data;

	while (true) {
		int status = response->body(data);

		if (data.size()) {
			{
				int error_code;
				socklen_t error_code_size = sizeof(error_code);
				getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);

				if (error_code != 0) {
					perror("server disconnected");
					break;
				}
			}

			size_t written = ::send(sock, &data[0], data.size(), 0);

			if (written != data.size()) {
				perror("server write failed");
				break;
			}
		}

		if (status) {
			break;
		}
	}

	delete request;
	delete response;

	// Close the socket connection:
	close(sock);
}

void HttpServer::run()
{
	int serverSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Bind to an IP address and port:
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(m_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Bind to any local interface
	// SO_REUSEADDR | SO_REUSEPORT
	int opt = 1;
	::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    int status = ::bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));

	if (status) {
		perror("bind failed");
		exit(5);
	}

    // Start listening for connections:
    status = ::listen(serverSocket, 64); // Maximum number of connections (e.g., 10)

	if (status) {
		perror("bind failed");
		exit(5);
	}

	printf("wait for connection on %d ...\n", m_port);

    while (!m_term) {
		accept(serverSocket);
	}
}

HttpServer::Response *HttpServer::process(Request* request)
{
	Response *retv = new Response(request);
	return retv;
}

HttpServer::Response::Response(Request* request)
	: m_request(request)
{
}

HttpServer::Response::~Response()
{
}

void HttpServer::Response::prepare()
{
	if (m_formatted.empty()) {
		std::string response;
		response = "HTTP/1.1 ";
		response += std::to_string(m_status.code) + " " + m_status.msg;
		response += "\r\n";

		for (auto&& header : m_headers) {
			response += header.second->toString() + "\r\n";
		}
		response += "\r\n";

		if (m_body.file == nullptr) {
			response += std::string(m_body.data.begin(), m_body.data.end());
		}

		m_formatted = response;
	}
}

void HttpServer::Response::addHeader(const std::string& key, const std::string& value, uint32_t flags)
{
	HeaderLine::Base *hlb = new HeaderLine::Base(key, value, flags);
	if (hlb) {
		m_headers.insert(std::pair<std::string, HeaderLine::Base *>(key, hlb));
	}
}

void HttpServer::Response::addHeader(HeaderLine::Base* headerline)
{
	if (headerline != nullptr) {
		m_headers.insert(std::pair<std::string, HeaderLine::Base *>(headerline->key(), headerline));
	}
}

void HttpServer::Response::setBody(const std::string& body)
{
	m_body.data = std::vector<uint8_t>(body.begin(), body.end());
}

void HttpServer::Response::setBody(FILE *file)
{
	m_body.file = file;
}

void HttpServer::Response::addBody(int c)
{
	m_body.data.push_back(c);
}

int HttpServer::Response::body(std::vector<uint8_t>& data) const
{
	if (m_body.file != nullptr) {
		static uint8_t buf[256 * 1024] = {};

		int readed = fread(buf, 1, sizeof(buf), m_body.file);

		std::vector<uint8_t> v(buf, buf + readed);
		data = v;

		return readed != sizeof(buf);
	}
	else {
		// std::string retv(m_body.data.begin(), m_body.data.end());;
		data = m_body.data;;
		return 1;
	}
}

int HttpServer::Response::status() const
{
	return m_status.code;
}

time_t HttpServer::Response::lastModifiedGMT()
{
	auto&& header = findHeader("last-modified");

	if ((header) && (header->type() == HeaderLine::HEADER_LASTMODIFIED)) {
		return ((HeaderLine::LastModified *)header)->lastModified();
	}

	return 0;
}

void HttpServer::Response::setStatus(int code, const std::string& msg)
{
	printf("status set : %d %s\n", code, msg.c_str());
	m_status.code = code;
	m_status.msg = msg;
}

void HttpServer::Response::setProtocol(const std::string& line)
{
	size_t firstIdx = line.find_first_of(' ');
	m_protocol = line.substr(0, firstIdx);
	size_t nextIdx = line.find_first_of(' ', firstIdx + 1);
    m_status.code = ::strtol(line.substr(firstIdx + 1, nextIdx - (firstIdx + 1)).c_str(), nullptr, 10);
	m_status.msg = line.substr(nextIdx + 1);
}

HttpServer::HeaderLine::Base* HttpServer::Response::findHeader(const std::string& key)
{
	try {
		return m_headers.at(key);
	} catch (std::out_of_range& e) {
		printf("exception %s\n", e.what());
	}

	return nullptr;
}

const std::string HttpServer::Response::headers() const
{
	return m_formatted;
}

