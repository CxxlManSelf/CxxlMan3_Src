#include <simple_persist_storage.hpp>

namespace CXXL
{
    class SimplePersistStorage: public ISimplePersistStorage
    {
    public:
        virtual ~SimplePersistStorage() {}
        virtual bool cxxlFASTCALL
        save(IPersistChannel *pPersistable) override;
        virtual bool cxxlFASTCALL
        load(IPersistChannel *pPersistable) override;
    };
}