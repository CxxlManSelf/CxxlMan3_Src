#include <string>
#include <sstream>

#include <sysdef.hpp>

extern "C"
{
    // 加法運算函數，將兩數相加的結果以 string 回傳
    std::string CXXL_DLLEXPORT add(float a, float b)
    {
        std::stringstream ss;
        ss << a << " + " << b << " = " << a + b;
        return ss.str();
    }

    // 減法運算函數，將兩數相加的結果以 string 回傳
    std::string CXXL_DLLEXPORT sub(float a, float b)
    {
        std::stringstream ss;
        ss << a << " - " << b << " = " << a - b;
        return ss.str();
    }

    // 乘法運算函數，將兩數相加的結果以 string 回傳
    std::string CXXL_DLLEXPORT mul(float a, float b)
    {
        std::stringstream ss;
        ss << a << " * " << b << " = " << a * b;
        return ss.str();
    }

    // 除法運算函數，將兩數相加的結果以 string 回傳
    std::string CXXL_DLLEXPORT divide(float a, float b)
    {
        std::stringstream ss;
        ss << a << " / " << b << " = ";
        if (b != 0)
            ss << a / b;
        else
            ss << "Inf";
        return ss.str();
    }
}
