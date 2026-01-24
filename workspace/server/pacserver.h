#ifndef PACSERVER_H
#define PACSERVER_H

#include "httpserver.h"

class PacServer
{
	public:
		class Controller
		{
			public:
				virtual void exit(int code = 0) = 0;
		};

	public:
		PacServer(Controller *controller);

		HttpServer::Response *process(HttpServer::Request *request);

		void index(HttpServer::Response *response);
		void packages(HttpServer::Response *response);
		void clients(HttpServer::Response *response);

		void addTransaction(const std::string& peer, const std::string& path);

	private:
		Controller *m_controller = nullptr;

};

#endif	// PACSERVER_H
