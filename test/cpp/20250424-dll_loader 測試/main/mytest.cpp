// ========================================================================
// 檔案名稱：mytest.cpp
// 專案名稱：跨平台動態連結庫載入測試
//
// 功能描述：
//   此程式展示如何在 C++ 中動態載入 DLL/SO/DYLIB 並調用其中的函數。
//   主要功能包括：
//   1. 使用 IDllLoader 介面動態載入插件
//   2. 從 DLL 中獲取並調用四則運算函數
//   3. 使用線程池進行多執行緒計算
//   4. 使用信號量確保多執行緒環境下的安全輸出
//
// 依賴標頭檔：
//   - dll_loader.hpp  : 跨平台的動態連結庫載入器介面
//   - threadmgr.hpp   : 線程池管理
//   - semaphore.hpp   : 信號量實作，用於多執行緒同步
//
// 編譯要求：
//   - C++20 標準
//   - 連結 cxxlcore 和 cxxlcommon 函數庫
// ========================================================================

#include <iostream>
#include <random>
#include <chrono>

#include <dll_loader.hpp>   // 動態載入 DLL 的介面
#include <threadmgr.hpp>    // 線程池管理
#include <semaphore.hpp>    // 信號量，用於多執行緒同步

using namespace CxxlMan3;

// ========================================================================
// 輔助函數：u8string 轉換為可輸出的字串
// ========================================================================
/**
 * @brief 將 cxxlSTDSTRING (可能是 u8string) 轉換為可用 std::cout 輸出的字串
 * @param str 要轉換的字串
 * @return std::string 可輸出的字串
 *
 * 在 C++20 環境下，cxxlSTDSTRING 是 std::u8string (char8_t)，
 * 無法直接用 std::cout 輸出，需要轉換為普通的 std::string (char)。
 * 在舊版 C++ 環境下，cxxlSTDSTRING 已經是 std::string，直接返回。
 */
std::string to_output_string(const cxxlSTDSTRING& str)
{
#if defined(__cpp_lib_char8_t)
    // C++20: u8string -> string
    return std::string(reinterpret_cast<const char*>(str.c_str()));
#else
    // 舊版 C++: 已經是 string，直接返回
    return str;
#endif
}

// ========================================================================
// 全域變數
// ========================================================================
// 全域線程池
// ThreadPool<false> 表示非阻塞式線程池
// 用於多執行緒計算和結果輸出
ThreadPool<false> g_threadPool;

// ========================================================================
// 輔助函數：執行緒安全的輸出函數
// ========================================================================
/**
 * @brief 執行緒安全的字串輸出函數
 * @param s 要輸出的字串
 *
 * 此函數使用信號量 (Semaphore) 確保多個執行緒不會同時輸出，
 * 避免輸出內容交錯混亂。每次輸出都會自動編號。
 */
void print(const std::string &s)
{
    // 靜態信號量，初始值和最大值都是 1 (二元信號量，類似互斥鎖)
    static cxxlSemaphore sem(1,1);
    // 靜態計數器，記錄輸出次數
    static size_t count = 0;

    // 信號量輔助類別，建構時自動取得信號量，解構時自動釋放 (RAII)
    cxxlSemaphoreHelper helper(sem);

    // 輸出帶編號的訊息
    std::cout << ++count << ": " << s << '\n';
    // 立即刷新緩衝區，確保輸出顯示
    std::cout.flush();
}

// ========================================================================
// 輔助函數：處理非同步計算結果的輸出
// ========================================================================
/**
 * @brief 處理 std::future 的結果輸出
 * @param s 包含計算結果的 future 物件
 *
 * 此函數嘗試在 0.1 秒內獲取計算結果：
 * - 如果結果已經準備好，直接輸出
 * - 如果結果尚未準備好，將等待和輸出的工作提交到線程池
 *
 * 這樣可以避免主執行緒或其他工作執行緒被阻塞。
 */
