#include <iostream>
#include <sstream>
#include <memory>

#include <treenode.hpp>
#include <treenode_io.hpp>

using namespace CXXL;

// 將 u8string 轉為普通 string 以便輸出
std::string u8str_to_string(const std::u8string& u8str)
{
    std::string result;
    for (char8_t c : u8str)
    {
        result.push_back(static_cast<char>(c));
    }
    return result;
}

// 測試 1: 基本的序列化和反序列化
void testBasicSerializeDeserialize()
{
    std::cout << "========== 測試 1: 基本序列化和反序列化 ==========" << std::endl;

    // 建立一個簡單的樹
    auto root = TreeNode<std::string>::createRoot(u8"根");
    root->setData("根節點的資料");

    auto child1 = root->addBackChild(u8"子節點1");
    child1->setData("我是第一個子節點");

    auto child2 = root->addBackChild(u8"子節點2");
    child2->setData("我是第二個子節點");

    auto grandchild = child1->addBackChild(u8"孫節點");
    grandchild->setData("我是孫節點");

    // 序列化到字符串流
    std::stringstream ss;
    TreeNode_O<TreeNode<std::string>>::serialize(ss, root,
        [](const std::string& s) { return s; }, 2);

    std::cout << "序列化結果：" << std::endl;
    std::cout << ss.str() << std::endl;

    // 反序列化
    ss.seekg(0);
    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root2 = TreeNode_I<TreeNode<std::string>>::deserialize(ss, 
        [](const std::string& s) { return s; }, &error);

    if (root2)
    {
        std::cout << "反序列化成功！" << std::endl;
        std::cout << "根節點名稱: " << u8str_to_string(root2->getName()) << std::endl;
        std::cout << "根節點資料: " << root2->getData() << std::endl;
        std::cout << "子節點數量: " << root2->childCount() << std::endl;
    }
    else
    {
        if (error.has_value())
        {
            std::cout << "反序列化失敗！" << std::endl;
            std::cout << "錯誤位置: 第 " << error->line << " 行，第 " << error->column << " 列" << std::endl;
            std::cout << "錯誤原因: " << error->message << std::endl;
        }
    }

    std::cout << std::endl;
}

// 測試 2: 處理特殊字符和轉義
void testEscapeCharacters()
{
    std::cout << "========== 測試 2: 特殊字符和轉義 ==========" << std::endl;

    auto root = TreeNode<std::string>::createRoot(u8"特殊字符[測試]");
    root->setData("包含\n換行\t製表符和\"引號\"的內容");

    auto child = root->addBackChild(u8"子\\節\\點");
    child->setData("資料中有\\反斜杠");

    // 序列化
    std::stringstream ss;
    TreeNode_O<TreeNode<std::string>>::serialize(ss, root,
        [](const std::string& s) { return s; }, 2);

    std::cout << "含特殊字符的序列化結果：" << std::endl;
    std::cout << ss.str() << std::endl;

    // 反序列化
    ss.seekg(0);
    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root2 = TreeNode_I<TreeNode<std::string>>::deserialize(ss, 
        [](const std::string& s) { return s; }, &error);

    if (root2)
    {
        std::cout << "反序列化成功，驗證特殊字符：" << std::endl;
        std::cout << "根節點名稱: " << u8str_to_string(root2->getName()) << std::endl;
        std::cout << "根節點資料: " << root2->getData() << std::endl;
    }

    std::cout << std::endl;
}

// 測試 3: 複雜的樹結構
void testComplexTree()
{
    std::cout << "========== 測試 3: 複雜的樹結構 ==========" << std::endl;

    auto root = TreeNode<std::string>::createRoot(u8"公司");
    root->setData("ABC公司");

    auto dept1 = root->addBackChild(u8"技術部");
    dept1->setData("負責軟體開發");

    auto team1 = dept1->addBackChild(u8"前端團隊");
    team1->setData("");

    team1->addBackChild(u8"小王");
    team1->addBackChild(u8"小李");
    team1->addBackChild(u8"小張");

    auto team2 = dept1->addBackChild(u8"後端團隊");
    team2->setData("Python & C++");

    team2->addBackChild(u8"老王");
    team2->addBackChild(u8"老李");

    auto dept2 = root->addBackChild(u8"市場部");
    dept2->setData("負責銷售和推廣");

    // 序列化
    std::stringstream ss;
    TreeNode_O<TreeNode<std::string>>::serialize(ss, root,
        [](const std::string& s) { return s; }, 4);

    std::cout << "複雜樹結構的序列化結果：" << std::endl;
    std::cout << ss.str() << std::endl;

    // 反序列化並驗證
    ss.seekg(0);
    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root2 = TreeNode_I<TreeNode<std::string>>::deserialize(ss);

    if (root2)
    {
        std::cout << "反序列化成功！樹的結構已還原。" << std::endl;
        std::cout << "根節點: " << u8str_to_string(root2->getName()) << std::endl;
        std::cout << "第一層子節點數: " << root2->childCount() << std::endl;

        auto tech_dept = root2->findChildByName(u8"技術部");
        if (tech_dept)
        {
            std::cout << "技術部下的子節點數: " << tech_dept->childCount() << std::endl;
        }
    }

    std::cout << std::endl;
}

