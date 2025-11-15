#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

#include <treenode.hpp>
#include <treenode_io.hpp>

using namespace CXXL;

// 測試計數器
int g_testCount = 0;
int g_passCount = 0;
int g_failCount = 0;

// 輔助函數：將 u8string 轉換為 string（用於顯示）
inline std::string u8str_to_str(const std::u8string& u8str) {
    std::string result;
    result.reserve(u8str.size());
    for (char8_t ch : u8str) {
        result.push_back(static_cast<char>(ch));
    }
    return result;
}

// 測試巨集
#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        std::cout << "\n=== 測試 " << #name << " ===" << std::endl; \
        g_testCount++; \
        try { \
            test_##name(); \
            g_passCount++; \
            std::cout << "✓ 通過" << std::endl; \
        } catch (const std::exception& e) { \
            g_failCount++; \
            std::cout << "✗ 失敗: " << e.what() << std::endl; \
        } \
    } \
    void test_##name()

#define ASSERT(condition, message) \
    if (!(condition)) { \
        throw std::runtime_error(std::string("斷言失敗: ") + message); \
    }

// 一般版本的 ASSERT_EQ
#define ASSERT_EQ(a, b, message) \
    if ((a) != (b)) { \
        std::ostringstream oss; \
        oss << "斷言失敗: " << message; \
        throw std::runtime_error(oss.str()); \
    }

// u8string 專用版本
#define ASSERT_EQ_U8(a, b, message) \
    if ((a) != (b)) { \
        throw std::runtime_error(std::string("斷言失敗: ") + message); \
    }

// ========== 1. 基本序列化測試 ==========

TEST(serialize_simple_root) {
    // 測試：序列化單一根節點
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData("根節點資料");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("[root]") != std::string::npos, "應包含節點名稱");
    ASSERT(result.find("根節點資料") != std::string::npos, "應包含節點資料");
}

TEST(serialize_with_children) {
    // 測試：序列化包含子節點的樹
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData("根");

    auto child1 = root->addChild(u8"child1");
    child1->setData("子節點1");

    auto child2 = root->addChild(u8"child2");
    child2->setData("子節點2");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("[root]") != std::string::npos, "應包含根節點");
    ASSERT(result.find("[child1]") != std::string::npos, "應包含子節點1");
    ASSERT(result.find("[child2]") != std::string::npos, "應包含子節點2");
    ASSERT(result.find("{") != std::string::npos, "應包含開始大括號");
    ASSERT(result.find("}") != std::string::npos, "應包含結束大括號");
}

TEST(serialize_nested_tree) {
    // 測試：序列化多層嵌套的樹
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData("層級0");

    auto level1 = root->addChild(u8"level1");
    level1->setData("層級1");

    auto level2 = level1->addChild(u8"level2");
    level2->setData("層級2");

    auto level3 = level2->addChild(u8"level3");
    level3->setData("層級3");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root,
        [](const std::string &data) { return data; }, 2); // 縮排寬度為2

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("[level3]") != std::string::npos, "應包含第3層節點");
}

TEST(serialize_empty_data) {
    // 測試：序列化空資料的節點
    auto root = TreeNode<std::string>::createRoot(u8"empty");
    root->setData("");  // 空資料

    auto child = root->addChild(u8"child");
    child->setData("");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("[empty]") != std::string::npos, "應包含空資料節點");
}

TEST(serialize_with_indent) {
    // 測試：序列化帶縮排的輸出
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData("資料");

    auto child = root->addChild(u8"child");
    child->setData("子資料");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root,
        [](const std::string &data) { return data; }, 4); // 縮排4格

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    // 檢查是否有正確的縮排
    ASSERT(result.find("    ") != std::string::npos, "應包含縮排空格");
}

// ========== 2. 跳脫字元測試 ==========

TEST(escape_special_chars_in_name) {
    // 測試：節點名稱中的特殊字元跳脫
    auto root = TreeNode<std::string>::createRoot(u8"node]with]brackets");
    root->setData("資料");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("\\]") != std::string::npos, "應跳脫 ] 字元");
}

