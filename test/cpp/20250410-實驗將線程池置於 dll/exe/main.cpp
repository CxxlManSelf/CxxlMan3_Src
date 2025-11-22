/**********************************************************
 * 這個範例和 20250409 範例的功能是一樣的，不同的地方在於
 * 將 g_threadPool 放到 dll 的 .cpp 檔中。
 * 
 * 當一個程式結束時，原先在 dll 中產生的 thread 會先被砍掉，
 * 沒有走正常的解構程序，為了因應這種狀況，特別為 ThreadPool 提
 * 供 waitAllTaskAndclear() 函數。
**********************************************************/


#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <random>
#include <chrono>

#include <semaphore.hpp>
#include <dll.hpp>

using namespace CxxlMan3;

// 加法運算函數，將兩數相加的結果以 string 回傳
std::string add(float a, float b)
{
    std::stringstream ss;
    ss << a << " + " << b << " = " << a + b;
    return ss.str();
}

// 減法運算函數，將兩數相加的結果以 string 回傳
std::string sub(float a, float b)
{
    std::stringstream ss;
    ss << a << " - " << b << " = " << a - b;
    return ss.str();
}

// 乘法運算函數，將兩數相加的結果以 string 回傳
std::string mul(float a, float b)
{
    std::stringstream ss;
    ss << a << " * " << b << " = " << a * b;
    return ss.str();
}

// 除法運算函數，將兩數相加的結果以 string 回傳
std::string divide(float a, float b)
{
    std::stringstream ss;
    ss << a << " / " << b << " = ";
    if (b != 0)
        ss << a / b;
    else
        ss << "Inf"; 
    return ss.str();
}

// 避免多個執行緒搶著輸出
void print(std::string s) 
{ 
    static cxxlSemaphore sem(1,1);
    static size_t count = 0;

    cxxlSemaphoreHelper helper(sem);
    std::cout << ++count << ": " << s << '\n'; //std::endl;
    std::cout.flush();
}

void print(std::future<std::string> s);

// 在執行緒池中非同步顯示結果
void asyncPrint(std::future<std::string> s)
{
	g_threadPool([s = std::move(s)]() mutable
		{
			print(std::move(s));
		});
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
        asyncPrint(std::move(s));
}


int main()
{

    // rand 初始化
    srand(static_cast<unsigned>(time(nullptr)));

    // 做100次隨機選擇四則運算式和兩個數值做運算，並且用 g_threadPool 做多執行緒的計算
    // 處理，以及結果的輸出
    for (int i = 0; i < 100; i++)
    {
        // 隨機選擇四則運算式
        std::function<std::string(float,float)> ops[] = {add, sub, mul, divide};

        std::function<std::string(float,float)> op = ops[rand() % 4];

        // 隨機選擇兩個數值
        float a = float(rand() % 100);
        float b = float(rand() % 100);

        // 用 g_threadPool 做子執行緒的計算
        std::optional<std::future<std::string>> res = g_threadPool(op, a, b);        

        // 用 g_threadPool 在子執行緒輸出結果
        if (res)        
        {
            std::future<std::string> s = std::move(res.value());
            asyncPrint(std::move(s));
        }

    }

    // 必須加上這一個，離開 main() 前先清光
    // 在 dll 中所產生的子執行緒
    g_threadPool.waitAllTaskAndClear();

    std::cout << "Hello, from mytest!\n";
}