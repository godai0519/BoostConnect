//
// manager.hpp
// ~~~~~~~~~~
//
// Sessionの寿命を管理するmanager
//

#ifndef BOOSTCONNECT_MANAGER_HPP
#define BOOSTCONNECT_MANAGER_HPP

#include <set>
#include <boost/shared_ptr.hpp>

namespace bstcon{

template<class ManagedType>
class manager /*final*/ {
public:
    typedef boost::shared_ptr<ManagedType> ManagedPtr;
    typedef std::set<ManagedPtr> ManagedSet;

    manager();
    virtual ~manager();

    const ManagedSet& data() const;
    void run(ManagedPtr& object);
    void stop(ManagedPtr& object);

private:
    ManagedSet data_;
};
    
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/manager.ipp"
#endif

#endif