TEST(escape_special_chars_in_data) {
    // 測試：資料中的特殊字元跳脫
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData("資料包含\"引號\"和\\反斜線\\");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("\\\"") != std::string::npos, "應跳脫引號");
    ASSERT(result.find("\\\\") != std::string::npos, "應跳脫反斜線");
}

TEST(escape_newline_tab_chars) {
    // 測試：換行符和Tab字元的跳脫
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData("第一行\n第二行\t有Tab");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::string result = oss.str();
    std::cout << "序列化結果:\n" << result << std::endl;

    ASSERT(result.find("\\n") != std::string::npos, "應跳脫換行符");
    ASSERT(result.find("\\t") != std::string::npos, "應跳脫Tab字元");
}

// ========== 3. 基本反序列化測試 ==========

TEST(deserialize_simple_root) {
    // 測試：反序列化單一根節點
    std::string input = "[root] = \"根節點資料\"\n";
    std::istringstream iss(input);

    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    ASSERT_EQ_U8(root->getName(), u8"root", "節點名稱應為 root");
    ASSERT_EQ(root->getData(), "根節點資料", "節點資料應正確");
}

TEST(deserialize_with_children) {
    // 測試：反序列化包含子節點的樹
    std::string input =
        "[root] = \"根\"\n"
        "{\n"
        "  [child1] = \"子節點1\"\n"
        "  [child2] = \"子節點2\"\n"
        "}\n";

    std::istringstream iss(input);
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    ASSERT_EQ(root->childCount(), 2, "應有2個子節點");

    auto child1 = root->findChildByName(u8"child1");
    ASSERT(child1 != nullptr, "應找到 child1");
    ASSERT_EQ(child1->getData(), "子節點1", "child1 資料應正確");

    auto child2 = root->findChildByName(u8"child2");
    ASSERT(child2 != nullptr, "應找到 child2");
    ASSERT_EQ(child2->getData(), "子節點2", "child2 資料應正確");
}

TEST(deserialize_nested_tree) {
    // 測試：反序列化多層嵌套的樹
    std::string input =
        "[root] = \"層級0\"\n"
        "{\n"
        "  [level1] = \"層級1\"\n"
        "  {\n"
        "    [level2] = \"層級2\"\n"
        "    {\n"
        "      [level3] = \"層級3\"\n"
        "    }\n"
        "  }\n"
        "}\n";

    std::istringstream iss(input);
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");

    auto level1 = root->findChildByName(u8"level1");
    ASSERT(level1 != nullptr, "應找到 level1");

    auto level2 = level1->findChildByName(u8"level2");
    ASSERT(level2 != nullptr, "應找到 level2");

    auto level3 = level2->findChildByName(u8"level3");
    ASSERT(level3 != nullptr, "應找到 level3");
    ASSERT_EQ(level3->getData(), "層級3", "level3 資料應正確");
}

TEST(deserialize_empty_data) {
    // 測試：反序列化空資料
    std::string input =
        "[empty]\n"
        "{\n"
        "  [child]\n"
        "}\n";

    std::istringstream iss(input);
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    ASSERT_EQ(root->getData(), "", "空資料應為空字串");

    auto child = root->findChildByName(u8"child");
    ASSERT(child != nullptr, "應找到子節點");
    ASSERT_EQ(child->getData(), "", "子節點資料應為空");
}

TEST(deserialize_unescape_chars) {
    // 測試：反序列化跳脫字元
    std::string input = "[root] = \"第一行\\n第二行\\t有Tab\\\"引號\\\"反斜線\\\\\"\n";
    std::istringstream iss(input);

    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    std::string data = root->getData();

    ASSERT(data.find('\n') != std::string::npos, "應包含實際換行符");
    ASSERT(data.find('\t') != std::string::npos, "應包含實際Tab字元");
    ASSERT(data.find('"') != std::string::npos, "應包含實際引號");
    ASSERT(data.find('\\') != std::string::npos, "應包含實際反斜線");
}

// ========== 4. 註解處理測試 ==========

