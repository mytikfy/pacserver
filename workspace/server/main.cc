#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <string>

#include "pacproxy.h"
#include "httpclient.h"
#include "version.h"

// #include "malloc.cc"

int main(int argc, char *argv[])
{
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	auto&& nextArg = [argc, argv] (int& i) {
		i++;

		if (i < argc) {
			return argv[i];
		}

		printf("missing argument for %s\n", argv[i-1]);
		exit(1);
	};

	std::string host;
	std::string path = "/";
	std::string file = "/";
	std::string cache = "./paccache";
	std::string config = "./pacconfig";
	int port = 80;
	bool store = false;
	int worker = 1;

	for (int i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--client", 8)) {
			host = nextArg(i);
		}
		else if (!strncmp(argv[i], "--path", 6)) {
			path = nextArg(i);
		}
		else if (!strncmp(argv[i], "--file", 6)) {
			file = nextArg(i);
		}
		else if (!strncmp(argv[i], "--port", 6)) {
			port = ::strtol(nextArg(i), nullptr, 10);
		}
		else if (!strncmp(argv[i], "--worker", 8)) {
			worker = ::strtol(nextArg(i), nullptr, 10);
			worker = 1;	// multiple worker not working
		}
		else if (!strncmp(argv[i], "--cache", 7)) {
			cache = nextArg(i);
		}
		else if (!strncmp(argv[i], "--config", 8)) {
			config = nextArg(i);
		}
		else if (!strncmp(argv[i], "--store", 7)) {
			store = true;
		}
		else if (!strncmp(argv[i], "--version", 9)) {
			printf("version: v%s\n", VERSION);
			exit(0);
		}
		else if (!strncmp(argv[i], "--", 2)) {
			break;
		}
	}

	if (host.empty()) {
		PacProxy server(port, config, cache);
		server.run(worker);

		printf("all done\n");
	}
	else {
		(void) store;

		printf("contacting %s:%d\n", host.c_str(), port);

		HttpServer::Request request;
		request.setMethod("GET");
		request.setPath(path);

		FILE *fhd = nullptr;

		if (!file.empty()) {
			fhd = fopen(file.c_str(), "w");
		}

		request.addHeader("host", host, 0);
		request.addHeader("user-agent", "pacman/7.1.0 (Linux x86_64) libalpm/16.0.1", 0);
		request.addHeader("accept", "*/*", 0);

		HttpClient client(host, port);
		HttpServer::Response *response = client.get(&request, fhd);

		printf("client.get ready\n");

		if (response->status() != 200) {
			printf("wrong response status\n");
		}
		else {
			if (fhd) {
				fclose(fhd);
				fhd = nullptr;
			}
			else {
				std::vector<uint8_t> body;

				int status = response->body(body);

				std::string b(body.begin(), body.end());;
				printf("body: (%d) >%s<\n", status, b.c_str());
			}
		}
	}

	return 0;
}
