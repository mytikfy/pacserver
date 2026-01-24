
#ifndef PACWORKER_H
#define PACWORKER_H

#include "pacserver.h"
#include "pacmirrors.h"

class PacWorker : public HttpServer::Worker, public PacServer::Controller
{
	typedef HttpServer::Worker super;

	public:
		class Controller {
			public:
				virtual void exit(int code = 0) = 0;
				virtual PacMirrors::Server *nextMirror() = 0;
		};

	public:
		PacWorker(int index, Controller *m_controller, const std::string& cache);

		void exit(int code = 0) override;

		int fetch(std::string& path, std::string& cachedpath, HttpServer::Response *response);

	private:
		HttpServer::Response *process(HttpServer::Request *request) override;

	private:
		Controller *m_controller = nullptr;
		PacServer m_server;

		std::string m_cache;
};

#endif	// PACWORKER_H

