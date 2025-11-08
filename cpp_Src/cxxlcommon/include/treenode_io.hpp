/**************************************************
 * treenode_io.hpp 1.1.7
 * 
 * 針對 TreeNodeBase 延伸類別 NODE<T> 設計的 
 * Stream 匯出匯入功能，採用 UTF-8 編碼
 * 
 * 比如 TreeNode<std::string>* 
 *
 * T 即為要被匯出匯入的資料，若為 std::string，可
 * 直接用內定的轉換方法直接使用，否則可以提供轉
 * 換函數
 * 
 * 支援格式：
 * [節點名稱] = "此節點的文字內容"
 * {
 *     [子節點] = "此節點的文字內容"
 *     {
 *         ...
 *     }
 * }
 * 
 * [] 代表一個節點，不能省略，節點名稱可有可無
 * = 只作為人類較容易閱讀的分隔符，並無作用
 * "" 代表一個節點的文字內容，若無內容可以省略
 * {} 代表一個節點包含的子節點，若無子節點可以省略
 *
 * 字元轉換：
 * 節點名稱中：] → \], \ → \\, '\n' → \n,
 *            '\r' → \r, '\t' → \t
 * 內容中：" → \", \ → \\, '\n' → \n,
 *        '\r' → \r, '\t' → \t
 * 
 * 此外可用 // 或 # 正規註解，事實上在支援格式之外
 * 的文字皆視而不見，可直接用於註解，但註解文字若
 * 含支援格式的識別字，為了避免干擾，請使用 // 或
 *  # 註解起來。
 * 或是為了清楚起見，可考慮一律採用正規註解
 * 
 * Author: CxxlMan
 * Date: 2025-
**************************************************/
#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <optional>



#include "treenode.hpp"

namespace CXXL 
{

// 為了做特別化的模板類別
template <typename T> 
class TreeNode_I;

template <typename T> 
class TreeNode_O;

// 提供對 NODE<DATA> 的 Stream 匯出
// NODE 為 TreeNodeBase 的子類別
// DATA 為 TreeNodeBase 的子類別包裹的資料
// "實作"是針對 TreeNode<std::string> 提供的操作設計
// 所以自訂的 NODE<DATA> 也應該去配合"實作"
template <typename DATA, template <typename> class NODE>
class TreeNode_O<NODE<DATA>>
{
    static inline std::string u8_to_string(const std::u8string &u8) noexcept
    {
        std::string s;
        s.reserve(u8.size());
        for (char8_t c : u8)
            s.push_back(static_cast<char>(c));
        return s;
    }

 
    static inline std::string escapeName(const std::string &s) noexcept
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            switch (c)
            {
            case ']': out += "\\]"; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c); break;
            }
        }
        return out;
    }

    static inline std::string escapeContent(const std::string &s) noexcept
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            switch (c)
            {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c); break;
            }
        }
        return out;
    }

    static inline std::string makeIndent(int depth, size_t indentWidth) noexcept
    {
        return std::string(depth > 0 && indentWidth ? (indentWidth * (depth - 1)) : 0, ' ');
    }

    static inline void collectChildren(const std::shared_ptr<const NODE<DATA>> &node,
        std::vector<std::shared_ptr<const NODE<DATA>>> &out)
    {
        out.clear();
        node->forEachChild([&out](const std::shared_ptr<const NODE<DATA>> &c)
        {
            if (c) out.push_back(c);
        });
    }


