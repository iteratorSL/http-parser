#include "http_parser.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>



class Result{
public:
	std::vector<std::string> header_field;
	std::vector<std::string> header_value;
	std::string url;
	std::string status;
	std::string body;
};

class HTTP_PARSER {
public:
	HTTP_PARSER(const char* buf):
		buf_(buf)
	{
		settings_.on_message_begin = HTTP_PARSER::OnMessageBeginCallback;
		settings_.on_url = HTTP_PARSER::OnURLCallback;
		settings_.on_status = HTTP_PARSER::OnStatusCallback;
		settings_.on_header_field = HTTP_PARSER::OnHeaderFieldCallback;
		settings_.on_header_value = HTTP_PARSER::OnHeaderValueCallback;
		settings_.on_headers_complete = HTTP_PARSER::OnHeadersCompleteCallback;
		settings_.on_body = HTTP_PARSER::OnBodyCallback;
		settings_.on_message_complete = HTTP_PARSER::OnMessageComplete;
		settings_.on_chunk_header = HTTP_PARSER::OnChunkHeaderCallback;
		settings_.on_chunk_complete = HTTP_PARSER::OnChunkCompleteCallback;
	}
	~HTTP_PARSER() {}
	void Init() {
		http_parser_init(&parser_, HTTP_REQUEST);
	}
	void RunParser() {
		http_parser_execute(&parser_, &settings_, buf_.c_str(), buf_.length());
	}
private:
	static int OnMessageBeginCallback(http_parser* parser) {
		std::cout << "Start parse message" << std::endl;
		parser->data = new Result();
		return 0;
	}
	static int OnURLCallback(http_parser* parser, const char* at, size_t length) {
		Result * resultData = (Result*)parser->data;
		resultData->url.assign(at, length);
		return 0;
	}
	static int OnStatusCallback(http_parser* parser, const char* at, size_t length) {
		Result * resultData = (Result*)parser->data;
		resultData->status.assign(at, length);
		return 0;
	}
	static int OnHeaderFieldCallback(http_parser* parser, const char* at, size_t length) {
		Result * resultData = (Result*)parser->data;
		std::string field;
		field.assign(at, length);
		resultData->header_field.push_back(field);
		return 0;
	}
	static int OnHeaderValueCallback(http_parser* parser, const char* at, size_t length) {
		Result * resultData = (Result*)parser->data;
		std::string value;
		value.assign(at, length);
		resultData->header_value.push_back(value);
		return 0;
	}
	static int OnHeadersCompleteCallback(http_parser* parser) {
		std::cout << "parse header complete." << std::endl;
		return 0;
	}
	static int OnBodyCallback(http_parser* parser, const char* at, size_t length) {
		std::cout << "parse body begin." << std::endl;
		Result * resultData = (Result*)parser->data;
		resultData->body.assign(at, length);
		std::cout << "Parse body complete" << std::endl;
		return 0;
	}
	static int OnMessageComplete(http_parser* parser) {
		std::cout << "parse message complete." << std::endl;
		return 0;
	}
	static int OnChunkHeaderCallback(http_parser* parser){
		return 0;
	}
	static int OnChunkCompleteCallback(http_parser* parser) {
		return 0;
	}

	std::string					buf_;
	http_parser_settings		settings_;
public:
	http_parser					parser_;
};

int main() {

	const char * buf = "GET /get_funky_content_length_body_hello HTTP/1.0\r\n"
		"conTENT-Length: 5\r\n"
		"\r\n"
		"HELLO";
	HTTP_PARSER parser(buf);
	parser.Init();
	parser.RunParser();

	Result* result = (Result*)parser.parser_.data;
	for (auto& field : result->header_field) {
		std::cout << "field : " << field << " " << std::endl;
	}
	for (auto& value : result->header_value) {
		std::cout << "value : " << value << " " << std::endl;
	}

	std::cout << "body : " << result->body << std::endl;
	return 0;
}