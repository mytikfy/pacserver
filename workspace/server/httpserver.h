#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <stddef.h>
#include <stdint.h>

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
				void parseMethod(const std::string& line);

				void setMethod(const std::string& method);
				void setPath(const std::string& path);
				void addHeader(HeaderLine::Base *headerline);
				void addHeader(const std::string& key, const std::string& value, uint32_t flags = 0);

				HeaderLine::Base *findHeader(const std::string& key);
				const std::string findArgument(const std::string& key);
				std::string userAgent();

				std::string method() const;
				std::string path() const;
				std::string url() const;
				std::vector<std::string> headers() const;

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
		};

		class Response
		{
			public:
				Response(Request *request);
				virtual ~Response();

				void addHeader(HeaderLine::Base *headerline);
				void addHeader(const std::string& key, const std::string& value, uint32_t flags = 0);
				void addBody(int c);
				// std::string body() const;
				int body(std::vector<uint8_t>& data) const;
				int status() const;
				time_t lastModifiedGMT();

				void setProtocol(const std::string& line);
				void setStatus(int status, const std::string& msg = "");

				void setBody(const std::string& body);
				void setBody(FILE *file);

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
					FILE *file;
					size_t size;
				} m_body;
		};

		class Tools
		{
			public:
				// IMF-fixdate conversion, RFC5322
				static time_t fromHttpDate(const std::string& date);
				static bool endsWith(const std::string& line, const std::string& ending);
		};

	public:
		HttpServer(int port);
		virtual ~HttpServer();

		void run();

	protected:
		virtual Response *process(Request *request);

	private:
		void accept(int serverSocket);
		Request *parse(const char *data, size_t size);

	private:
		int m_port = 80;
		bool m_term = false;
};

#endif /* HTTPSERVER_H */