public:

    // serialize into stream
    // NODE 須是 TreeNodeBase 的子類別
    // DATA 須是 NODE 這個類別包裹的資料，也是要轉換
    // 成 std::string 的資料，若 DATA 是 std::string 就
    // 比較好辦，內定的動作轉換就好了。否則要指
    // 定轉換函數 dataToString
    // os: 輸出流
    // root: 根節點 
    // dataToString: 轉換函數
    // indentWidth: 每層縮排的寬度
    static void serialize(std::ostream &os, const std::shared_ptr<const NODE<DATA>> &root,
        const std::function<const std::string(const DATA &)> &dataToString =
            [](const DATA &data)
            { return std::string(data); },
        size_t indentWidth = 0)
    {
        if (!root) return;

        struct Frame { std::shared_ptr<const NODE<DATA>> node; int state; int depth; };
        std::vector<Frame> stk;
        stk.push_back({root, 0, 1});

        while (!stk.empty())
        {
            Frame f = stk.back(); stk.pop_back();
            if (!f.node && f.state == 1)
            {
                os << makeIndent(f.depth, indentWidth) << '}' << '\n';
                continue;
            }
            if (!f.node) continue;

            std::string name_s = u8_to_string(f.node->getName());
            std::string escaped_name = escapeName(name_s);
            std::string data_s = dataToString(f.node->getData());
            std::string escaped_data = escapeContent(data_s);

            std::string indent = makeIndent(f.depth, indentWidth);
            if (escaped_data.empty())
                os << indent << '[' << escaped_name << ']' << '\n';
            else
                os << indent << '[' << escaped_name << "] = \"" << escaped_data << "\"" << '\n';

            std::vector<std::shared_ptr<const NODE<DATA>>> children;
            collectChildren(f.node, children);
            if (!children.empty())
            {
                os << indent << '{' << '\n';
                stk.push_back({nullptr, 1, f.depth});
                for (auto it = children.rbegin(); it != children.rend(); ++it)
                    stk.push_back({*it, 0, f.depth + 1});
            }
        }
    }


};

// 提供對 NODE<DATA> 的 Stream 匯入
// NODE 為 TreeNodeBase 的子類別
// DATA 為 TreeNodeBase 的子類別包裹的資料
// "實作"是針對 TreeNode<std::string> 提供的操作設計
// 所以自訂的 NODE<DATA> 也應該去配合"實作"
template <typename DATA, template <typename> class NODE>
class TreeNode_I<NODE<DATA>>
{
public:
    // stream-based parsing that reports parse errors (line/column)
    struct ParseError { size_t line; size_t column; std::string message; };
    
private:    

    const std::function<DATA(const std::string &)> m_stringToData;
    std::optional<ParseError>* m_outError;
    std::istream &m_is;

    size_t m_line = 1;
    size_t m_col = 1;

    std::u8string string_to_u8(const std::string &s) noexcept
    {
        std::u8string u;
        u.reserve(s.size());
        for (unsigned char c : s)
            u.push_back(static_cast<char8_t>(c));
        return u;

    }

