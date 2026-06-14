#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringBuilder.hpp>

namespace toolkit {

    struct ASTTransUnit;
    class ParserContext;

    struct RkAttributeModel
    {
        TkString name;
        TkString value;
    };

    struct RkParamModel
    {
        TkString name;
        TkString cppTypeText;
        TkString typeOfText;
        TkString passing;
    };

    struct RkFieldModel
    {
        TkString name;
        TkString cppTypeText;
        TkString typeOfText;
        TkString getterName;
        TkString setterName;
        TkArray<RkAttributeModel> attributes;
        bool isStatic = false;
        bool isReadonly = false;
        bool isTransient = false;
        bool isPointer = false;
        bool isReference = false;
    };

    struct RkMethodModel
    {
        TkString name;
        TkString cppReturnTypeText;
        TkString returnTypeOfText;
        TkString returnKind;
        TkString flags;
        TkArray<RkParamModel> params;
        TkArray<RkAttributeModel> attributes;
        bool isStatic = false;
        bool isConst = false;
        bool isConstructor = false;
    };

    struct RkConstructorModel
    {
        TkArray<RkParamModel> params;
    };

    struct RkEnumItemModel
    {
        TkString name;
        TkString valueText;
    };

    struct RkTypeModel
    {
        enum Kind : u8
        {
            kClass,
            kStruct,
            kEnum,
            kInterface
        };

        Kind kind = kClass;
        TkString name;
        TkString namespacePath;
        TkString qualifiedCppName;
        TkString symbolName;
        TkString sourceFile;
        TkArray<TkString> baseTypes;
        TkString reflectedBaseType;
        TkArray<RkFieldModel> fields;
        TkArray<RkMethodModel> methods;
        TkArray<RkConstructorModel> constructors;
        TkArray<RkEnumItemModel> enumItems;
        TkString enumUnderlyingType;
        bool isEnumClass = false;
        bool derivesFromRkObject = false;
        bool hasSpawnConstructor = false;
        bool noSpawn = false;
    };

    struct RkFileModel
    {
        TkString inputFile;
        TkString outputSource;
        TkString includePath;
        TkArray<RkTypeModel> types;
    };

    class CodeWriter
    {
    public:
        void indent();
        void unindent();
        void line(const TkString& text = TkString::Empty);
        void raw(const TkString& text);
        const TkStringBuilder& builder() const;
        TkString toString() const;

    private:
        TkStringBuilder m_builder;
        i32 m_indent = 0;
    };

    class ModelBuilder
    {
    public:
        bool build(const TkString& inputFile, const TkString& outputDir, const TkString& outputName, ASTTransUnit* root, RkFileModel* out);

    private:
        void collectNode(struct ASTNode* node, const TkString& namespacePath, RkFileModel* out);
        RkTypeModel buildType(struct ASTTypeDecl* type, const TkString& namespacePath, const TkString& inputFile);
        RkTypeModel buildEnum(struct ASTEnum* type, const TkString& namespacePath, const TkString& inputFile);
    };

    class Generator
    {
    public:
        bool generate(ParserContext& ctx);
        bool generate(const RkFileModel& model);

    private:
        bool emitSource(const RkFileModel& model, CodeWriter& w);
        bool validateModel(const RkFileModel& model) const;
        bool writeIfChanged(const TkString& path, const TkString& text);

        void emitType(const RkTypeModel& type, CodeWriter& w);
        void emitFieldAccessors(const RkTypeModel& type, CodeWriter& w);
        void emitClassLifecycle(const RkTypeModel& type, CodeWriter& w);
        void emitMethodInvokes(const RkTypeModel& type, const TkArray<TkString>& methodSymbols, CodeWriter& w);
        void emitFieldArray(const RkTypeModel& type, CodeWriter& w);
        void emitMethodArray(const RkTypeModel& type, const TkArray<TkString>& methodSymbols, CodeWriter& w);
        void emitEnumItems(const RkTypeModel& type, CodeWriter& w);
        void emitDescAndRegister(const RkTypeModel& type, CodeWriter& w);
        static TkArray<TkString> buildMethodSymbols(const RkTypeModel& type);
    };

}
