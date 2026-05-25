#pragma once

#include "toolkit/types/Nullable.hpp"

#include <toolkit/collection/HashMap.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit {
    
    struct ASTNode 
    {
    public:
        enum NodeType : u8 {
            kUnknwon = 0,
            kTransUnit, // Root of CXX header file
            kTypeDecl,
            kNamespace,
            kField,
            kFunction,
            kEnumValue,
            kEnum
        };
        static TkStringView toString(NodeType);
        
    public:
        ASTNode(NodeType type);
        virtual ~ASTNode() = default;
        virtual TkString toString() const;

    public:
        struct CXXSourceFile* src;
        const NodeType type;
    };


    // Declearation Type Node (base class for struct/class/enum)
    struct ASTTypeDecl : ASTNode 
    {
    public:
        enum DeclType : u8 { 
            kUnknwon = 0,
            kClass, 
            kStruct, 
            kEnum 
        };
        static TkStringView toString(DeclType);
    
    public:
        ASTTypeDecl(const TkString& name, DeclType kind);
        virtual ~ASTTypeDecl() override;

        virtual TkString toString() const override;

    public:
        DeclType kind;
        const TkString name;
        TkArray<TkString> baseClasses;
        TkArray<ASTNode*> members;
    };

    
    // TopLevel Node for each CXX header file
    struct ASTTransUnit : ASTNode 
    {
    public:
        ASTTransUnit();
        virtual ~ASTTransUnit() override;

        virtual TkString toString() const override;

    public:
        TkArray<ASTNode*> decls;
    };

    
    // Namespace Node
    struct ASTNamespace : ASTNode 
    {
    public:
        ASTNamespace(const TkString&);
        virtual ~ASTNamespace() override;

        virtual TkString toString() const override;

    public:
        TkString name;
        TkArray<ASTNode*> decls;
    };


    // Field NodeType
    struct ASTField : ASTNode 
    {
    public:
        ASTField();
        virtual ~ASTField() override;

        virtual TkString toString() const override;

    public:
        TkString type;
        TkString name;

        TkString getFunc;
        TkString setFunc;

        TkArray<TkString> attributes;

        bool isStatic = false;
        bool isReadonly = false;
        bool isTransient = false;
        bool isConst = false;
        bool isPointer = false;
        bool isReference = false;
    };

    
    // Func Node
    struct ASTParam
    {
        TkString type;
        TkString name;
    };

    struct ASTFunction : ASTNode 
    {
    public:
        ASTFunction();
        virtual ~ASTFunction() override;

        virtual TkString toString() const override;

    public:
        TkString returnType;
        TkString name;

        TkArray<ASTParam> params;
        TkArray<TkString> attributes;

        bool isConstructor = false;
        bool isReflectConstructor = false;
        bool isStatic = false;
        bool isConst = false;
        bool isNoexcept = false;
    };


    // Enum Item NodeType
    struct ASTEnumValue : ASTNode 
    {
    public:
        ASTEnumValue();
        virtual ~ASTEnumValue() override;

        virtual TkString toString() const override;

    public:
        TkString name;
        TkNullable<i64> value;
        TkString rawValue;
    };

    
    // Enum Body NodeType
    struct ASTEnum : ASTTypeDecl 
    {
    public:
        ASTEnum(const TkString& name);
        virtual ~ASTEnum() override;

        virtual TkString toString() const override;

    public:
        TkArray<ASTEnumValue> values;
        TkString underlyingType;
        bool isEnumClass = false;
    };

}
