#ifndef BOOSTCONNECT_MANAGER_IPP
#define BOOSTCONNECT_MANAGER_IPP

#include <boostconnect/manager.hpp>

namespace bstcon{
    
template<class ManagedType>
manager<ManagedType>::manager()
{
}
template<class ManagedType>
manager<ManagedType>::~manager()
{
}

template<class ManagedType>
const typename manager<ManagedType>::ManagedSet& manager<ManagedType>::data() const
{
    return data_;
}

template<class ManagedType>
void manager<ManagedType>::run(ManagedPtr& object){
    data_.insert(object);
    return;
}

template<class ManagedType>
void manager<ManagedType>::stop(ManagedPtr& object){
    data_.erase(object);
    return;
}

} // namespace bstcon

#endif