void print(std::future<std::string> s)
{
    // 等待 future 結果，最多等待 100 毫秒
    std::future_status status = s.wait_for(std::chrono::milliseconds(100));

    // 如果 0.1 秒之內能夠得到結果
    if(status == std::future_status::ready)
    {
        // 直接獲取並輸出結果
        print(s.get());
    }
    else
    {
        // 如果 0.1 秒之內無法得到結果
        // 則將等待和輸出的工作提交到線程池，避免阻塞當前執行緒
        g_threadPool([s = std::move(s)]() mutable
            {
                // 在新執行緒中遞迴調用自己，繼續等待結果
                print(std::move(s));
            });
    }
}

// ========================================================================
// 跨平台動態連結庫檔案名稱設定
// ========================================================================
// 根據不同平台設定正確的 DLL 檔案名稱和副檔名：
// - Windows (MSVC)  : PlugDll.dll
// - Windows (MinGW) : libPlugDll.dll (MinGW 會自動加上 lib 前綴)
// - macOS           : libPlugDll.dylib
// - Linux           : libPlugDll.so
#if (PLATFORM_NAME == _WINDOWS_CxxlMan3)
    #ifdef _MSC_VER
        cxxlSTDSTRING libPath = u8"PlugDll.dll";
    #else
        cxxlSTDSTRING libPath = u8"libPlugDll.dll";
    #endif
#elif (PLATFORM_NAME == _MAC_CxxlMan3)
    cxxlSTDSTRING libPath = u8"libPlugDll.dylib";
#elif (PLATFORM_NAME == _LINUX_CxxlMan3)
    cxxlSTDSTRING libPath = u8"libPlugDll.so";
#endif

// ========================================================================
// 主函數：程式進入點
// ========================================================================
/**
 * @brief 主函數
 * @param argc 命令列參數個數
 * @param argv 命令列參數陣列
 * @return 0 表示成功，-1 表示失敗
 *
 * 程式執行流程：
 * 1. 動態載入插件 DLL
 * 2. 從 DLL 中獲取四則運算函數指標
 * 3. 執行 100 次隨機運算測試
 * 4. 使用線程池進行多執行緒計算和輸出
 * 5. 等待所有任務完成
 */