TEST(deserialize_with_line_comments) {
    // 測試：處理 // 單行註解
    std::string input =
        "// 這是註解\n"
        "[root] = \"資料\"\n"
        "// 另一個註解\n"
        "{\n"
        "  // 子節點註解\n"
        "  [child] = \"子資料\"\n"
        "}\n";

    std::istringstream iss(input);
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    ASSERT_EQ(root->getData(), "資料", "註解不應影響解析");
    ASSERT_EQ(root->childCount(), 1, "應有1個子節點");
}

TEST(deserialize_with_hash_comments) {
    // 測試：處理 # 單行註解
    std::string input =
        "# 這是井號註解\n"
        "[root] = \"資料\"\n"
        "{\n"
        "  # 子節點註解\n"
        "  [child] = \"子資料\"\n"
        "}\n";

    std::istringstream iss(input);
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    ASSERT_EQ(root->childCount(), 1, "應有1個子節點");
}

TEST(deserialize_mixed_comments) {
    // 測試：混合使用不同註解
    std::string input =
        "// 雙斜線註解\n"
        "# 井號註解\n"
        "[root] = \"資料\"\n"
        "{\n"
        "  // 註解1\n"
        "  [child1]\n"
        "  # 註解2\n"
        "  [child2]\n"
        "}\n";

    std::istringstream iss(input);
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(root != nullptr, "應成功建立根節點");
    ASSERT_EQ(root->childCount(), 2, "應有2個子節點");
}

// ========== 5. 錯誤處理測試 ==========

TEST(deserialize_error_unterminated_name) {
    // 測試：未結束的節點名稱
    std::string input = "[root_without_closing_bracket = \"data\"\n";
    std::istringstream iss(input);

    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss,
        [](const std::string &s) { return s; }, &error);

    ASSERT(root == nullptr, "應返回 nullptr");
    ASSERT(error.has_value(), "應有錯誤資訊");
    std::cout << "錯誤: " << error->message << " (第 " << error->line
              << " 行, 第 " << error->column << " 列)" << std::endl;
}

TEST(deserialize_error_unterminated_quote) {
    // 測試：未結束的引號
    std::string input = "[root] = \"unterminated string\n";
    std::istringstream iss(input);

    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss,
        [](const std::string &s) { return s; }, &error);

    ASSERT(root == nullptr, "應返回 nullptr");
    ASSERT(error.has_value(), "應有錯誤資訊");
    std::cout << "錯誤: " << error->message << " (第 " << error->line
              << " 行, 第 " << error->column << " 列)" << std::endl;
}

TEST(deserialize_error_unbalanced_braces) {
    // 測試：不匹配的大括號
    std::string input =
        "[root]\n"
        "{\n"
        "  [child]\n"
        "  {\n"
        "    [grandchild]\n"
        "  }\n";  // 缺少一個 }

    std::istringstream iss(input);

    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss,
        [](const std::string &s) { return s; }, &error);

    ASSERT(root == nullptr, "應返回 nullptr");
    ASSERT(error.has_value(), "應有錯誤資訊");
    std::cout << "錯誤: " << error->message << std::endl;
}

TEST(deserialize_error_no_root) {
    // 測試：沒有根節點
    std::string input = "// 只有註解\n# 沒有任何節點\n";
    std::istringstream iss(input);

    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(iss,
        [](const std::string &s) { return s; }, &error);

    ASSERT(root == nullptr, "應返回 nullptr");
    ASSERT(error.has_value(), "應有錯誤資訊");
    std::cout << "錯誤: " << error->message << std::endl;
}

// ========== 6. 往返測試（序列化後再反序列化）==========

TEST(roundtrip_simple_tree) {
    // 測試：簡單樹的往返
    auto original = TreeNode<std::string>::createRoot(u8"root");
    original->setData("原始資料");

    auto child1 = original->addChild(u8"child1");
    child1->setData("子資料1");

    auto child2 = original->addChild(u8"child2");
    child2->setData("子資料2");

    // 序列化
    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, original);
    std::string serialized = oss.str();

    std::cout << "序列化結果:\n" << serialized << std::endl;

    // 反序列化
    std::istringstream iss(serialized);
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功反序列化");
    ASSERT_EQ_U8(restored->getName(), original->getName(), "根節點名稱應相同");
    ASSERT_EQ(restored->getData(), original->getData(), "根節點資料應相同");
    ASSERT_EQ(restored->childCount(), original->childCount(), "子節點數量應相同");

    auto restoredChild1 = restored->findChildByName(u8"child1");
    ASSERT(restoredChild1 != nullptr, "應找到 child1");
    ASSERT_EQ(restoredChild1->getData(), child1->getData(), "child1 資料應相同");
}

