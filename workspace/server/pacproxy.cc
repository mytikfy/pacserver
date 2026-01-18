// GET /arch/core/os/x86_64/core.db HTTP/1.1
// Host: mirror.informatik.tu-freiberg.de
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: de,en-US;q=0.7,en;q=0.3
// Accept-Encoding: gzip, deflate
// DNT: 1
// Sec-GPC: 1
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Priority: u=0, i
// Pragma: no-cache
// Cache-Control: no-cache

// HTTP/1.1 200 OK
// Date: Mon, 12 Jan 2026 20:54:27 GMT
// Server: Apache/2.4.65 (Debian)
// Last-Modified: Mon, 12 Jan 2026 20:22:19 GMT
// ETag: "1e74e-64836a1bb5843"
// Accept-Ranges: bytes
// Content-Length: 124750
// Keep-Alive: timeout=5, max=100
// Connection: Keep-Alive

#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>
 #include <unistd.h>

#include "pacproxy.h"
#include "httpclient.h"

// mirrolist
// Server = http://localhost:8080/$repo/os/$arch
// Server = https://mirror.dogado.de/archlinux/$repo/os/$arch
// Server = http://mirror.informatik.tu-freiberg.de/arch/$repo/os/$arch

#define MY_MIRRORSTRING		"/archlinux/$repo/os/$arch"
#define MIRRORSERVER    	"mirror.informatik.tu-freiberg.de"
#define MIRRORPATH    		"/arch/$repo/os/$arch"

PacProxy::PacProxy(int port, const std::string& config, const std::string& cache)
	: super(port), m_config(config), m_cache(cache)
{
}

PacProxy::~PacProxy()
{
}

#include <string.h>

static std::vector<std::string> split(const std::string& line, char tk)
{
	char *l = (char *)alloca(line.size() + 1);
	memset(l, 0, line.size() + 1);
	strncpy(l, &line[0], line.size());

	const char _tk[2] = { tk , 0 };
	std::vector<std::string> retv;
	char *tok = strtok(l, _tk);

	while  (tok) {
		retv.push_back(tok);
		tok = strtok(nullptr, _tk);
	}

	return retv;
}

int mkdirs(const std::string& ddir, const std::vector<std::string>& path)
{
	std::string dir = ddir;

	for (size_t i = 0; i < path.size(); i++) {
		struct stat st;
		int status = ::stat(dir.c_str(), &st);

		if (status) {
			status = ::mkdir(dir.c_str(), 0774);

			if (status) {
				perror("dir failed");
				return 1;
			}
		}

		dir += "/" + path[i];
	}

	return 0;
}

static std::string toHex(int value) {
	std::string result = "0";

    if (value != 0) {
		// std::string hexChars = "0123456789ABCDEF";
		const char *hexChars = "0123456789ABCDEF";

		while (value > 0) {
			result = hexChars[value % 16] + result;
			value /= 16;
		}
	}

	return result;
}

int PacProxy::fetch(std::vector<std::string>& mysplt, std::vector<std::string>& usersplt, std::string& cachedpath, Response *response)
{
	std::string repo;
	std::string arch;
	std::string package;

	for (size_t i = 0; i < mysplt.size(); i++) {
		if (usersplt[i] != mysplt[i]) {
			if (mysplt[i] == "$repo") {
				repo = usersplt[i];
			}
			else if (mysplt[i] == "$arch") {
				arch = usersplt[i];
			}
		}
	}

	package = usersplt.back();

	auto&& mirror   = split(MIRRORPATH, '/');
	std::string mirrorpath = "/";

	for (size_t i = 0; i < mirror.size(); i++) {
		if (mirror[i] == "http:") {
			mirrorpath =  "http:/";
		}
		else if (mirror[i] == "$repo") {
			mirrorpath += repo;
		}
		else if (mirror[i] == "$arch") {
			mirrorpath += arch;
		}
		else {
			mirrorpath += mirror[i];
		}

		mirrorpath += "/";
	}

	mirrorpath += package;

	printf("get package from server : %s, %s\n", MIRRORSERVER, mirrorpath.c_str());

	HttpServer::Request request;
	request.setMethod("GET");
	request.setPath(mirrorpath);

	FILE *fhd = nullptr;
	std::string tmpCachedpath = cachedpath;

	if (!cachedpath.empty()) {
		tmpCachedpath = cachedpath + "-" + toHex(std::rand());
		fhd = ::fopen(tmpCachedpath.c_str(), "w");
	}

	request.addHeader("host", MIRRORSERVER, 0);
	request.addHeader("user-agent", "pacserver/7.1.0 (Linux x86_64) libalpm/16.0.1", 0);
	request.addHeader("accept", "*/*", 0);

	HttpClient client(MIRRORSERVER, 80);
	HttpServer::Response *clientresponse = client.get(&request, fhd);

	if (fhd) {
		::fclose(fhd);

		if (clientresponse->status() != 200) {
			printf("wrong response status %d\n", clientresponse->status());
			::unlink(tmpCachedpath.c_str());
		}
		else {
			std::rename(tmpCachedpath.c_str(), cachedpath.c_str());
			struct utimbuf tms;

			tms.actime = clientresponse->lastModifiedGMT();
			tms.modtime = tms.actime;

			utime(cachedpath.c_str(), &tms);
		}
	}

	int status = clientresponse->status();
	return status;
}

HttpServer::Response* PacProxy::process(Request* request)
{
	if ((request->userAgent() == "pacman") || (1)) {
		Response *retv = new PacProxy::Response(request);

		std::string path = request->path();
		std::string filetype = ".db";

		std::string cachedpath = m_cache + path;

		FILE *fhd = 0;
		bool update = false;

		if (Tools::endsWith(path, ".db")) {
			update = true;
		}

		while (true) {
			fhd = ::fopen(cachedpath.c_str(), "r");

			if ((fhd == 0) || update) {
				update = false;

				auto&& usersplt = split(path, '/');
				auto&& mysplt   = split(MY_MIRRORSTRING, '/');

				mkdirs(m_cache, usersplt);

				if (usersplt.size() != (mysplt.size() + 1)) {
					printf("splits different : %d, %d\n", (int)usersplt.size(), (int)mysplt.size());
					retv->setStatus(404, "not found");
					break;
				}
				else {
					int status = fetch(mysplt, usersplt, cachedpath, retv);

					if (status != 200) {
						retv->setStatus(status, "not found");
						break;
					}
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

		auto&& fmttime = [] (int offset) {
			char buf[100] = {};
			time_t now = time(0) + offset;
			struct tm tm = *gmtime(&now);
			strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
			return std::string(buf);
		};

		retv->addHeader("Date", fmttime(0));
		retv->addHeader("Last-Modified", fmttime(-120));

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

PacProxy::Response::Response(Request* request)
	: super(request)
{
}