int main(int argc, char *argv[])
{
    // ====================================================================
    // 步驟 1：初始化隨機數生成器
    // ====================================================================
    // 使用當前時間作為隨機數種子
    srand(static_cast<unsigned>(time(nullptr)));

    // ====================================================================
    // 步驟 2：動態載入插件 DLL
    // ====================================================================
    // 使用 IDllLoader 介面的工廠方法創建載入器實例
    // 此方法會根據平台自動選擇正確的載入方式：
    // - Windows: LoadLibrary/GetProcAddress
    // - Linux:   dlopen/dlsym
    // - macOS:   dlopen/dlsym
    std::shared_ptr<IDllLoader> loader = IDllLoader::create(libPath);
    if (loader == nullptr || !loader->isValid())
    {
        std::cout << "插件檔案載入失敗: " << to_output_string(libPath) << std::endl;
        return -1;
    }

    std::cout << "插件載入成功: " << to_output_string(libPath) << std::endl;

    // ====================================================================
    // 步驟 3：從 DLL 中獲取四則運算函數
    // ====================================================================
    // 使用 getProc 模板方法獲取函數指標
    // 模板參數 <std::string(float, float)> 指定函數簽名
    // 這些函數在 DLL 中使用 extern "C" 和 CXXL_DLLEXPORT 導出
    std::function<std::string(float, float)> ops[] =
        {
            loader->getProc<std::string(float, float)>(u8"add"),     // 加法
            loader->getProc<std::string(float, float)>(u8"sub"),     // 減法
            loader->getProc<std::string(float, float)>(u8"mul"),     // 乘法
            loader->getProc<std::string(float, float)>(u8"divide")   // 除法
        };

    // 檢查所有函數是否成功獲取
    const char* opNames[] = {"add", "sub", "mul", "divide"};
    for (size_t i = 0; i < 4; i++)
    {
        if (!ops[i])
        {
            std::cout << "無法獲取函數: " << opNames[i] << std::endl;
            return -1;
        }
    }
    std::cout << "成功獲取所有四則運算函數" << std::endl;

    // ====================================================================
    // 步驟 4：執行 100 次隨機運算測試
    // ====================================================================
    // 使用線程池進行多執行緒計算，展示：
    // 1. 動態載入的函數可以正常調用
    // 2. 多執行緒環境下的非同步計算
    // 3. 執行緒安全的結果輸出
    for (int i = 0; i < 100; i++)
    {
        // 隨機選擇一個四則運算函數
        std::function<std::string(float,float)> &op = ops[rand() % 4];

        // 隨機生成兩個 0-99 之間的浮點數
        float a = float(rand() % 100);
        float b = float(rand() % 100);

        // 將運算任務提交到線程池
        // g_threadPool(op, a, b) 會在線程池的某個工作執行緒中執行 op(a, b)
        // 返回值是 std::optional<std::future<std::string>>
        // - 如果線程池接受任務，返回包含 future 的 optional
        // - 如果線程池已滿或關閉，返回空的 optional
        std::optional<std::future<std::string>> res = g_threadPool(op,a,b);

        // 處理計算結果的輸出
        if (res)
        {
            // 將 future 移動出 optional
            std::future<std::string> s = std::move(res.value());

            // 將結果輸出任務也提交到線程池
            // 使用 lambda 捕獲 future (move 語意)
            g_threadPool([s = std::move(s)]() mutable
                {
                    // 在工作執行緒中調用 print 函數
                    // print 函數會處理等待 future 結果和執行緒安全輸出
                    print(std::move(s));
                });
        }
    }

    // ====================================================================
    // 步驟 5：等待所有任務完成
    // ====================================================================
    // 阻塞主執行緒，直到線程池中的所有任務都執行完畢
    // 確保程式結束前所有運算和輸出都已完成
    g_threadPool.waitAllTask();

    std::cout << "\n所有運算任務已完成\n" << std::endl;

    // ====================================================================
    // 步驟 6：錯誤處理測試
    // ====================================================================
    std::cout << "=== 錯誤處理測試 ===" << std::endl;

    // 測試獲取不存在的函數
    auto invalidFunc = loader->getProc<std::string(float, float)>(u8"nonexistent");
    if (!invalidFunc)
    {
        std::cout << "✓ 正確處理：無法獲取不存在的函數 'nonexistent'" << std::endl;
    }
    else
    {
        std::cout << "✗ 錯誤：不應該能獲取不存在的函數" << std::endl;
    }

    // 測試載入不存在的 DLL
    auto invalidLoader = IDllLoader::create(u8"nonexistent_dll.dll");
    if (!invalidLoader || (invalidLoader && !invalidLoader->isValid()))
    {
        std::cout << "✓ 正確處理：無法載入不存在的 DLL" << std::endl;
    }
    else
    {
        std::cout << "✗ 錯誤：不應該能載入不存在的 DLL" << std::endl;
    }

    // ====================================================================
    // 步驟 7：DLL 生命週期管理測試
    // ====================================================================
    std::cout << "\n=== DLL 生命週期管理測試 ===" << std::endl;

    // 從現有 loader 獲取一個函數用於測試
    auto testFunc = loader->getProc<std::string(float, float)>(u8"add");

    if (testFunc)
    {
        // 先測試函數是否正常工作
        std::string result1 = testFunc(100.0f, 200.0f);
        std::cout << "釋放 loader 前測試: " << result1 << std::endl;

        // 釋放 loader 的 shared_ptr
        // 如果 dll_loader 正確實現了生命週期管理，
        // testFunc 內部持有的 shared_ptr 會保持 DLL 載入狀態
        loader.reset();
        std::cout << "已釋放 loader 的 shared_ptr" << std::endl;

        // 測試函數是否仍然有效
        // 這證明了 lambda 捕獲的 shared_ptr 正確延長了 DLL 的生命週期
        std::string result2 = testFunc(300.0f, 400.0f);
        std::cout << "✓ 釋放 loader 後測試成功: " << result2 << std::endl;
        std::cout << "✓ DLL 生命週期管理正確：函數對象持有的 shared_ptr 保持了 DLL 的載入狀態" << std::endl;
    }
    else
    {
        std::cout << "✗ 無法獲取測試函數" << std::endl;
    }

    std::cout << "\n=== 所有測試完成 ===" << std::endl;

    return 0;
}