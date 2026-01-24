#ifndef PACPROXY_H
#define PACPROXY_H

#include "httpserver.h"
#include "pacmirrors.h"
#include "pacserver.h"
#include "pacworker.h"

class PacProxy : public HttpServer, public PacWorker::Controller
{
	typedef HttpServer super;

	public:
		class Response : public HttpServer::Response
		{
			typedef HttpServer::Response super;

			public:
				Response(HttpServer::Request *request);

			private:
		};

	public:
		PacProxy(int port = 80, const std::string& config = "./pacconfig", const std::string& cache = "./paccache");
		virtual ~PacProxy();

		void exit(int code = 0) override;
		PacMirrors::Server* nextMirror() override;

		HttpServer::Worker *createWorker(int index) override;

	private:
		std::string m_config;
		std::string m_cache;

		PacMirrors m_mirrors;
};


#endif	// PACPROXY_H