TEST(roundtrip_complex_tree) {
    // 測試：複雜樹的往返
    auto original = TreeNode<std::string>::createRoot(u8"文件");
    original->setData("這是\"文件\"內容\\包含\\特殊\n字元\t和Tab");

    auto section1 = original->addChild(u8"章節]1");
    section1->setData("第一章\n內容");

    auto subsection1 = section1->addChild(u8"小節1.1");
    subsection1->setData("細節\"資料\"");

    auto subsection2 = section1->addChild(u8"小節1.2");
    subsection2->setData("");

    auto section2 = original->addChild(u8"章節2");
    section2->setData("第二章");

    // 序列化
    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, original,
        [](const std::string &data) { return data; }, 2);
    std::string serialized = oss.str();

    std::cout << "序列化結果:\n" << serialized << std::endl;

    // 反序列化
    std::istringstream iss(serialized);
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功反序列化");
    ASSERT_EQ(restored->getData(), original->getData(), "根節點資料應相同");
    ASSERT_EQ(restored->childCount(), 2, "應有2個子節點");

    auto restoredSection1 = restored->findChildByName(u8"章節]1");
    ASSERT(restoredSection1 != nullptr, "應找到章節]1");
    ASSERT_EQ(restoredSection1->childCount(), 2, "章節1應有2個小節");
}

// ========== 7. 自訂資料型別轉換測試 ==========

TEST(custom_data_type_int) {
    // 測試：使用整數型別
    auto root = TreeNode<int>::createRoot(u8"numbers");
    root->setData(42);

    auto child1 = root->addChild(u8"num1");
    child1->setData(100);

    auto child2 = root->addChild(u8"num2");
    child2->setData(-50);

    // 序列化（需要提供 int to string 轉換）
    std::ostringstream oss;
    TreeNode_O<TreeNode<int>>::serialize(oss, root,
        [](const int &data) { return std::to_string(data); });

    std::string serialized = oss.str();
    std::cout << "序列化結果:\n" << serialized << std::endl;

    // 反序列化（需要提供 string to int 轉換）
    std::istringstream iss(serialized);
    auto restored = TreeNode_I<TreeNode<int>>::deserialize(iss,
        [](const std::string &s) { return std::stoi(s); });

    ASSERT(restored != nullptr, "應成功反序列化");
    ASSERT_EQ(restored->getData(), 42, "根節點資料應為 42");

    auto restoredChild1 = restored->findChildByName(u8"num1");
    ASSERT(restoredChild1 != nullptr, "應找到 num1");
    ASSERT_EQ(restoredChild1->getData(), 100, "num1 資料應為 100");

    auto restoredChild2 = restored->findChildByName(u8"num2");
    ASSERT(restoredChild2 != nullptr, "應找到 num2");
    ASSERT_EQ(restoredChild2->getData(), -50, "num2 資料應為 -50");
}

// ========== 8. 邊界情況測試 ==========

TEST(edge_case_very_long_name) {
    // 測試：非常長的節點名稱
    std::string longName(1000, 'a');
    auto root = TreeNode<std::string>::createRoot(
        std::u8string(longName.begin(), longName.end()));
    root->setData("資料");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::istringstream iss(oss.str());
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功處理長名稱");
    ASSERT_EQ(restored->getName().size(), 1000, "名稱長度應正確");
}

TEST(edge_case_very_long_data) {
    // 測試：非常長的資料
    std::string longData(10000, 'x');
    auto root = TreeNode<std::string>::createRoot(u8"root");
    root->setData(longData);

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::istringstream iss(oss.str());
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功處理長資料");
    ASSERT_EQ(restored->getData().size(), 10000, "資料長度應正確");
}

