//
// manager.hpp
// ~~~~~~~~~~
//
// Session‚Ìõ–½‚ğŠÇ—‚·‚émanager
//

#ifndef BOOSTCONNECT_MANAGER
#define BOOSTCONNECT_MANAGER

#include <set>
#include <boost/shared_ptr.hpp>

namespace bstcon{

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
  
} // namespace bstcon

#endif
