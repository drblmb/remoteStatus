#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#define BUFFER_SIZE 256

using boost::asio::ip::tcp;
using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      cerr << "Usage: client <host> <port>" << endl;
      return 1;
    }

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], argv[2]);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }
    if (error)
      throw boost::system::system_error(error);

    while(1)
    {
      char in[BUFFER_SIZE];
      boost::array<char, BUFFER_SIZE> buf;
      boost::system::error_code error;

      cout << "\n\nPlease enter command ('mem' or 'cpu') ->";

      cin.getline(in, BUFFER_SIZE);
      size_t in_length = strlen(in);

      if (in_length == BUFFER_SIZE) {
        cout << "String is too long\n";
        continue;
      }

      // Add newline
      in[in_length++] = '\n'; 

      socket.write_some(boost::asio::buffer(in, in_length), error);

      if (error == boost::asio::error::eof)
        break; // Connection closed 
      else if (error)
        throw boost::system::system_error(error);

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof)
        break; // Connection closed 
      else if (error)
        throw boost::system::system_error(error);

      cout.write(buf.data(), len);
    }
  }
  catch (exception& e)
  {
    cerr << e.what() << endl;
  }

  return 0;
}

