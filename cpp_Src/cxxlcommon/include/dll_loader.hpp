/*****************************************************************************
 * dll_loader.hpp v1.0.8
 *
 * 跨平臺動態連結程式庫載入器介面，取得的函數都用 std::function 包裹。
 * 所有載入器的持有者放棄持有，以及所有取得的函數銷毀後，動態連結程式庫才會自動卸載
 *
 * Author: CxxlMan
 * Date: 2025 -
 *****************************************************************************/
#ifndef __CXXLCOMMON_DLL_LOADER_HPP_CxxlMan3
#define __CXXLCOMMON_DLL_LOADER_HPP_CxxlMan3

#include <memory>
#include <string>
#include <functional>

#include "sysdef.hpp"
#include "commondef.hpp"
#include "cxxlcommon.hpp"

namespace CXXL
{
    class IDllLoader
    {
        // 取得包裹自己的 std::shared_ptr
        virtual std::shared_ptr<IDllLoader> cxxlFASTCALL getDllLoader() const = 0;

        virtual void *cxxlFASTCALL getProcAddress(const cxxlSTDSTRING &procName) const = 0;

    protected:
        IDllLoader() {}

    public:
        virtual ~IDllLoader() {}

        // 取得指定的函數
        // procName 程式庫中的函數名稱
        // 若無法取得回覆 nullptr
        template <typename T>
        std::function<T> cxxlFASTCALL getProc(const cxxlSTDSTRING &procName) const
        {
            using FuncPtr = typename std::add_pointer<T>::type;
            FuncPtr func = reinterpret_cast<FuncPtr>(getProcAddress(procName));
            
            if(func == nullptr) 
                return std::function<T>();
            
            return [func](auto&&...args) -> decltype(auto)
            {
                return func(std::forward<decltype(args)>(args)...);
            };
        }

        // 建立載入器，會載入指定的動態連結程式庫
        // 若無法載入回覆 nullptr
        static CXXLCOMMON_DLLEXPORT std::shared_ptr<IDllLoader> cxxlFASTCALL create(const cxxlSTDSTRING &dllPath);
    };
    
}

#endif