// 測試 4: 無縮排和有縮排的序列化
void testIndentation()
{
    std::cout << "========== 測試 4: 縮排設定 ==========" << std::endl;

    auto root = TreeNode<std::string>::createRoot(u8"配置");
    root->setData("應用配置");

    auto db = root->addBackChild(u8"數據庫");
    db->setData("localhost");

    auto user = db->addBackChild(u8"用戶名");
    user->setData("admin");

    auto pass = db->addBackChild(u8"密碼");
    pass->setData("password123");

    std::cout << "無縮排序列化：" << std::endl;
    std::stringstream ss1;
    TreeNode_O<TreeNode<std::string>>::serialize(ss1, root,
        [](const std::string& s) { return s; }, 0);
    std::cout << ss1.str() << std::endl;

    std::cout << "有縮排序列化（寬度=3）：" << std::endl;
    std::stringstream ss2;
    TreeNode_O<TreeNode<std::string>>::serialize(ss2, root,
        [](const std::string& s) { return s; }, 3);
    std::cout << ss2.str() << std::endl;
}

// 測試 5: 註解測試
void testWithComments()
{
    std::cout << "========== 測試 5: 含註解的反序列化 ==========" << std::endl;

    std::string input = R"(
# 這是一個配置文件的例子
[設定] = "主配置"
{
    # 數據庫配置
    [DB] = "MySQL"
    {
        [host] = "localhost" // 主機名
        [port] = "3306"      # 默認端口
        [user] = "root"
    }
    // 應用配置
    [應用] = "MyApp"
}
)";

    std::stringstream ss(input);
    std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
    auto root = TreeNode_I<TreeNode<std::string>>::deserialize(ss, 
        [](const std::string& s) { return s; }, &error);

    if (root)
    {
        std::cout << "含註解的資料解析成功！" << std::endl;
        std::cout << "根節點: " << u8str_to_string(root->getName()) 
                  << " = \"" << root->getData() << "\"" << std::endl;
        std::cout << "子節點數: " << root->childCount() << std::endl;

        // 遍歷子節點
        root->forEachChild([](const std::shared_ptr<const TreeNode<std::string>>& child)
        {
            std::cout << "  - " << u8str_to_string(child->getName()) 
                      << " = \"" << child->getData() << "\"" << std::endl;
        });
    }
    else
    {
        if (error.has_value())
        {
            std::cout << "解析失敗！" << std::endl;
            std::cout << "錯誤: " << error->message << std::endl;
        }
    }

    std::cout << std::endl;
}

// 測試 6: 錯誤處理
void testErrorHandling()
{
    std::cout << "========== 測試 6: 錯誤處理 ==========" << std::endl;

    // 測試 1: 未結束的節點名稱
    {
        std::string input = "[未結束的節點\n";
        std::stringstream ss(input);
        std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
        auto root = TreeNode_I<TreeNode<std::string>>::deserialize(ss, 
            [](const std::string& s) { return s; }, &error);

        std::cout << "測試未結束的節點名稱:" << std::endl;
        if (!root && error.has_value())
        {
            std::cout << "  正確捕捉到錯誤: " << error->message << std::endl;
        }
    }

    // 測試 2: 未結束的引號
    {
        std::string input = "[節點] = \"未結束的引號\n";
        std::stringstream ss(input);
        std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
        auto root = TreeNode_I<TreeNode<std::string>>::deserialize(ss, 
            [](const std::string& s) { return s; }, &error);

        std::cout << "測試未結束的引號:" << std::endl;
        if (!root && error.has_value())
        {
            std::cout << "  正確捕捉到錯誤: " << error->message << std::endl;
        }
    }

    // 測試 3: 不匹配的大括號
    {
        std::string input = "[根] { [子] }}\n";
        std::stringstream ss(input);
        std::optional<TreeNode_I<TreeNode<std::string>>::ParseError> error;
        auto root = TreeNode_I<TreeNode<std::string>>::deserialize(ss, 
            [](const std::string& s) { return s; }, &error);

        std::cout << "測試完成（多餘括號可被容忍）" << std::endl;
        if (root)
        {
            std::cout << "  成功解析根節點" << std::endl;
        }
    }

    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "===== treenode_io 測試程式 =====" << std::endl << std::endl;

    try
    {
        testBasicSerializeDeserialize();
        testEscapeCharacters();
        testComplexTree();
        testIndentation();
        testWithComments();
        testErrorHandling();

        std::cout << "===== 所有測試完成 =====" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "發生異常: " << e.what() << std::endl;
        return 1;
    }

    // 等待非同步刪除器完成
    AsyncNodeDeletor::wait();


    return 0;
}
