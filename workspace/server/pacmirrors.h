#ifndef PACMIRRORS_H
#define PACMIRRORS_H

#include <string>
#include <vector>
#include <tuple>

class PacMirrors
{
	public:
		class Server
		{

			public:
				Server(const std::string& host, const std::string& path, const std::string& arch);

				std::string host() const;
				std::string arch() const;

				std::string get(const std::string& path);

				int isDisabled() const;

			private:
				void disable();

			public:
				static Server *instance(const std::string& server, const std::string& arch);

			private:
				std::string m_host;
				std::vector<std::string> m_pathes;
				std::string m_arch;
				int m_disabled = false;
		};

	public:

		int load(const std::string& arch, const std::string& path);
		void print();
		Server *next(const char *arch = "x86");

	private:
		void addServer(const std::string& server, const std::string& arch);

	private:
		std::vector<Server *> m_servers;
		size_t m_pos = 0;
};

#endif	// PACMIRRORS_H

