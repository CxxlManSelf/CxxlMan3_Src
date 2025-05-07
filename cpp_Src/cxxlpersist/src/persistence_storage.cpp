
#include <persistence.hpp>

namespace CXXL
{
    // IPersistStorage 的實作
    class PersistStorage:public IPersistStorage
    {
        std::shared_ptr<IPersistContainer> m_container_ptr;

        virtual bool cxxlFASTCALL save(Persistable *pPersistable) override
        {
            
        }

        virtual bool cxxlFASTCALL load(Persistable *pPersistable) override
        {
            
        }

    public:
        // Constructor
        PersistStorage(const std::shared_ptr<IPersistContainer> &container) 
        : m_container_ptr(container)
        {}

        // Destructor
        virtual ~PersistStorage() {}
    };

    std::shared_ptr<IPersistStorage> cxxlFASTCALL
    defaultPersistStorage(const std::shared_ptr<IPersistContainer> &container)
    {
        return std::shared_ptr<IPersistStorage>(new PersistStorage(container));
    }
}