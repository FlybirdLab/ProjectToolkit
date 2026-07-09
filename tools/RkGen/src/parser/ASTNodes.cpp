#include "ASTNodes.hpp"

#include "toolkit/types/StringBuilder.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{

    static const char *s_NodeType[] = {
        "Unknwon",
        "TransUnit",
        "TypeDecl",
        "Namespace",
        "Field",
        "Function",
        "EnumValue",
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
        return TkString::format("[%s]", toString(type).getText());
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
        return TkString::format("ASTTypeDecl (%s): %s", toString(kind).getText(), name.getText());
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
        return TkString::format("ASTField: %s %s [%s] [getFunc: %s] [setFunc: %s]",
            type.getText(),
            name.getText(),
            b.toString().getText(),
            getFunc.isEmpty() ? "Default" : getFunc.getText(),
            setFunc.isEmpty() ? "Default" : setFunc.getText());
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
            b.append(item.type).append(' ').append(item.name).append(',');
        }
        auto p = b.toString();

        if (p.length() > 0)
        {
            p.remove(p.length() - 1, 1);
        }
        b.clear();

        for (auto &item : attributes)
        {
            b.append(item).append(' ');
        }

        return TkString::format("ASTFunction: %s %s(%s) [%s]", returnType.getText(), name.getText(), p.getText(), b.toString().getText());
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
        const TkString valueText = value.hasValue() ? TkStringUtils::toString(value.getValue()) : rawValue;
        return TkString::format("%s %s", name.getText(), valueText.getText());
    }

    ASTEnum::ASTEnum(const TkString &name)
        : ASTTypeDecl{name, ASTTypeDecl::kEnum}
    {
    }
    ASTEnum::~ASTEnum()
    {
    }
    TkString ASTEnum::toString() const
    {
        TkStringBuilder b{};
        b.append("ASTTypeDecl (Enum): ").appendLine(name);
        for (auto &item : values)
        {
            b.append("\n   ├──").appendLine(item.toString());
        }
        return b.toString();
    }

} // namespace cobalt
