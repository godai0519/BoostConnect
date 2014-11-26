//
// boostconnect.cpp
// ~~~~~~~~~~
//
// Library(.so or .lib) Build Source File
// boost/config.hpp is Setting file. You can set USE SSL.
//

#include <boostconnect/config.hpp>

#include <boostconnect/utility/impl/radix.ipp>
#include <boostconnect/utility/impl/percent_encoder.ipp>
#include <boostconnect/utility/impl/syntax.ipp>

//#include <boostconnect/content/impl/content_base.ipp>
//#include <boostconnect/content/impl/text.ipp>
//#include <boostconnect/content/impl/binary.ipp>
//#include <boostconnect/content/impl/urlencoded.ipp>
//#include <boostconnect/content/impl/multipart.ipp>
//#include <boostconnect/content/impl/multipart_mixed.ipp>
//#include <boostconnect/content/impl/multipart_form_data.ipp>

#include <boostconnect/application_layer/impl/tcp_socket.ipp>
#include <boostconnect/application_layer/impl/ssl_socket.ipp>

#include <boostconnect/connection_type/impl/async_connection.ipp>
#include <boostconnect/connection_type/impl/sync_connection.ipp>

#include <boostconnect/protocol_type/impl/http.ipp>

#include <boostconnect/session_type/impl/http_session.ipp>
#include <boostconnect/utility/impl/http.ipp>

//#include <boostconnect/impl/server.ipp>
