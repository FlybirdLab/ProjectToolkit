#include "ASTNodes.hpp"

#include "toolkit/types/StringBuilder.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{

    static const char *s_NodeType[] = {
        "Unknwon"
        "TransUnit"
        "TypeDecl"
        "Namespace"
        "Field"
        "Function"
        "EnumValue"
        "Enum"};
    TkStringView ASTNode::toString(NodeType t)
    {
        return s_NodeType[(u8)t];
    }

    ASTNode::ASTNode(NodeType type)
        : type{type}
    {
    }
    TkString ASTNode::toString() const
    {
        return TkString::format("[%s]", toString(type));
    }

    static const char *s_DeclType[] = {
        "Unknwon",
        "Class",
        "Struct",
        "Enum"};
    TkStringView ASTTypeDecl::toString(DeclType t)
    {
        return s_DeclType[(u8)t];
    }

    ASTTypeDecl::ASTTypeDecl(const TkString &name, DeclType kind)
        : ASTNode{NodeType::kTypeDecl}, kind{kind}, name{name}
    {
    }
    ASTTypeDecl::~ASTTypeDecl()
    {
        members.clearDelete();
        baseClasses.clear();
    }
    TkString ASTTypeDecl::toString() const
    {
        return TkString::format("ASTTypeDecl (%s): %s", toString(kind), name);
    }

    ASTTransUnit::ASTTransUnit()
        : ASTNode{kTransUnit}
    {
    }
    ASTTransUnit::~ASTTransUnit()
    {
        decls.clearDelete();
    }
    TkString ASTTransUnit::toString() const
    {
        TkStringBuilder result;
        for (auto *item : decls)
        {
            result.append("\n├── ").appendLine(item->toString());
        }
        return result.toString();
    }

    ASTNamespace::ASTNamespace(const TkString &name)
        : ASTNode{kNamespace}, name{name}
    {
    }
    ASTNamespace::~ASTNamespace()
    {
        decls.clearDelete();
    }
    TkString ASTNamespace::toString() const
    {
        TkStringBuilder result;
        result.append("ASTNamespace: ").appendLine(name);
        for (auto *item : decls)
        {
            result.append("\n   ├── ").appendLine(item->toString());
        }
        return result.toString();
    }

    ASTField::ASTField()
        : ASTNode{kField}
    {
    }
    ASTField::~ASTField()
    {
        attributes.clear();
    }
    TkString ASTField::toString() const
    {
        TkStringBuilder b{};
        for (auto &item : attributes)
        {
            b.append(item).append(' ');
        }
        return TkString::format("ASTField: {0} {1} [{2}] [getFunc: {3}] [setFunc: {4}]", type, name, b.toString(), getFunc.isEmpty() ? "Default" : getFunc, setFunc.isEmpty() ? "Default" : setFunc);
    }

    ASTFunction::ASTFunction()
        : ASTNode{kFunction}
    {
    }
    ASTFunction::~ASTFunction()
    {
        params.clear();
        attributes.clear();
    }
    TkString ASTFunction::toString() const
    {
        TkStringBuilder b{};
        for (auto &item : params)
        {
            b.append(item.key).append(' ').append(item.value).append(',');
        }
        auto p = b.toString();

        p.remove(p.length() - 1, 1);
        b.clear();

        for (auto &item : attributes)
        {
            b.append(item).append(' ');
        }

        return TkString::format("ASTFunction: {0} {1}({2}) [{3}]", type, name, p, b.toString());
    }

    ASTEnumValue::ASTEnumValue()
        : ASTNode{kEnumValue}
    {
    }
    ASTEnumValue::~ASTEnumValue()
    {
        value.reset();
    }
    TkString ASTEnumValue::toString() const
    {
        return TkString::format("{0} {1}", name, value.hasValue() ? TkStringUtils::toString(value.getValue()) : "");
    }

    ASTEnum::ASTEnum(const TkString &name)
        : ASTTypeDecl{name, ASTTypeDecl::kEnum}
    {
    }
    ASTEnum::~ASTEnum()
    {
        members.clear();
    }
    TkString ASTEnum::toString() const
    {
        TkStringBuilder b{};
        b.append("ASTTypeDecl (Enum): ").appendLine(name);
        for (auto &item : members)
        {
            b.append("\n   ├──").appendLine(item->toString());
        }
        return b.toString();
    }

} // namespace cobalt