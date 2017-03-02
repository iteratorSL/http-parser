#include "http_parser.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>


typedef std::map<std::string, std::string> header_t;
typedef header_t::iterator header_iter_t;

struct HttpRequest {
	std::string http_method;
	std::string http_url;
	//std::string http_version;

	header_t    http_headers;
	std::string http_header_field; //field is waiting for value while parsing

	std::string http_body;

};


struct HttpResponse {
	//std::string http_version;
	int         http_code;
	std::string http_phrase;

	header_t    http_headers;

	std::string http_body;

	std::string GetResponse();
	void        ResetResponse();
};

std::string HttpResponse::GetResponse()
{
	std::ostringstream ostream;
	ostream << "HTTP/1.1" << " " << http_code << " " << http_phrase << "\r\n"
		<< "Connection: keep-alive" << "\r\n";

	header_iter_t iter = http_headers.begin();

	while (iter != http_headers.end())
	{
		ostream << iter->first << ": " << iter->second << "\r\n";
		++iter;
	}
	ostream << "Content-Length: " << http_body.size() << "\r\n\r\n";
	ostream << http_body;

	return ostream.str();
}

void HttpResponse::ResetResponse()
{
	//http_version = "HTTP/1.1";
	http_code = 200;
	http_phrase = "OK";

	http_body.clear();
	http_headers.clear();
}

class HttpParser
{
public:
	void InitParser(Connection *con);
	int  HttpParseRequest(const std::string &inbuf);

	static int OnMessageBeginCallback(http_parser *parser);
	static int OnUrlCallback(http_parser *parser, const char *at, size_t length);
	static int OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length);
	static int OnHeaderValueCallback(http_parser *parser, const char *at, size_t length);
	static int OnHeadersCompleteCallback(http_parser *parser);
	static int OnBodyCallback(http_parser *parser, const char *at, size_t length);
	static int OnMessageCompleteCallback(http_parser *parser);

private:
	http_parser          parser;
	http_parser_settings settings;
};

class Connection {
public:
	Connection(){}
	~Connection(){}

	HttpRequest        *http_request_parser;    //����ʱ��
	HttpRequest        *http_request_process;   //��������ʱ��
	HttpResponse        http_response;
	HttpParser          http_parser;
	std::vector<HttpRequest> req_queue;
};

int HttpParser::HttpParseRequest(const std::string &inbuf)
{
	int nparsed = http_parser_execute(&parser, &settings, inbuf.c_str(), inbuf.size());

	if (parser.http_errno != HPE_OK)
	{
		return -1;
	}

	return nparsed;
}

/* ��ʼ��http_request_parser */
int HttpParser::OnMessageBeginCallback(http_parser *parser)
{
	Connection *con = (Connection*)parser->data;

	con->http_request_parser = new HttpRequest();

	return 0;
}

/* �������õ�url��ֵ��http_url */
int HttpParser::OnUrlCallback(http_parser *parser, const char *at, size_t length)
{
	Connection *con = (Connection*)parser->data;

	con->http_request_parser->http_url.assign(at, length);

	return 0;
}

/* ����������header_field�ݴ���http_header_field�� */
int HttpParser::OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length)
{
	Connection *con = (Connection*)parser->data;

	con->http_request_parser->http_header_field.assign(at, length);

	return 0;
}

/* ����������header_value��header_fieldһһ��Ӧ */
int HttpParser::OnHeaderValueCallback(http_parser *parser, const char *at, size_t length)
{
	Connection      *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;

	request->http_headers[request->http_header_field] = std::string(at, length);

	return 0;
}

/* ���չٷ��ĵ� */
int HttpParser::OnHeadersCompleteCallback(http_parser *parser)
{
	Connection *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;
	request->http_method = http_method_str((http_method)parser->method);
	return 0;
}

/* ���������ܱ����ò�ֹһ�Σ����ʹ��append */
int HttpParser::OnBodyCallback(http_parser *parser, const char *at, size_t length)
{
	Connection *con = (Connection*)parser->data;

	con->http_request_parser->http_body.append(at, length);

	return 0;
}

/* ��������ϵ���Ϣ�ŵ���Ϣ������ */
int HttpParser::OnMessageCompleteCallback(http_parser *parser)
{
	Connection *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;

	con->req_queue.push_back(*request);
	con->http_request_parser = NULL;
	return 0;
}

int main() {

	http_parser_settings settings;
	
	http_parser parser;


	return 0;
}