    std::string unescape(const std::string &s)
    {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];
            if (c == '\\' && i + 1 < s.size())
            {
                char n = s[i + 1];
                switch (n)
                {
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case '\\': out.push_back('\\'); break;
                case '"': out.push_back('"'); break;
                case ']': out.push_back(']'); break;
                default: out.push_back(n); break;
                }
                ++i;
            }
            else
            {
                out.push_back(c);
            }
        }
        return out;
    }

    std::shared_ptr<NODE<DATA>> 
    setError(size_t line, size_t col, const std::string &msg) noexcept
    {
        if (m_outError)
            *m_outError = ParseError{line, col, msg};
        return nullptr;
    }

    int getChar()
    {
        int ci = m_is.get();
        if (ci == std::char_traits<char>::eof()) return ci;
        char c = static_cast<char>(ci);
        if (c == '\n') 
        { 
            ++m_line; 
            m_col = 1; 
        }
        else 
            ++m_col;

        return ci;
    };

    // skip whitespace and comments, leave stream positioned at next meaningful char
    void skipWSAndComments() 
    {
        while (true)
        {
            int p = m_is.peek();
            if (p == std::char_traits<char>::eof()) return;
            
            char c = static_cast<char>(p);
            if (std::isspace(static_cast<unsigned char>(c))) 
            { 
                getChar();
                continue; 
            }

            if (c == '/')
            {
                size_t old_line = m_line;
                size_t old_col = m_col;

                getChar(); // consume '/'
                int p2 = m_is.peek();
                if (p2 != std::char_traits<char>::eof() && static_cast<char>(p2) == '/')
                {
                    getChar(); // consume second '/'
                    // consume until end of line
                    int g;
                    while ((g = getChar()) != std::char_traits<char>::eof()) { if (static_cast<char>(g) == '\n') break; }
                    continue;
                }
                else
                {
                    // it's not a comment start; put back the first '/'
                    // unfortunately istream has no unget of peek; but we've consumed one '/'
                    // we can use putback to restore
                    m_is.putback('/');
                    // adjust col back (we consumed '/'), so decrement col
                    m_col = old_col; m_line = old_line;
                    return;
                }
            }
            if (c == '#')
            {
                getChar(); // consume '#'
                int g;
                while ((g = getChar()) != std::char_traits<char>::eof()) { if (static_cast<char>(g) == '\n') break; }
                continue;
            }
            break;
        }
    };

    std::pair<bool,std::string> parseNameStream()
    {
        // expects '[' at current position
        int g = getChar();
        if (g == std::char_traits<char>::eof())
            return {false, std::string()};

        char ch = static_cast<char>(g);
        if (ch != '[')
            return {false, std::string()};

        std::string name_esc;
        bool escaped = false;
        while (true)
        {
            int gc = getChar();
            if (gc == std::char_traits<char>::eof()) { return {false, std::string()}; }

            char c2 = static_cast<char>(gc);

            if (escaped)
            {
                // 前一個字符是 '\'，當前字符無論是什麼都照單全收
                name_esc.push_back(c2);
                escaped = false;
            }
            else if (c2 == '\\')
            {
                // 遇到 '\'，標記轉義狀態，並保留 '\' 字符
                name_esc.push_back(c2);
                escaped = true;
            }
            else if (c2 == ']')
            {
                // 未轉義的 ']'，結束解析
                break;
            }
            else
            {
                // 普通字符
                name_esc.push_back(c2);
            }
        }

        return {true, unescape(name_esc)};
    }

    std::pair<bool,std::string> parseQuotedStream()
    {
        // expects '"' at current position
        int g = getChar();
        if (g == std::char_traits<char>::eof())
            return {false, std::string()};

        char ch = static_cast<char>(g);
        if (ch != '"')
            return {false, std::string()};

        std::string content_esc;
        bool escaped = false;
        while (true)
        {
            int gc = getChar();
            if (gc == std::char_traits<char>::eof())
                return {false, std::string()};

            char c2 = static_cast<char>(gc);

            if (escaped)
            {
                // 前一個字符是 '\'，當前字符無論是什麼都照單全收
                content_esc.push_back(c2);
                escaped = false;
            }
            else if (c2 == '\\')
            {
                // 遇到 '\'，標記轉義狀態，並保留 '\' 字符
                content_esc.push_back(c2);
                escaped = true;
            }
            else if (c2 == '"')
            {
                // 未轉義的 '"'，結束解析
                break;
            }
            else
            {
                // 普通字符
                content_esc.push_back(c2);
            }
        }

        return {true, unescape(content_esc)};
    };

    std::shared_ptr<NODE<DATA>> deserialize()
    {
        std::shared_ptr<NODE<DATA>> root = nullptr;
        std::vector<std::shared_ptr<NODE<DATA>>> parentStack;
        std::shared_ptr<NODE<DATA>> lastCreated = nullptr;

        while (true)
        {
            skipWSAndComments();
            int p = m_is.peek();
            if (p == std::char_traits<char>::eof()) break;
            char c = static_cast<char>(p);

            if (c == '{')
            {
                getChar();
                if (lastCreated)
                {
                    parentStack.push_back(lastCreated);
                    lastCreated = nullptr;
                }
                continue;
            }
            if (c == '}')
            {
                getChar();
                if (!parentStack.empty()) parentStack.pop_back();
                lastCreated = nullptr;
                continue;
            }

            if (c == '[')
            {
                // record current position for error reporting
                size_t startLine = m_line, startCol = m_col;
                auto pr = parseNameStream();
                if (!pr.first) return setError(startLine, startCol, "unterminated or invalid node name");
                std::string name = pr.second;

                // after name, skip whitespace/comments and check for '"' (only '"' indicates data)
                skipWSAndComments();

                std::string content;
                // check for quoted content
                // 跳過 [name] 和 "content" 之間無關字元
                // 以及取得 "content"
                // "content" 也可以不存在
                while(true)
                {
                    p = m_is.peek();

                    if (p == std::char_traits<char>::eof()) break;
                    c = static_cast<char>(p);
                    if (c == '"') 
                    {
                        size_t qLine = m_line, qCol = m_col;
                        auto pc = parseQuotedStream();
                        if (!pc.first) return setError(qLine, qCol, "unterminated quoted content");

                        content = pc.second;

                        break;
                    }
                    else if( c == '{' || 
                        c == '}' || 
                        c == '['
                        )
                    {
                        break;
                    }
                    getChar();
                }

                DATA data = m_stringToData(content);
                if (parentStack.empty())
                {
                    root = NODE<DATA>::createRoot(string_to_u8(name));
                    if (!root) return setError(startLine, startCol, "failed to create root node");
                    root->setData(std::move(data));
                    lastCreated = root;
                }
                else
                {
                    auto parent = parentStack.back();
                    auto child = parent->addBackChild(string_to_u8(name));
                    if (!child) return setError(startLine, startCol, "failed to create child node");
                    child->setData(std::move(data));
                    lastCreated = child;
                }

                continue;
            }

            // unknown char/token -> consume and continue
            getChar();
        }


        if (!parentStack.empty())
        {
            // unbalanced '{'
            return setError(m_line, m_col, "unterminated '{' (missing closing '}')");
        }

        if (!root)
        {
            // no root found
            return setError(0, 0, "no root node found");
        }


        if (m_outError) *m_outError = std::nullopt;
        return root;
    }



    // Constructor
    TreeNode_I(const std::function<DATA(const std::string &)> &stringToData,
        std::optional<ParseError> *outError,
        std::istream &is)
        : m_stringToData(stringToData), m_outError(outError), m_is(is)
    {}




