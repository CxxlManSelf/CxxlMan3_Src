#include <iostream>
#include <sstream>
#include <string>
#include <treenode_io.hpp>

using namespace CXXL;

// 測試案例結構
struct TestCase {
    std::string name;
    std::string input;
    bool shouldSucceed;
};

// 顏色輸出輔助函數
void printSuccess(const std::string& msg) {
    std::cout << "\033[32m[SUCCESS]\033[0m " << msg << "\n";
}

void printError(const std::string& msg) {
    std::cout << "\033[31m[ERROR]\033[0m " << msg << "\n";
}

void printInfo(const std::string& msg) {
    std::cout << "\033[36m[INFO]\033[0m " << msg << "\n";
}

void printTestCase(const std::string& name) {
    std::cout << "\n\033[33m=== Test Case: " << name << " ===\033[0m\n";
}

int main(int, char**) {
    std::cout << "TreeNode_I deserialize() 錯誤偵測範例\n";
    std::cout << "========================================\n";

    // 定義測試案例
    std::vector<TestCase> testCases = {
        // 正常案例
        {
            "正常的樹狀結構",
            "[root] = \"Root Data\"\n"
            "{\n"
            "    [child1] = \"Child 1 Data\"\n"
            "    [child2] = \"Child 2 Data\"\n"
            "    {\n"
            "        [grandchild] = \"Grandchild Data\"\n"
            "    }\n"
            "}\n",
            true
        },

        // 錯誤案例 1: 未結束的節點名稱
        {
            "未結束的節點名稱 (缺少 ])",
            "[root = \"Root Data\"\n",
            false
        },

        // 錯誤案例 2: 未結束的引號內容
        {
            "未結束的引號內容 (缺少 \")",
            "[root] = \"Root Data\n"
            "[child] = \"test\"\n",
            false
        },

        // 錯誤案例 3: 未結束的大括號
        {
            "未結束的大括號 (缺少 })",
            "[root] = \"Root Data\"\n"
            "{\n"
            "    [child] = \"Child Data\"\n",
            false
        },

        // 錯誤案例 4: 空輸入
        {
            "空輸入 (無根節點)",
            "",
            false
        },

        // 錯誤案例 5: 重複的節點名稱
        {
            "重複的節點名稱",
            "[root] = \"Root Data\"\n"
            "{\n"
            "    [child] = \"Child 1\"\n"
            "    [child] = \"Child 2\"\n"
            "}\n",
            false
        },

        // 正常案例: 帶註解
        {
            "帶有註解的輸入",
            "// 這是註解\n"
            "[root] = \"Root Data\"\n"
            "{\n"
            "    # 另一種註解\n"
            "    [child] = \"Child Data\"\n"
            "}\n",
            true
        },

        // 正常案例: 轉義字符
        {
            "帶有轉義字符",
            "[root\\]test] = \"Data with \\\"quotes\\\" and \\n newline\"\n",
            true
        }
    };

    int passedTests = 0;
    int totalTests = testCases.size();

    // 執行測試案例
    for (const auto& testCase : testCases) {
        printTestCase(testCase.name);

        std::cout << "輸入內容:\n";
        std::cout << "---\n" << testCase.input << "---\n\n";

        // 建立輸入串流
        std::istringstream iss(testCase.input);

        // 錯誤資訊容器
        std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;

        // 反序列化
        auto root = TreeNode_I<TreeNode<std::string>>::deserialize(
            iss,
            [](const std::string& s) { return s; },
            &error
        );

        // 檢查結果
        if (root) {
            if (testCase.shouldSucceed) {
                printSuccess("反序列化成功");
                passedTests++;

                // 顯示樹狀結構
                std::cout << "\n反序列化的樹狀結構:\n";
                std::ostringstream oss;
                TreeNode_O<TreeNode<std::string>>::serialize(
                    oss,
                    root,
                    [](const std::string& data) { return data; },
                    4  // 4 spaces indent
                );
                std::cout << oss.str();
            } else {
                printError("預期失敗但卻成功了!");
            }
        } else {
            if (!testCase.shouldSucceed) {
                printSuccess("正確偵測到錯誤");
                passedTests++;
            } else {
                printError("預期成功但卻失敗了!");
            }

            // 顯示錯誤資訊
            if (error) {
                std::cout << "\n錯誤詳情:\n";
                std::cout << "  位置: 第 " << error->line << " 行, 第 "
                         << error->column << " 列\n";
                std::cout << "  訊息: " << error->message << "\n";
            } else {
                std::cout << "\n沒有提供詳細的錯誤資訊\n";
            }
        }
    }

    // 顯示測試結果摘要
    std::cout << "\n========================================\n";
    std::cout << "測試結果: " << passedTests << "/" << totalTests << " 通過\n";

    if (passedTests == totalTests) {
        printSuccess("所有測試通過!");
        return 0;
    } else {
        printError("部分測試失敗!");
        return 1;
    }
}
