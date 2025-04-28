#include <iostream>
#include <random>
#include <chrono>

#include <dll_loader.hpp>
#include <threadmgr.hpp>

using namespace CxxlMan3;

// 線程池
ThreadPool<false> g_threadPool;

// 避免多個執行緒搶著輸出
void print(const std::string &s) 
{ 
    static cxxlSemaphore sem(1,1);
    static size_t count = 0;

    cxxlSemaphoreHelper helper(sem);
    std::cout << ++count << ": " << s << '\n'; //std::endl;
    std::cout.flush();
}

// 顯示計算的結果
void print(std::future<std::string> s)
{
    std::future_status status = s.wait_for(std::chrono::milliseconds(100));

    // 如果0.1秒之內能夠得到結果
    if(status == std::future_status::ready)
        print(s.get());

        
    else
        // 如果0.1秒之內無法得到結果
        // 則將以新執行緒再執行
        
		g_threadPool([s = std::move(s)]() mutable
			{
				print(std::move(s));
			});
            
}


// Windows 和 Linux/macOS 使用不同的檔案副檔名
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

int main(int argc, char *argv[])
{
    // rand 初始化
    srand(static_cast<unsigned>(time(nullptr)));

    // 載入 dll
    std::shared_ptr<IDllLoader> loader = IDllLoader::create(libPath);
    if (loader == nullptr)
    {
        std::cout << "插件檔案載入失敗" << std::endl;
        return -1;
    }

    // 取得四則運算函數
    std::function<std::string(float, float)> ops[] =
        {
            loader->getProc<std::string(float, float)>(u8"add"),
            loader->getProc<std::string(float, float)>(u8"sub"),
            loader->getProc<std::string(float, float)>(u8"mul"),
            loader->getProc<std::string(float, float)>(u8"divide")};

    // 做100次隨機選擇四則運算式和兩個數值做運算，並且用 g_threadPool 做多執行緒的計算
    // 處理，以及結果的輸出
    for (int i = 0; i < 100; i++)
    {
        // 隨機選擇四則運算式
        std::function<std::string(float,float)> &op = ops[rand() % 4];

        // 隨機選擇兩個數值
        float a = float(rand() % 100);
        float b = float(rand() % 100);

        // 用 g_threadPool 做多執行緒的計算
        std::optional<std::future<std::string>> res = g_threadPool(op,a,b);

        // 用 g_threadPool 在子執行緒輸出結果
        if (res)        
        {
            std::future<std::string> s = std::move(res.value());
			g_threadPool([s = std::move(s)]() mutable
				{
					print(std::move(s));
				});
        }
    }

    // 等待所有任務結束
    g_threadPool.waitAllTask();

    return 0;
}