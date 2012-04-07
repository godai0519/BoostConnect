//
// manager.hpp
// ~~~~~~~~~~
//
// Sessionの寿命を管理するmanager
//

#ifndef TWIT_LIB_PROTOCOL_SESSION_MANAGER
#define TWIT_LIB_PROTOCOL_SESSION_MANAGER

#include <boost/enable_shared_from_this.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include "../application_layer/socket_base.hpp"
#include "../request.hpp"
#include "../response.hpp"

namespace oauth{
namespace protocol{
namespace session{

template<class ManagedType>
class manager /*final*/ {
public:
  typedef boost::shared_ptr<typename ManagedType> ManagedPtr;
  typedef std::set<ManagedPtr> ManagedSet;

  const ManagedSet& data() const { return data_; }
  void run(ManagedPtr& object){
    data_.insert(object);
    return;
  }
  void stop(ManagedPtr& object){
    data_.erase(object);
    return;
  }

private:
  ManagedSet data_;
};
  
} // namespace session
} // namespace protocol
} // namespace oauth

#endif
