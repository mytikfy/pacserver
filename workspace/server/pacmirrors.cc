
#include <stdint.h>
#include <stdio.h>

#include "httpserver.h"
#include "pacmirrors.h"

static pthread_mutex_t GET_LOCK = PTHREAD_MUTEX_INITIALIZER;

#define GLUE(A, B)   A ## B
// #define P(X) pthread_mutex_lock(&GLUE(#X, _LOCK))
#define P(X) pthread_mutex_lock(&X)
// #define V(X)
//#define V(X) pthread_mutex_unlock(#X ## _LOCK)
#define V(X) pthread_mutex_unlock(&X)

PacMirrors::Server::Server(const std::string& host, const std::string& path, const std::string& arch)
	: m_host(host), m_pathes(HttpServer::Tools::split(path, '/')), m_arch(arch)
{
}

std::string PacMirrors::Server::host() const
{
	return m_host;
}

std::string PacMirrors::Server::arch() const
{
	return m_arch;
}

void PacMirrors::Server::disable()
{
	m_disabled = true;
}

int PacMirrors::Server::isDisabled() const
{
	return m_disabled;
}

std::string PacMirrors::Server::get(const std::string& path)
{
	P(GET_LOCK);

	auto&& pathes = HttpServer::Tools::split(path, '/');

	std::string repo;
	std::string arch;
	std::string package;

	if (pathes.size() != (m_pathes.size() + 1)) {
		printf("splits different : %s -> %d, %d\n", m_host.c_str(), (int)m_pathes.size(), (int)pathes.size());
		printf("                 : %s\n", HttpServer::Tools::join(m_pathes, 0, '/').c_str());
		disable();
		exit(1);
	}

	for (size_t i = 0; i < m_pathes.size(); i++) {
//		printf("m_pathes/pathes : %s, %s\n", m_pathes[i].c_str(), pathes[i].c_str());
		if (m_pathes[i] != pathes[i]) {
			if (m_pathes[i] == "$repo") {
				repo = pathes[i];
			}
			else if (m_pathes[i] == "$arch") {
				arch = pathes[i];
			}
		}
	}

	package = pathes.back();

	// printf("repo/arch : %s, %s\n", repo.c_str(), arch.c_str());

	std::string retv = "/";

	for (size_t i = 0; i < m_pathes.size(); i++) {
		if (m_pathes[i] == "http:") {
			retv = "http:/";
		}
		else if (m_pathes[i] == "$repo") {
			retv += repo;
		}
		else if (m_pathes[i] == "$arch") {
			retv += arch;
		}
		else {
			retv += m_pathes[i];
		}

		retv += "/";
	}

	retv += package;

	// printf("retv : %s\n", retv.c_str());

	V(GET_LOCK);

	return retv;
}

PacMirrors::Server *PacMirrors::Server::instance(const std::string& server, const std::string& arch)
{
	auto&& splt = HttpServer::Tools::split(server, '/');

	if ((splt[0] == "http:") && (splt.size() == 6)) {
		return new Server(splt[1], HttpServer::Tools::join(splt, 2, '/'), arch);
	}

	return nullptr;
}

int PacMirrors::load(const std::string& arch, const std::string& path)
{
	FILE *fhd = nullptr;

	fhd = ::fopen(path.c_str(), "r");

	if (fhd != 0) {
		uint8_t buf[256];
		std::string line;

		while (true) {
			int readed = ::fread(buf, 1, sizeof(buf), fhd);

			for (int i = 0; i < readed; i++) {
				int c = buf[i];

				if (c == '\n') {
					if (!line.empty() && (line.find("#") != 0)) {
						auto&& splt = HttpServer::Tools::split(line, ' ');

						if ((splt.size() == 3) && (splt[0] == "Server") && (splt[1] == "=")) {
							addServer(splt[2], arch);
						}
					}

					line = "";
				}
				else if (c == '\r') {
				}
				else {
					line += (char) c;
				}
			}

			if (readed != sizeof(buf)) {
				break;
			}
		}

		::fclose(fhd);
		fhd = nullptr;
	}

	return 0;
}

void PacMirrors::print()
{
	for (size_t i = 0; i < m_servers.size(); i++) {
		printf("%-8s : %-35s -- %s\n", m_servers[i]->arch().c_str(), m_servers[i]->host().c_str(), "++");
	}
}

PacMirrors::Server* PacMirrors::next(const char *arch)
{
	while (true) {
//		m_pos++;

		if (m_pos >= m_servers.size()) {
			m_pos = 0;
		}

		if (!m_servers[m_pos]->isDisabled()) {
			break;
		}
	}

	return m_servers[m_pos];
}

void PacMirrors::addServer(const std::string& server, const std::string& arch)
{
	auto&& s = PacMirrors::Server::instance(server, arch);

	if (s != nullptr) {
		m_servers.push_back(s);
	}
}

