////
//// Created by Djevayo Pattij on 8/22/20.
////
//
//#ifndef WEBSERV_HTTP_HTTPPARSER_HPP
//#define WEBSERV_HTTP_HTTPPARSER_HPP
//
//#include <map>
//#include <string>
//
//namespace http {
//
//class HttpParser {
//public:
//    HttpParser();
//    virtual ~HttpParser();
//
//    std::string getBody();
//    std::string getHeader(const std::string& name);
//    std::map<std::string, std::string> getHeaders();
//    std::string getMethod();
//    std::string getURL();
//    std::string getVersion();
//    std::string getStatus();
//    std::string getReason();
//
//    bool hasHeader(const std::string& name);
//
//    void parse(std::string message);
//    void parse(Socket & socket);
//    void parseResponse(std::string message);
//
//private:
//    std::string method;
//    std::string url;
//    std::string version;
//    std::string body;
//    std::string status;
//    std::string reason;
//    std::map<std::string, std::string> headers;
//
//    void parseRequestLine(std::string& line);
//    void parseStatusLine(std::string& line);
//
//};
//}
//
//#endif //WEBSERV_HTTP_HTTPPARSER_HPP
