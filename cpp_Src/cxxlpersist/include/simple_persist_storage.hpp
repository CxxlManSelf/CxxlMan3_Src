/*****************************************************
 * simple_persist_storage.hpp v1.0.0
 *
 * CxxlMan3 提供的一個簡單 IPersistStorage 實作
 *
 *
 * Author: CxxlMan
 * Date: 2025 -
******************************************************/
#ifndef __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
#define __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3

#include <cstdint>
#include <stdfloat>
#include <memory>

#include <cxxlpersist.hpp>
#include <persist_storage.hpp>

namespace CXXL
{

    // ISimplePersistStorage 的容器介面
    class ISimplePersistContainer
    {

    public:
        virtual ~ISimplePersistContainer() {}

        // 儲存
        virtual void cxxlFASTCALL
        save(std::int8_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::int16_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::int32_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::int64_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::uint8_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::uint16_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::uint32_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::uint64_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::float32_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::float64_t *p, size_t count) = 0;

        virtual void cxxlFASTCALL
        save(std::float128_t *p, size_t count) = 0;
        

        // 讀取
        virtual bool cxxlFASTCALL
        load(std::int8_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::int16_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::int32_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::int64_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint8_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint16_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint32_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint64_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::float32_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::float64_t *p, size_t count) = 0;

        virtual bool cxxlFASTCALL
        load(std::float128_t *p, size_t count) = 0;

    };

    std::shared_ptr<ISimplePersistContainer> 
    CXXLPERSIST_DLLEXPORT defaultSimplePersistContainer();


    class ISimplePersistStorage: public IPersistStorage
    {
    protected:        
        ISimplePersistStorage() = default;
    public:        
        virtual ~ISimplePersistStorage() {}

        virtual bool cxxlFASTCALL save(IPersistChannel *pPersistable) override;
        virtual bool cxxlFASTCALL load(IPersistChannel *pPersistable) override;


        static std::shared_ptr<IPersistStorage> 
        CXXLPERSIST_DLLEXPORT create();
    };


    class SimpleSerialize: public ISerialize
    {

    };

} // namespace CXXL
#endif // __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
