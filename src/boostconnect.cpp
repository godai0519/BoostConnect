//
// boostconnect.cpp
// ~~~~~~~~~~
//
// Library(.so or .lib) Build Source File
// boost/config.hpp is Setting file. You can set USE SSL.
//

#include <boostconnect/config.hpp>

#include <boostconnect/request.hpp>
#include <boostconnect/response.hpp>

#include <boostconnect/utility/impl/radix.ipp>
#include <boostconnect/utility/impl/percent_encoder.ipp>
#include <boostconnect/utility/impl/syntax.ipp>

#include <boostconnect/content/content_base.hpp>
#include <boostconnect/content/text.hpp>
#include <boostconnect/content/binary.hpp>
#include <boostconnect/content/urlencoded.hpp>

#include <boostconnect/application_layer/impl/socket_base.ipp>
#include <boostconnect/application_layer/impl/tcp_socket.ipp>
#include <boostconnect/application_layer/impl/ssl_socket.ipp>

#include <boostconnect/connection_type/impl/connection_base.ipp>
#include <boostconnect/connection_type/impl/async_connection.ipp>
#include <boostconnect/connection_type/impl/sync_connection.ipp>

#include <boostconnect/session/impl/session_base.ipp>
#include <boostconnect/session/impl/http_session.ipp>

#include <boostconnect/system/impl/error_code.ipp>

#include <boostconnect/impl/manager.ipp>
#include <boostconnect/impl/client.ipp>
#include <boostconnect/impl/server.ipp>