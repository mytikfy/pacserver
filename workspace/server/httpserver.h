#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <stddef.h>
#include <stdint.h>
#include <mqueue.h>
#include <pthread.h>

#include <string>
#include <map>
#include <vector>

class HttpServer
{
		static const size_t BUFFER_SIZE	= 16 * 1024;

	public:
		class HeaderLine
		{

			public:
				static const int HEADER_UNKNOWN				= 0;
				static const int HEADER_BASE				= 1;
				static const int HEADER_USERAGENT			= 2;
				static const int HEADER_CONTENTLENGTH		= 3;
				static const int HEADER_LASTMODIFIED		= 4;

			public:
				class Base
				{
					public:
						Base(const std::string& key, const std::string& value, uint32_t flags = 0);

						std::string key() const;

						virtual std::string value() const;
						virtual int type() const;
						virtual std::string toString() const;

					public:
						static Base *instance(const std::string& line);

					private:
						std::string m_key;
						std::string m_value;
						uint32_t m_flags;
				};

				class UserAgent : public Base
				{
					typedef Base super;

					public:
						UserAgent(const std::string& value);

						virtual int type() const override;

						std::string product() const;
						std::string productVersion() const;
						std::string system() const;

					private:
						void parse(const std::string& line);

					private:
						std::string m_product;
						std::string m_productVersion;
						std::string m_system;
				};

				class LastModified : public Base
				{
					typedef Base super;

					public:
						LastModified(const std::string& value);

//						std::string value() const override;
						int type() const override;

						time_t lastModified() const;

					private:
						void parse(const std::string& line);

					private:
						time_t m_lastModified = 0;
				};

				class ContentLength : public Base
				{
					typedef Base super;

					public:
						ContentLength(const std::string& value);

						std::string value() const override;
						int type() const override;

						size_t contentLength() const;

					private:
						void parse(const std::string& line);

					private:
						size_t m_contentLength = -1;
				};

				class TestHL : public Base
				{
					typedef Base super;

					public:
						TestHL(const std::string& value);
				};
		};

		class Request
		{
			public:
				bool parseMethod(const std::string& line);

				void setMethod(const std::string& method);
				void setPath(const std::string& path);
				void setPeer(const char *name);
				void addHeader(HeaderLine::Base *headerline);
				void addHeader(const std::string& key, const std::string& value, uint32_t flags = 0);

				HeaderLine::Base *findHeader(const std::string& key);
				const std::string findArgument(const std::string& key);
				std::string userAgent();

				std::string method() const;
				std::string path() const;
				std::string url() const;
				std::vector<std::string> headers() const;
				std::string peer() const;

				void print() const;

				std::string toString() const;

			private:
				void parsePath();

			private:
				std::map<std::string, std::string> m_arguments;
				std::string m_query;
				std::string m_method;
				std::string m_path;
				std::string m_protocol = "HTTP/1.1";
				std::string m_fragment;
				std::map<const std::string, HeaderLine::Base *> m_headers;
				std::string m_peer;
		};

		class Response
		{
			public:
				Response(Request *request);
				virtual ~Response();

				void addHeader(HeaderLine::Base *headerline);
				void addHeader(const std::string& key, const std::string& value, uint32_t flags = 0);
				void addBody(int c);
				void addBody(const std::string& data);

				int body(std::vector<uint8_t>& data) const;
				size_t bodySize() const;
				int status() const;
				time_t lastModifiedGMT();

				void setProtocol(const std::string& line);
				void setStatus(int status, const std::string& msg = "");

				void setBody(const std::string& body);
				void setBody(FILE *file);
				void close();

				HeaderLine::Base *findHeader(const std::string& key);

				virtual const std::string headers() const;

				virtual void prepare();

			private:
				Request *m_request = nullptr;

				std::string m_formatted = "";

				std::map<const std::string, HeaderLine::Base *> m_headers;

				std::string m_protocol;

				struct {
					int code = 200;
					std::string msg = "OK";
				} m_status;

				struct {
					std::vector<uint8_t> data;
					FILE *file = nullptr;
					size_t size = 0;
				} m_body;
		};

		class Worker
		{
			public:
				typedef struct {
					int type;
					int socket;
				} IPC_Data;

			public:
				Worker(int id);

				void start();
				void term();
				bool isIdle() const;
				int id() const;

				virtual Request *parse(const char *data, size_t size);

			protected:
				virtual Response *process(Request *request);

			private:
				void *run();

			private:
				static void *trampoline(void *args);

			private:
				int m_id = -1;
				bool m_term = false;
				bool m_idle = true;

				pthread_t m_thread = 0;

				struct {
					std::string name;
					mqd_t read = 0;
				} m_mq;
		};

		class Tools
		{
			public:
				// IMF-fixdate conversion, RFC5322
				static time_t fromHttpDate(const std::string& date);
				static bool endsWith(const std::string& line, const std::string& ending);
				static std::vector<std::string> split(const std::string& line, char tk);
				static std::string fmttime(int offset);
				static std::string join(const std::vector<std::string>& pack, int index, char sep);
				static int mkdirs(const std::string& ddir, const std::vector<std::string>& path);
				static std::string toHex(int value);
				static inline void ltrim(std::string& s);
				static inline void rtrim(std::string& s);
				static inline std::string fromUrlEncoding(const std::string& from);


		};

	private:
		typedef struct {
			mqd_t write = 0;
			Worker *worker = nullptr;
		} ThreadController;

	public:
		HttpServer(int port);
		virtual ~HttpServer();

		void run(int threads = 1);

	protected:
		// virtual Response *process(Request *request);

		virtual Worker *createWorker(int index);
		void term();

	private:
		void accept(int serverSocket);
		// Request *parse(const char *data, size_t size);

	private:
		int m_port = 80;
		bool m_term = false;

		std::vector<ThreadController *> m_workers;
};

#endif /* HTTPSERVER_H */

