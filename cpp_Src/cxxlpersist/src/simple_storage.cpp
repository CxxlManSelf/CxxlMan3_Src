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

    std::shared_ptr<ISimplePersistContainer> cxxlFASTCALL
            ISimplePersistContainer_StringTreeNode::create(const std::shared_ptr<TreeNode<std::u8string>> &treeNode_ptr)
            {

            }
}