public:


    // deserialize from stream
    // NODE 須是 TreeNodeBase 的子類別
    // DATA 須是 NODE 這個類別包裹的資料，也是要轉換
    // 成 std::string 的資料，若 DATA 是 std::string 就
    // 比較好辦，內定的動作轉換就好了。否則要指
    // 定轉換函數 stringToData
    // is: 輸入流
    // stringToData: 轉換函數    
    [[nodiscard]] static std::shared_ptr<NODE<DATA>> deserialize(std::istream &is,
        const std::function<DATA(const std::string &)> &stringToData =
            [](const std::string &s) { return DATA(s); })
    {
        // Backward-compatible wrapper: call stream-based parser and ignore error
        std::optional<ParseError> err;
        return deserialize(is, stringToData, &err);
    }


    // outError: 輸出錯誤，若 is 有錯誤可從中知道錯誤位置(line/column)，
    // 以及錯誤原因(message):
    // unterminated or invalid node name: 沒有結束中括號 ] 或無效的節點名稱
    // unterminated quoted content: 沒有結束引號 "
    // failed to create root node: 根節點名稱不能用
    // failed to create child node: 子節點名稱不能用
    // unterminated '{' (missing closing '}'): 資料來源已無資料，但 TreeNode 未建構完成
    // no root node found: 不是 TreeNode 的 serialize 資料
    [[nodiscard]] static std::shared_ptr<NODE<DATA>> deserialize(std::istream &is,
        const std::function<DATA(const std::string &)> &stringToData,
        std::optional<ParseError> *outError)
    {
        TreeNode_I<NODE<DATA>> parser(stringToData, outError, is);

        return parser.deserialize();
    }


};



} // namespace CXXL
