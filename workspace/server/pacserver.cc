

#include "pacserver.h"

#include "index.html.h"
#include "packages.html.h"
#include "clients.html.h"

PacServer::PacServer(Controller *controller)
	: m_controller(controller)
{
}

// commands
//   index				-> dashboard
//   packages
//   clients
//   exit

static struct command {
	const char *name;
	int tk;
	void (PacServer::*func)(HttpServer::Response *response);
} Commands[] = {
	{"index",    0, &PacServer::index},
	{"packages", 0, &PacServer::packages},
	{"clients",  0, &PacServer::clients},
};

void PacServer::index(HttpServer::Response *response)
{
	response->setBody(index_html_h);
	response->addHeader("Content-Length", std::to_string(response->bodySize()));
}

void PacServer::packages(HttpServer::Response *response)
{
	response->setBody(packages_html_h);
	response->addHeader("Content-Length", std::to_string(response->bodySize()));
}

void PacServer::clients(HttpServer::Response *response)
{
	response->setBody(clients_html_h);
	response->addHeader("Content-Length", std::to_string(response->bodySize()));
}

void PacServer::addTransaction(const std::string& peer, const std::string& path)
{
	printf("new transaction : %s, %s\n", peer.c_str(), path.c_str());
}

HttpServer::Response* PacServer::process(HttpServer::Request* request)
{
	auto&& splt = HttpServer::Tools::split(request->path(), '/');

	printf("pacserver processing (%ld)..\n", splt.size());

	if (splt.size() == 1) {
		splt.push_back("index.html");
	}

	if ((splt[1] == "exit") && (m_controller != nullptr)) {
		m_controller->exit();
	}

	HttpServer::Response *retv = new HttpServer::Response(request);

	retv->addHeader("Date", HttpServer::Tools::fmttime(0));
	retv->addHeader("Last-Modified", HttpServer::Tools::fmttime(-120));

	for (unsigned int i = 0; i < sizeof(Commands) / sizeof(Commands[0]); i++) {
		if (splt[1].find(Commands[i].name) == 0) {
			if (Commands[i].func != nullptr) {
				(this->*Commands[i].func)(retv);

				return retv;
			}
		}
	}


	retv->addHeader("Content-Type", "text/html");
	retv->addHeader("Connection", "close");

	retv->addBody("\n<html>");
	retv->addBody("<head>");
	retv->addBody("</head>");
	retv->addBody("<body>");
	retv->addBody("CONSOLE");
	retv->addBody("</body>");
	retv->addBody("</html>\n");

	retv->addHeader("Content-Length", std::to_string(retv->bodySize()));

	return retv;
}
