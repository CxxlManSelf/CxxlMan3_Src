/*****************************************************************************
 * dll_loader.hpp v1.0.9
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
        // 用於延長 DLL 的生命週期，確保從 getProc() 返回的函數對象有效
        virtual std::shared_ptr<IDllLoader> cxxlFASTCALL getDllLoader() const = 0;

        virtual void *cxxlFASTCALL getProcAddress(const cxxlSTDSTRING &procName) const = 0;

    protected:
        IDllLoader() {}

    public:
        virtual ~IDllLoader() {}

        // 檢查 DLL 是否成功載入
        // 回傳 true 表示成功載入，false 表示載入失敗
        virtual bool cxxlFASTCALL isValid() const noexcept = 0;

        // 取得指定的函數
        // procName 程式庫中的函數名稱
        // 若無法取得回覆 nullptr
        // 返回的函數對象會持有 DLL 的 shared_ptr，確保 DLL 在函數對象存活期間不會被卸載
        template <typename T>
        std::function<T> cxxlFASTCALL getProc(const cxxlSTDSTRING &procName) const
        {
            using FuncPtr = typename std::add_pointer<T>::type;
            FuncPtr func = reinterpret_cast<FuncPtr>(getProcAddress(procName));

            if(func == nullptr)
                return std::function<T>();

            // lambda 捕獲 shared_ptr，延長 DLL 生命週期
            return [func, holder = getDllLoader()](auto&&...args) -> decltype(auto)
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