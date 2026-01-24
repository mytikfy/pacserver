
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "pacproxy.h"
#include "httpclient.h"
#include "pacserver.h"
#include "pacmirrors.h"

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
	printf("pacload\n");

	m_mirrors.load("x86", config + "/mirrorlist");
	m_mirrors.load("x86", config + "/mirrorlist.x86");
	m_mirrors.load("arm", config + "/mirrorlist.arm");

	m_mirrors.print();

	printf("pacload - ready %d\n", ::getpid());
}

PacProxy::~PacProxy()
{
}

HttpServer::Worker *PacProxy::createWorker(int index)
{
	printf("creating PacProxy -> Worker (%d)\n", index);
	return new PacWorker(index, this, m_cache);
}

void PacProxy::exit(int code)
{
	printf("server exit\n");

	super::term();
}

PacMirrors::Server* PacProxy::nextMirror()
{
	return m_mirrors.next();
}

PacProxy::Response::Response(Request* request)
	: super(request)
{
}
