//
// sync_connection.ipp
// ~~~~~~~~~~
//
// Specialization from connection_base to Sync Connection
//

#ifndef BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_IPP
#define BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_IPP

#include <boost/make_shared.hpp>
#include "../sync_connection.hpp"

namespace bstcon{
namespace connection_type{

sync_connection::sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
{
	socket_ = socket;
	read_buf_ = boost::make_shared<boost::asio::streambuf>();
	return;
}

sync_connection::~sync_connection()
{
}

sync_connection::connection_ptr sync_connection::connect(const std::string& host, ConnectionHandler handler)
{
	boost::asio::ip::tcp::resolver resolver(socket_->get_io_service());
	boost::asio::ip::tcp::resolver::query query(host, socket_->service_protocol());

	// Connect Start
	error_code ec;
	boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query, ec);
	boost::asio::connect(socket_->lowest_layer(), ep_iterator, ec);

#ifdef USE_SSL_BOOSTCONNECT
	socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#endif

	handler(shared_from_this(), ec);
	return this->shared_from_this();
}

sync_connection::connection_ptr sync_connection::connect(const endpoint_type& ep, ConnectionHandler handler)
{
	// Connect Start
	error_code ec;
	socket_->lowest_layer().connect(ep, ec);
#ifdef USE_SSL_BOOSTCONNECT
	socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#endif

	handler(shared_from_this(), ec);
	return this->shared_from_this();
}

std::future<std::size_t> sync_connection::write(const boost::shared_ptr<boost::asio::streambuf>& buf, WriteHandler handler)
{
	error_code ec;
	const std::size_t wrote_size = boost::asio::write(*socket_, *buf, ec);

	if (handler) handler(wrote_size);

	std::promise<std::size_t> p;
	p.set_value(wrote_size);
	return p.get_future();
}

std::future<std::string> sync_connection::read(ReadHandler handler)
{
	error_code ec;
	boost::asio::read(*socket_, *read_buf_, boost::asio::transfer_all(), ec);

	const boost::asio::streambuf::const_buffers_type buf = read_buf_->data();
	const std::string received(boost::asio::buffers_begin(buf), boost::asio::buffers_end(buf));
	read_buf_->consume(received.size());

	if (handler) handler(received);

	std::promise<std::string> p;
	p.set_value(received);
	return p.get_future();
}

std::future<std::string> sync_connection::read_size(const std::size_t size, ReadHandler handler)
{
	error_code ec;

    if(read_buf_->size() < size)
        boost::asio::read(*socket_, *read_buf_, boost::asio::transfer_exactly(size - read_buf_->size()), ec);

	const boost::asio::streambuf::const_buffers_type buf = read_buf_->data();
	const std::string received(boost::asio::buffers_begin(buf), boost::asio::buffers_begin(buf) + size);
	read_buf_->consume(size);

	if (handler) handler(received);

	std::promise<std::string> p;
	p.set_value(received);
	return p.get_future();
}

std::future<std::string> sync_connection::read_until(const std::string& until, ReadHandler handler)
{
	error_code ec;
	const std::size_t read_size = boost::asio::read_until(*socket_, *read_buf_, until, ec);

	const boost::asio::streambuf::const_buffers_type buf = read_buf_->data();
	const std::string received(boost::asio::buffers_begin(buf), boost::asio::buffers_begin(buf) + read_size);
	read_buf_->consume(read_size);

	if (handler) handler(received);

	std::promise<std::string> p;
	p.set_value(received);
	return p.get_future();
}

} // namespace connection_type
} // namespace bstcon

#endif
