#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <sstream>
constexpr unsigned short SERVER_PORT = 8000;

int main() {
    try {
        std::cout << "Listen on port " << SERVER_PORT << std::endl;

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), SERVER_PORT));
        boost::system::error_code error;
        boost::asio::ip::tcp::socket socket(io_service);
        acceptor.accept(socket);

        std::cout << "Get client connection." << std::endl;
        boost::asio::streambuf request_buf;
        boost::asio::read_until(socket, request_buf, "\n\n");
        std::istream request_stream(&request_buf);
        boost::array<char, 1024> buf;
        std::string file_path;
        size_t file_size = 0;
        request_stream >> file_path;
        request_stream >> file_size;
        request_stream.read(buf.c_array(), 2);
        size_t pos = file_path.find_last_of('\\');
        if (pos != std::string::npos)
            file_path = file_path.substr(pos + 1);
        std::cout << "\"" << file_path << "\" size is " << file_size << " bytes" << std::endl;

        std::ofstream output_file(file_path.c_str(), std::ios_base::binary);
        if (!output_file) {
            std::cout << "Failed to open " << file_path << std::endl;
            return __LINE__;
        }

        while (true) {
            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            if (len > 0)
                output_file.write(buf.c_array(), (std::streamsize)len);
            if (output_file.tellp() == (std::fstream::pos_type)(std::streamsize)file_size)
                break;
            if (error) {
                std::cout << error << std::endl;
                break;
            }
        }
        std::cout << "Received " << output_file.tellp() << " bytes.\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}