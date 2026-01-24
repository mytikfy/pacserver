
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pacworker.h"
#include "httpclient.h"
#include "httpserver.h"
#include "pacproxy.h"

PacWorker::PacWorker(int index, Controller *controller, const std::string& cache)
	: super(index), m_controller(controller), m_server(this), m_cache(cache)
{
}

void PacWorker::exit(int code)
{
	m_controller->exit(code);
}

int PacWorker::fetch(std::string& path, std::string& cachedpath, HttpServer::Response *response)
{
	//auto&& pm = m_mirrors.next();
	auto&& pm = m_controller->nextMirror();

	auto&& mirrorpath = pm->get(path);

	printf("get package from server : %s, %s\n", pm->host().c_str(), mirrorpath.c_str());

	HttpServer::Request request;
	request.setMethod("GET");
	request.setPath(mirrorpath);

	FILE *fhd = nullptr;
	std::string tmpCachedpath = cachedpath;

	if (!cachedpath.empty()) {
		auto&& mysplt = HttpServer::Tools::split(tmpCachedpath, '/');

		HttpServer::Tools::mkdirs(".", mysplt);

		tmpCachedpath = cachedpath + "-" + HttpServer::Tools::toHex(std::rand());

		printf("local path shall be : %s\n", tmpCachedpath.c_str());
		fhd = ::fopen(tmpCachedpath.c_str(), "w");

		if (fhd == nullptr) {
			printf("fdh == null\n");
			exit(102);
		}
	}

	request.addHeader("host", pm->host(), 0);
	request.addHeader("user-agent", "pacserver/7.1.0 (Linux x86_64) libalpm/16.0.1", 0);
	request.addHeader("accept", "*/*", 0);

	HttpClient client(pm->host(), 80);
	HttpServer::Response *clientresponse = client.get(&request, fhd);

	if (fhd) {
		::fclose(fhd);
		fhd = nullptr;

		if (clientresponse->status() != 200) {
			printf("wrong response status %d, %d\n", __LINE__, clientresponse->status());
			::unlink(tmpCachedpath.c_str());
		}
		else {
			std::rename(tmpCachedpath.c_str(), cachedpath.c_str());
			struct utimbuf tms;

			tms.actime = clientresponse->lastModifiedGMT();
			tms.modtime = tms.actime;

			::utime(cachedpath.c_str(), &tms);
		}
	}

	int status = clientresponse->status();

	return status;
}

HttpServer::Response* PacWorker::process(HttpServer::Request* request)
{
	if ((request->path().size() >= 8) && (request->path().rfind("/console") == 0)) {
		return m_server.process(request);
	}

	if ((request->path().size() >= 11) && (request->path().rfind("/archlinux/") == 0)) {
		PacProxy::Response *retv = new PacProxy::Response(request);

		std::string path = request->path();

		std::string cachedpath = m_cache + path;

		FILE *fhd = 0;
		bool update = false;

		if (HttpServer::Tools::endsWith(path, ".db")) {
			update = true;
		}

		// update = true;

		m_server.addTransaction(request->peer(), cachedpath);

		while (true) {
			fhd = ::fopen(cachedpath.c_str(), "r");

			if ((fhd == 0) || update) {
				if (fhd != 0) {
					::fclose(fhd);
					fhd = nullptr;
				}

				update = false;

				int status = fetch(path, cachedpath, retv);

				if (status != 200) {
					retv->setStatus(status, "not found");
					break;
				}
			}
			else {
				struct stat st;
				::stat(cachedpath.c_str(), &st);
				retv->addHeader("Content-Length", std::to_string(st.st_size));
				break;
			}
		}

		retv->addHeader("Server", "pacserver/0.9");

		retv->addHeader("Date", HttpServer::Tools::fmttime(0));
		retv->addHeader("Last-Modified", HttpServer::Tools::fmttime(-120));

		retv->addHeader("Content-Type", "application/octet-stream");
		retv->addHeader("Accept-Ranges", "bytes");

		retv->addHeader("Connection", "keep-alive");

		retv->setBody(fhd);

		return retv;
	}
	else {
		return super::process(request);
	}
}