TEST(edge_case_deep_nesting) {
    // 測試：深層嵌套
    auto root = TreeNode<std::string>::createRoot(u8"level0");
    auto current = root;

    // 建立100層深的樹
    for (int i = 1; i <= 100; ++i) {
        std::string name = "level" + std::to_string(i);
        auto child = current->addChild(
            std::u8string(name.begin(), name.end()));
        child->setData("資料" + std::to_string(i));
        current = child;
    }

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::istringstream iss(oss.str());
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功處理深層嵌套");

    // 驗證深度
    auto check = restored;
    int depth = 0;
    while (check && check->childCount() > 0) {
        check = check->getFirstChild();
        depth++;
    }
    ASSERT_EQ(depth, 100, "深度應為100");
}

TEST(edge_case_many_siblings) {
    // 測試：大量兄弟節點
    auto root = TreeNode<std::string>::createRoot(u8"root");

    // 建立1000個兄弟節點
    for (int i = 0; i < 1000; ++i) {
        std::string name = "child" + std::to_string(i);
        auto child = root->addChild(
            std::u8string(name.begin(), name.end()));
        child->setData("資料" + std::to_string(i));
    }

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::istringstream iss(oss.str());
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功處理大量兄弟節點");
    ASSERT_EQ(restored->childCount(), 1000, "應有1000個子節點");
}

TEST(edge_case_unicode_content) {
    // 測試：Unicode 內容
    auto root = TreeNode<std::string>::createRoot(u8"根節點");
    root->setData("中文內容：你好世界！🌍");

    auto child = root->addChild(u8"子節點");
    child->setData("日本語：こんにちは");

    std::ostringstream oss;
    TreeNode_O<TreeNode<std::string>>::serialize(oss, root);

    std::cout << "Unicode 序列化:\n" << oss.str() << std::endl;

    std::istringstream iss(oss.str());
    auto restored = TreeNode_I<TreeNode<std::string>>::deserialize(iss);

    ASSERT(restored != nullptr, "應成功處理 Unicode");
    ASSERT(restored->getData().find("你好世界") != std::string::npos,
           "應保留中文內容");
}

// ========== 主函數 ==========

int main(int, char**) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║   TreeNode_IO 完整測試程式               ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    // 執行所有測試
    run_test_serialize_simple_root();
    run_test_serialize_with_children();
    run_test_serialize_nested_tree();
    run_test_serialize_empty_data();
    run_test_serialize_with_indent();

    run_test_escape_special_chars_in_name();
    run_test_escape_special_chars_in_data();
    run_test_escape_newline_tab_chars();

    run_test_deserialize_simple_root();
    run_test_deserialize_with_children();
    run_test_deserialize_nested_tree();
    run_test_deserialize_empty_data();
    run_test_deserialize_unescape_chars();

    run_test_deserialize_with_line_comments();
    run_test_deserialize_with_hash_comments();
    run_test_deserialize_mixed_comments();

    run_test_deserialize_error_unterminated_name();
    run_test_deserialize_error_unterminated_quote();
    run_test_deserialize_error_unbalanced_braces();
    run_test_deserialize_error_no_root();

    run_test_roundtrip_simple_tree();
    run_test_roundtrip_complex_tree();

    run_test_custom_data_type_int();

    run_test_edge_case_very_long_name();
    run_test_edge_case_very_long_data();
    run_test_edge_case_deep_nesting();
    run_test_edge_case_many_siblings();
    run_test_edge_case_unicode_content();

    // 輸出測試結果摘要
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║           測試結果摘要                   ║\n";
    std::cout << "╠══════════════════════════════════════════╣\n";
    std::cout << "║  總測試數: " << g_testCount << "                         ║\n";
    std::cout << "║  通過: " << g_passCount << "                             ║\n";
    std::cout << "║  失敗: " << g_failCount << "                              ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    if (g_failCount == 0) {
        std::cout << "\n🎉 所有測試通過！\n" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ 有測試失敗，請檢查上方輸出。\n" << std::endl;
        return 1;
    }
}
