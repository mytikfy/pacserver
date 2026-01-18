#ifndef PACPROXY_H
#define PACPROXY_H

#include "httpserver.h"
#include "pacmirrors.h"

class PacProxy : public HttpServer
{
	typedef HttpServer super;


	class Response : public HttpServer::Response
	{
		typedef HttpServer::Response super;

		public:
			Response(Request *request);

		private:
	};

	public:
		PacProxy(int port = 80, const std::string& config = "./pacconfig", const std::string& cache = "./paccache");
		virtual ~PacProxy();

	private:
		HttpServer::Response* process(Request* request) override;
		int fetch(std::vector<std::string>& mysplt, std::vector<std::string>& usersplt,std::string& cachedpath, Response *response);

	private:
		std::string m_config;
		std::string m_cache;
};


#endif	// PACPROXY_H

