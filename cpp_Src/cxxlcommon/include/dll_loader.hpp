/*****************************************************************************
 * dll_loader.hpp v1.0.5
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

#include "cxxlcommon.hpp"

namespace CXXL
{
    class IDllLoader
    {
        // 取得包裹自己的 std::shared_ptr
        virtual std::shared_ptr<IDllLoader> cxxlFASTCALL getDllLoader() = 0;

        virtual void *cxxlFASTCALL getProcAddress(const cxxlSTDSTRING &procName) = 0;

    protected:
        IDllLoader() {}

    public:
        virtual ~IDllLoader() {}

        // 取得指定的函數
        // procName 程式庫中的函數名稱
        // 若無法取得回覆 nullptr
        template <typename RT, typename... Args>
        std::function<RT(Args...)> cxxlFASTCALL getProc(const cxxlSTDSTRING &procName)
        {
            RT(*func)(Args...) = getProcAddress(procName);
            if(func == nullptr) 
                return std::function<RT(Args...)>();

            // self_ptr 只是為了避免動態連結程式庫被卸載
            return [self_ptr = getDllLoader(),func](Args &&...args) -> RT
            {
                return func(std::forward<Args>(args)...);
            };
        }

        // 建立載入器，會載入指定的動態連結程式庫
        // 若無法載入回覆 nullptr
        static std::shared_ptr<IDllLoader> CXXLCOMMON_DLLEXPORT create(const cxxlSTDSTRING &dllPath);
    };
    
}

#endif