//
// manager.hpp
// ~~~~~~~~~~
//
// Session‚Ìõ–½‚ğŠÇ—‚·‚émanager
//

#ifndef BOOSTCONNECT_SESSION_MANAGER
#define BOOSTCONNECT_SESSION_MANAGER

#include <boost/enable_shared_from_this.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include "../application_layer/socket_base.hpp"
#include "../request.hpp"
#include "../response.hpp"

namespace bstcon{
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
} // namespace bstcon

#endif
