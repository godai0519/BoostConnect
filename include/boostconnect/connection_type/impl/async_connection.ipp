//
// async_connection.ipp
// ~~~~~~~~~~
//
// Specialization from connection_base to ASync Connection
//

#ifndef BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION_IPP
#define BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION_IPP

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include "../async_connection.hpp"

namespace bstcon{
namespace connection_type{

async_connection::async_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
{
	socket_ = socket;
	read_buf_ = boost::make_shared<boost::asio::streambuf>();
	return;
}
async_connection::~async_connection()
{
}

async_connection::connection_ptr async_connection::connect(const std::string& host, ConnectionHandler handler)
{
	auto resolver = boost::make_shared<boost::asio::ip::tcp::resolver>(socket_->get_io_service());
	boost::asio::ip::tcp::resolver::query query(host, socket_->service_protocol());

	auto self = shared_from_this();
	resolver->async_resolve(
		query,
		[this, self, resolver, handler](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator ep_iterator)
		{
			if (ec)
			{
				std::cout << "Error Resolve: " << ec.message() << std::endl;
				return;
			}

			boost::asio::async_connect(
				socket_->lowest_layer(),
				ep_iterator,
				boost::bind(&async_connection::handle_connect, shared_from_this(), boost::asio::placeholders::error, handler)
				);

			return;
		}
	);

	return self;
}

async_connection::connection_ptr async_connection::connect(const endpoint_type& ep, ConnectionHandler handler)
{
	socket_->lowest_layer().async_connect(ep,
		boost::bind(&async_connection::handle_connect, shared_from_this(), boost::asio::placeholders::error, handler)
		);

	return shared_from_this();
}

async_connection::connection_ptr async_connection::accepted(ConnectionHandler handler)
{
#ifdef USE_SSL_BOOSTCONNECT
    socket_->async_handshake(
        application_layer::socket_base::ssl_socket_type::handshake_type::server,
        boost::bind(handler, shared_from_this(), boost::asio::placeholders::error)
        );
#else
    if(handler) handler(shared_from_this(), error_code());
#endif

    return shared_from_this();
}

std::future<std::size_t> async_connection::write(const boost::shared_ptr<boost::asio::streambuf>& buf, WriteHandler handler)
{
	const auto p = boost::make_shared<std::promise<std::size_t>>();
	const auto self = shared_from_this();

	boost::asio::async_write(*socket_, *buf,
		[this, self, p, handler](const error_code& ec, const std::size_t sz)
		{
			handler(sz);
			p->set_value(sz);
			return;
		}
	);

	return p->get_future();
}

std::future<std::string> async_connection::read(ReadHandler handler)
{
	const auto p = boost::make_shared<std::promise<std::string>>();
	const auto self = shared_from_this();
    
	boost::asio::async_read(*socket_, *read_buf_, boost::asio::transfer_all(),
		[this, self, p, handler](const error_code& ec, const std::size_t)
		{
			const boost::asio::streambuf::const_buffers_type buf = read_buf_->data();
			const std::string received(boost::asio::buffers_begin(buf), boost::asio::buffers_end(buf));
			read_buf_->consume(received.size());

			if (handler) handler(received);
			p->set_value(received);
			return;
		}
	);

	return p->get_future();
}

std::future<std::string> async_connection::read_size(const std::size_t size, ReadHandler handler)
{
	const auto p = boost::make_shared<std::promise<std::string>>();
	const auto self = shared_from_this();
    
    auto next_handler =
		[this, self, p, size, handler](const error_code& ec, const std::size_t)
		{
			const boost::asio::streambuf::const_buffers_type buf = read_buf_->data();
			const std::string received(boost::asio::buffers_begin(buf), boost::asio::buffers_begin(buf) + size);
			read_buf_->consume(size);

			if (handler) handler(received);
			p->set_value(received);
			return;
		};

    if(read_buf_->size() < size)
        boost::asio::async_read(*socket_, *read_buf_, boost::asio::transfer_exactly(size - read_buf_->size()), next_handler);
    else
        next_handler(error_code(), 0);

	return p->get_future();
}

std::future<std::string> async_connection::read_until(const std::string& until, ReadHandler handler)
{
	const auto p = boost::make_shared<std::promise<std::string>>();
	const auto self = shared_from_this();

	boost::asio::async_read_until(*socket_, *read_buf_, until,
		[this, self, p, handler](const error_code& ec, const std::size_t sz)
		{
			const boost::asio::streambuf::const_buffers_type buf = read_buf_->data();
			const std::string received(boost::asio::buffers_begin(buf), boost::asio::buffers_begin(buf) + sz);
			read_buf_->consume(sz);

			if (handler) handler(received);
			p->set_value(received);
			return;
		}
	);

	return p->get_future();
}

void async_connection::handle_connect(const error_code& ec, ConnectionHandler handler)
{
	if (!ec)
	{
#ifdef USE_SSL_BOOSTCONNECT
		socket_->async_handshake(
			application_layer::socket_base::ssl_socket_type::handshake_type::client,
			boost::bind(handler, shared_from_this(), boost::asio::placeholders::error)
			);
#else
        if(handler) handler(shared_from_this(), error_code());
#endif
	}
	else std::cout << "Error Connect!?" << std::endl;
}

} // namespace connection_type
} // namespace bstcon

#endif
