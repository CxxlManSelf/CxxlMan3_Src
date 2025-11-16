/***********************************************************
 * database_io.hpp
 *
 * 提供 PathNode<shared_ptr<IDataBase>> 的序列化/反序列化功能
 * 使用 TreeNode_io 作為基礎
 *
 * Author: CxxlMan
 * Date: 2025-
 ************************************************************/
#ifndef __DATABASE_IO_HPP_CxxlMan3
#define __DATABASE_IO_HPP_CxxlMan3

#include "pathnode.hpp"
#include "treenode_io.hpp"
#include "database_types.hpp"
#include <fstream>

namespace CXXL
{

// 資料型別工廠
class DataFactory
{
public:
    // 從序列化字串創建對應的資料物件
    static std::shared_ptr<IDataBase> createFromString(const std::string& str)
    {
        if (str.empty())
            return std::make_shared<EmptyData>();

        // 找到類型名稱（在第一個 '|' 之前）
        size_t pos = str.find('|');
        std::string typeName;
        if (pos != std::string::npos)
        {
            typeName = str.substr(0, pos);
            // 移除前後空白
            typeName.erase(0, typeName.find_first_not_of(" \t\r\n"));
            typeName.erase(typeName.find_last_not_of(" \t\r\n") + 1);
        }
        else
        {
            return std::make_shared<EmptyData>();
        }

        std::shared_ptr<IDataBase> data;

        // 根據類型名稱創建對應的物件
        if (typeName == "PersonData")
            data = std::make_shared<PersonData>();
        else if (typeName == "ProductData")
            data = std::make_shared<ProductData>();
        else if (typeName == "OrderData")
            data = std::make_shared<OrderData>();
        else if (typeName == "OrderItemData")
            data = std::make_shared<OrderItemData>();
        else if (typeName == "StringData")
            data = std::make_shared<StringData>();
        else if (typeName == "DateData")
            data = std::make_shared<DateData>();
        else if (typeName == "FloatData")
            data = std::make_shared<FloatData>();
        else if (typeName == "EmptyData")
            data = std::make_shared<EmptyData>();
        else
            return std::make_shared<EmptyData>(); // 未知類型

        // 反序列化
        if (!data->deserialize(str))
            return std::make_shared<EmptyData>();

        return data;
    }
};

// 資料庫節點類型
using DBNode = PathNode<std::shared_ptr<IDataBase>>;

// 資料庫 IO 類
class DatabaseIO
{
public:
    // 儲存資料庫到檔案
    static bool save(const std::string& filename, const std::shared_ptr<DBNode>& root)
    {
        std::ofstream ofs(filename);
        if (!ofs)
            return false;

        // 使用 TreeNode_O 進行序列化
        TreeNode_O<DBNode>::serialize(
            ofs,
            root,
            [](const std::shared_ptr<IDataBase>& data) -> std::string
            {
                if (!data)
                    return "";
                return data->serialize();
            },
            4  // 縮排寬度
        );

        return true;
    }

    // 從檔案載入資料庫
    static std::shared_ptr<DBNode> load(const std::string& filename)
    {
        std::ifstream ifs(filename);
        if (!ifs)
            return nullptr;

        std::optional<TreeNode_I<DBNode>::ParseError> error;

        auto root = TreeNode_I<DBNode>::deserialize(
            ifs,
            [](const std::string& str) -> std::shared_ptr<IDataBase>
            {
                return DataFactory::createFromString(str);
            },
            &error
        );

        if (error.has_value())
        {
            std::cerr << "Parse error at line " << error->line
                      << ", column " << error->column
                      << ": " << error->message << std::endl;
            return nullptr;
        }

        return root;
    }
};

} // namespace CXXL

#endif // __DATABASE_IO_HPP_CxxlMan3
