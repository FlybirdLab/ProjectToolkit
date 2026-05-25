#include "Generator.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/utils/StringUtils.hpp"
#include "parser/ASTNodes.hpp"
#include "parser/Context.hpp"
#include "parser/Source.hpp"

#include <cctype>
#include <filesystem>

namespace toolkit {

    static TkString trimCopy(const TkString& input)
    {
        i32 begin = 0;
        i32 end = input.length();
        while (begin < end && isspace((unsigned char)input[begin])) begin++;
        while (end > begin && isspace((unsigned char)input[end - 1])) end--;
        return input.substring(begin, end - begin);
    }

    static TkString sanitizeSymbol(const TkString& text)
    {
        TkString out = text;
        out.replace(':', '_');
        out.replace('/', '_');
        out.replace('\\', '_');
        out.replace('.', '_');
        out.replace(' ', '_');
        out.replace('<', '_');
        out.replace('>', '_');
        out.replace('*', '_');
        out.replace('&', '_');
        return out;
    }

    static TkString fileStem(const TkString& path)
    {
        return TkString(std::filesystem::path(path.getText()).stem().string().c_str());
    }

    static TkString fileName(const TkString& path)
    {
        return TkString(std::filesystem::path(path.getText()).filename().string().c_str());
    }

    static TkString joinPath(const TkString& lhs, const TkString& rhs)
    {
        return TkString(std::filesystem::path(lhs.getText()).append(rhs.getText()).string().c_str());
    }

    static TkString normalizeType(TkString type)
    {
        type = trimCopy(type);
        type.replace("const ", "");
        type.replace("volatile ", "");
        type.replace("*", "");
        type.replace("&", "");
        return trimCopy(type);
    }

    static bool isBuiltinType(const TkString& type)
    {
        return type == "void" ||
               type == "bool" ||
               type == "char" ||
               type == "signed char" ||
               type == "unsigned char" ||
               type == "short" ||
               type == "unsigned short" ||
               type == "int" ||
               type == "unsigned int" ||
               type == "long" ||
               type == "unsigned long" ||
               type == "long long" ||
               type == "unsigned long long" ||
               type == "float" ||
               type == "double" ||
               type == "long double";
    }

    static TkString qualifyTypeName(const TkString& type, const TkString& namespacePath)
    {
        const TkString normalized = normalizeType(type);
        if (normalized.isEmpty() || isBuiltinType(normalized) || normalized.find("::") >= 0 || namespacePath.isEmpty())
            return normalized;
        return "::" + namespacePath + "::" + normalized;
    }

    static bool isRkObjectBase(const TkString& type)
    {
        const TkString normalized = normalizeType(type);
        return normalized == "RkObject" || normalized == "toolkit::RkObject" || normalized.endsWith(TkStringView{"::RkObject"});
    }

    static bool isKnownReflectedType(const TkString& type, const TkArray<TkString>& reflectedTypes)
    {
        const TkString normalized = normalizeType(type);
        if (isRkObjectBase(normalized))
            return true;
        for (const TkString& reflectedType : reflectedTypes)
        {
            if (normalized == reflectedType)
                return true;
            if (reflectedType.endsWith("::" + normalized))
                return true;
        }
        return false;
    }

    static TkString typeOfText(
        const TkString& type,
        const TkString& namespacePath = TkString::Empty,
        const TkArray<TkString>& reflectedTypes = TkArray<TkString>())
    {
        const TkString normalized = qualifyTypeName(type, namespacePath);
        if (normalized.isEmpty() || normalized == "void")
            return "::toolkit::TypeHandle()";
        if (!isBuiltinType(normalizeType(type)) && !isKnownReflectedType(normalized, reflectedTypes))
            return "::toolkit::TypeHandle()";
        return TkString::format("::toolkit::typeOf<%s>()", normalized.getText());
    }

    static TkString fieldFlags(const RkFieldModel& field, bool ownerClass)
    {
        TkString result = "toolkit::FieldFlags::kNone";
        if (field.isStatic)
            result += " | toolkit::FieldFlags::kStatic";
        if (field.isReadonly)
            result += " | toolkit::FieldFlags::kReadOnly";
        if (ownerClass)
            result += " | toolkit::FieldFlags::kOwnerClass";
        return result;
    }

    static TkString methodFlags(const RkMethodModel& method)
    {
        TkString result = "toolkit::MethodFlags::kNone";
        if (method.isStatic)
            result += " | toolkit::MethodFlags::kStatic";
        if (method.isConst)
            result += " | toolkit::MethodFlags::kConst";
        return result;
    }

    static TkString returnKindFor(const TkString& type)
    {
        const TkString trimmed = trimCopy(type);
        if (trimmed == "void" || trimmed.isEmpty())
            return "toolkit::ReturnKind::kVoid";
        if (trimmed.find("*") >= 0)
            return "toolkit::ReturnKind::kPointer";
        if (trimmed.find("const") >= 0 && trimmed.find("&") >= 0)
            return "toolkit::ReturnKind::kConstLValueRef";
        if (trimmed.find("&") >= 0)
            return "toolkit::ReturnKind::kLValueRef";
        return "toolkit::ReturnKind::kValue";
    }

    static TkString passingFor(const TkString& type)
    {
        const TkString trimmed = trimCopy(type);
        if (trimmed.find("*") >= 0)
            return "toolkit::ArgPassing::kPointer";
        if (trimmed.find("const") >= 0 && trimmed.find("&") >= 0)
            return "toolkit::ArgPassing::kConstLValueRef";
        if (trimmed.find("&&") >= 0)
            return "toolkit::ArgPassing::kRValueRef";
        if (trimmed.find("&") >= 0)
            return "toolkit::ArgPassing::kLValueRef";
        return "toolkit::ArgPassing::kValue";
    }

    void CodeWriter::indent() { m_indent++; }
    void CodeWriter::unindent() { if (m_indent > 0) m_indent--; }
    void CodeWriter::line(const TkString& text)
    {
        for (i32 i = 0; i < m_indent; i++)
            m_builder.append("    ");
        m_builder.appendLine(text);
    }
    void CodeWriter::raw(const TkString& text) { m_builder.append(text); }
    const TkStringBuilder& CodeWriter::builder() const { return m_builder; }
    TkString CodeWriter::toString() const { return m_builder.toString(); }

    bool ModelBuilder::build(const TkString& inputFile, const TkString& outputDir, ASTTransUnit* root, RkFileModel* out)
    {
        if (root == nullptr || out == nullptr)
            return false;
        out->inputFile = inputFile;
        out->includePath = inputFile;
        out->includePath.replace('\\', '/');
        const TkString stem = fileStem(inputFile);
        out->outputSource = joinPath(outputDir, stem + ".rkgen.cpp");
        out->types.clear();
        for (ASTNode* node : root->decls)
            collectNode(node, TkString::Empty, out);

        TkArray<TkString> reflectedTypes;
        for (const RkTypeModel& type : out->types)
            reflectedTypes.add(type.qualifiedCppName);
        for (RkTypeModel& type : out->types)
        {
            if (type.kind == RkTypeModel::kClass)
            {
                for (const TkString& baseType : type.baseTypes)
                {
                    if (isRkObjectBase(baseType))
                        type.reflectedBaseType = "::toolkit::typeOf<::toolkit::RkObject>()";
                    else if (isKnownReflectedType(qualifyTypeName(baseType, type.namespacePath), reflectedTypes))
                        type.reflectedBaseType = typeOfText(baseType, type.namespacePath, reflectedTypes);
                }
            }
            for (RkFieldModel& field : type.fields)
                field.typeOfText = typeOfText(field.cppTypeText, type.namespacePath, reflectedTypes);
            for (RkConstructorModel& ctor : type.constructors)
            {
                for (RkParamModel& param : ctor.params)
                    param.typeOfText = typeOfText(param.cppTypeText, type.namespacePath, reflectedTypes);
            }
            for (RkMethodModel& method : type.methods)
            {
                method.returnTypeOfText = typeOfText(method.cppReturnTypeText, type.namespacePath, reflectedTypes);
                for (RkParamModel& param : method.params)
                    param.typeOfText = typeOfText(param.cppTypeText, type.namespacePath, reflectedTypes);
            }
        }
        return true;
    }

    void ModelBuilder::collectNode(ASTNode* node, const TkString& namespacePath, RkFileModel* out)
    {
        if (node == nullptr)
            return;
        if (node->type == ASTNode::kNamespace)
        {
            auto* ns = static_cast<ASTNamespace*>(node);
            const TkString nextNamespace = namespacePath.isEmpty() ? ns->name : namespacePath + "::" + ns->name;
            for (ASTNode* child : ns->decls)
                collectNode(child, nextNamespace, out);
        }
        else if (node->type == ASTNode::kTypeDecl)
        {
            auto* type = static_cast<ASTTypeDecl*>(node);
            out->types.add(buildType(type, namespacePath, out->inputFile));
        }
    }

    RkTypeModel ModelBuilder::buildEnum(ASTEnum* type, const TkString& namespacePath, const TkString& inputFile)
    {
        RkTypeModel model;
        model.kind = RkTypeModel::kEnum;
        model.name = type->name;
        model.namespacePath = namespacePath;
        model.qualifiedCppName = namespacePath.isEmpty() ? type->name : namespacePath + "::" + type->name;
        model.symbolName = sanitizeSymbol(model.qualifiedCppName);
        model.sourceFile = inputFile;
        model.enumUnderlyingType = type->underlyingType;
        model.isEnumClass = type->isEnumClass;
        for (auto& item : type->values)
        {
            RkEnumItemModel enumItem;
            enumItem.name = item.name;
            enumItem.valueText = item.value.hasValue() ? TkStringUtils::toString(item.value.getValue()) : item.rawValue;
            if (enumItem.valueText.isEmpty())
                enumItem.valueText = "0";
            model.enumItems.add(enumItem);
        }
        return model;
    }

    RkTypeModel ModelBuilder::buildType(ASTTypeDecl* type, const TkString& namespacePath, const TkString& inputFile)
    {
        if (type->kind == ASTTypeDecl::kEnum)
            return buildEnum(static_cast<ASTEnum*>(type), namespacePath, inputFile);

        RkTypeModel model;
        model.kind = type->kind == ASTTypeDecl::kStruct ? RkTypeModel::kStruct : RkTypeModel::kClass;
        model.name = type->name;
        model.namespacePath = namespacePath;
        model.qualifiedCppName = namespacePath.isEmpty() ? type->name : namespacePath + "::" + type->name;
        model.symbolName = sanitizeSymbol(model.qualifiedCppName);
        model.sourceFile = inputFile;
        model.baseTypes.add(type->baseClasses);
        for (const TkString& base : model.baseTypes)
        {
            if (isRkObjectBase(base))
                model.derivesFromRkObject = true;
        }

        for (ASTNode* member : type->members)
        {
            if (member->type == ASTNode::kField)
            {
                auto* field = static_cast<ASTField*>(member);
                if (field->name.isEmpty() || field->type.isEmpty())
                    continue;
                RkFieldModel dst;
                dst.name = field->name;
                dst.cppTypeText = field->type;
                dst.getterName = field->getFunc;
                dst.setterName = field->setFunc;
                dst.isStatic = field->isStatic;
                dst.isReadonly = field->isReadonly;
                dst.isTransient = field->isTransient;
                dst.isPointer = field->isPointer;
                dst.isReference = field->isReference;
                for (auto& attr : field->attributes)
                    dst.attributes.add(RkAttributeModel{ attr, TkString::Empty });
                model.fields.add(dst);
            }
            else if (member->type == ASTNode::kFunction)
            {
                auto* fn = static_cast<ASTFunction*>(member);
                if (fn->isConstructor)
                {
                    if (!fn->isReflectConstructor)
                        continue;
                    RkConstructorModel ctor;
                    for (auto& p : fn->params)
                    {
                        RkParamModel param;
                        param.cppTypeText = p.type;
                        param.name = p.name;
                        param.passing = passingFor(p.type);
                        ctor.params.add(param);
                    }
                    model.constructors.add(ctor);
                    if (model.kind == RkTypeModel::kClass)
                        model.hasSpawnConstructor = true;
                    continue;
                }
                RkMethodModel dst;
                dst.name = fn->name;
                dst.cppReturnTypeText = fn->returnType;
                dst.returnKind = returnKindFor(fn->returnType);
                dst.isStatic = fn->isStatic;
                dst.isConst = fn->isConst;
                for (auto& p : fn->params)
                {
                    RkParamModel param;
                    param.cppTypeText = p.type;
                    param.name = p.name;
                    param.passing = passingFor(p.type);
                    dst.params.add(param);
                }
                model.methods.add(dst);
            }
        }
        return model;
    }

    bool Generator::generate(ParserContext& ctx)
    {
        ModelBuilder builder;
        for (auto& item : ctx.m_files)
        {
            if (item.value == nullptr)
                continue;
            ASTTransUnit* root = static_cast<ASTTransUnit*>(item.value->getASTRoot());
            if (root == nullptr)
                continue;
            RkFileModel model;
            if (!builder.build(item.key, ctx.m_data.outputDir, root, &model))
                return false;
            if (!generate(model))
                return false;
        }
        return true;
    }

    bool Generator::generate(const RkFileModel& model)
    {
        if (model.types.isEmpty())
        {
            CodeWriter source;
            source.line(TkString::format("// No reflected declarations found in %s.", model.inputFile.getText()));
            if (!writeIfChanged(model.outputSource, source.toString()))
                return false;

            TkString staleHeader = model.outputSource;
            staleHeader.replace(".rkgen.cpp", ".rkgen.hpp");
            if (staleHeader != model.outputSource && TkFileSystem::isFileExist(staleHeader))
                TkFileSystem::deleteFile(staleHeader);

            return true;
        }

        if (!validateModel(model))
            return false;

        CodeWriter source;
        if (!emitSource(model, source))
            return false;
        if (!writeIfChanged(model.outputSource, source.toString()))
            return false;

        TkString staleHeader = model.outputSource;
        staleHeader.replace(".rkgen.cpp", ".rkgen.hpp");
        if (staleHeader != model.outputSource && TkFileSystem::isFileExist(staleHeader))
            TkFileSystem::deleteFile(staleHeader);

        return true;
    }

    bool Generator::validateModel(const RkFileModel& model) const
    {
        for (const RkTypeModel& type : model.types)
        {
            if (type.kind != RkTypeModel::kClass || !type.derivesFromRkObject)
                continue;

            if (!type.hasSpawnConstructor)
            {
                LogError("[RkGen] cclass %s derives from RkObject and must declare a reflected constructor, e.g. 'cctor %s() = default;'.",
                    type.qualifiedCppName.getText(),
                    type.name.getText());
                return false;
            }
        }
        return true;
    }

    bool Generator::writeIfChanged(const TkString& path, const TkString& text)
    {
        if (TkFileSystem::isFileExist(path))
        {
            OpenFileParams readParams{ .path = path };
            FileHandle readFile = TkFileSystem::openFile(readParams);
            TkString oldText;
            if (readFile != kInvalidFileHandle)
            {
                TkFileSystem::readAsText(readFile, oldText);
                TkFileSystem::closeFile(readFile);
                if (oldText == text)
                    return true;
            }
        }
        OpenFileParams params{ .path = path, .mode = FileMode::kCreateAlways, .access = FileAccess::kWrite, .share = FileShare::kRead };
        FileHandle file = TkFileSystem::openFile(params);
        if (file == kInvalidFileHandle)
            return false;
        const bool ok = TkFileSystem::writeText(file, text, Encoding::kUTF8);
        TkFileSystem::closeFile(file);
        return ok;
    }

    bool Generator::emitSource(const RkFileModel& model, CodeWriter& w)
    {
        const TkString fileRegisterSymbol = sanitizeSymbol(fileStem(model.inputFile));

        w.line(TkString::format("#include \"%s\"", model.includePath.getText()));
        w.line("#include <toolkit/reflect/RkModule.hpp>");
        w.line("#include <toolkit/reflect/RkManager.hpp>");
        w.line("#include <toolkit/reflect/RkObject.hpp>");
        w.line("#include <toolkit/reflect/RkType.hpp>");
        w.line("#include <toolkit/reflect/invoking/ArgValue.hpp>");
        w.line("#include <toolkit/reflect/invoking/InvokeContext.hpp>");
        w.line("#include <toolkit/reflect/invoking/ReturnValue.hpp>");
        w.line("#include <toolkit/reflect/type/TypeResolver.hpp>");
        w.line("#include <toolkit/memory/Memory.hpp>");
        w.line("#include <cstddef>");
        w.line("");
        w.line("namespace {");
        w.indent();
        for (const RkTypeModel& type : model.types)
            emitType(type, w);
        w.unindent();
        w.line("}");
        w.line("");
        for (const RkTypeModel& type : model.types)
            emitDescAndRegister(type, w);
        w.line(TkString::format("void rkgen_register_file_%s(::toolkit::RkModule& module)", fileRegisterSymbol.getText()));
        w.line("{");
        w.indent();
        for (const RkTypeModel& type : model.types)
            w.line(TkString::format("rkgen_register_%s(module);", type.symbolName.getText()));
        w.unindent();
        w.line("}");
        return true;
    }

    void Generator::emitType(const RkTypeModel& type, CodeWriter& w)
    {
        if (type.kind == RkTypeModel::kEnum)
        {
            emitEnumItems(type, w);
            return;
        }
        emitClassLifecycle(type, w);
        emitFieldAccessors(type, w);
        emitMethodInvokes(type, w);
        emitFieldArray(type, w);
        emitMethodArray(type, w);
    }

    void Generator::emitFieldAccessors(const RkTypeModel& type, CodeWriter& w)
    {
        for (const RkFieldModel& field : type.fields)
        {
            w.line(TkString::format("void* rk_field_accessor_%s_%s(void* instance)", type.symbolName.getText(), field.name.getText()));
            w.line("{");
            w.indent();
            if (field.isStatic)
                w.line(TkString::format("return &%s::%s;", type.qualifiedCppName.getText(), field.name.getText()));
            else if (!field.getterName.isEmpty())
            {
                w.line("if (instance == nullptr) return nullptr;");
                w.line(TkString::format("return &static_cast<%s*>(instance)->%s();", type.qualifiedCppName.getText(), field.getterName.getText()));
            }
            else
            {
                w.line("if (instance == nullptr) return nullptr;");
                w.line(TkString::format("return &static_cast<%s*>(instance)->%s;", type.qualifiedCppName.getText(), field.name.getText()));
            }
            w.unindent();
            w.line("}");
            w.line("");
        }
    }

    void Generator::emitClassLifecycle(const RkTypeModel& type, CodeWriter& w)
    {
        if (type.kind != RkTypeModel::kClass || !type.derivesFromRkObject || !type.hasSpawnConstructor)
            return;

        w.line(TkString::format("::toolkit::RkObject* rk_spawn_%s(const ::toolkit::ObjectSpawnParams& params)", type.symbolName.getText()));
        w.line("{");
        w.indent();
        for (const RkConstructorModel& ctor : type.constructors)
        {
            w.line(TkString::format("if (params.argCount == %i)", ctor.params.count()));
            w.line("{");
            w.indent();
            if (!ctor.params.isEmpty())
                w.line("if (params.args == nullptr) return nullptr;");
            TkStringBuilder args;
            for (i32 i = 0; i < ctor.params.count(); i++)
            {
                const RkParamModel& param = ctor.params[i];
                const TkString baseType = qualifyTypeName(param.cppTypeText, type.namespacePath);
                w.line(TkString::format("if (params.args[%i].data == nullptr) return nullptr;", i));
                w.line(TkString::format("if (params.args[%i].type != %s) return nullptr;", i, param.typeOfText.getText()));
                w.line(TkString::format("if (params.args[%i].passing != %s) return nullptr;", i, param.passing.getText()));
                if (param.passing == "toolkit::ArgPassing::kPointer")
                    w.line(TkString::format("auto %s = *reinterpret_cast<%s**>(params.args[%i].data);", param.name.getText(), baseType.getText(), i));
                else
                    w.line(TkString::format("auto& %s = *reinterpret_cast<%s*>(params.args[%i].data);", param.name.getText(), baseType.getText(), i));
                if (i > 0)
                    args.append(", ");
                args.append(param.name);
            }
            if (ctor.params.isEmpty())
                w.line(TkString::format("auto* object = ::toolkit::TkMemory::newObject<%s>(::toolkit::RkManager::getAllocator());", type.qualifiedCppName.getText()));
            else
                w.line(TkString::format("auto* object = ::toolkit::TkMemory::newObject<%s>(::toolkit::RkManager::getAllocator(), %s);", type.qualifiedCppName.getText(), args.toString().getText()));
            w.line("::toolkit::RkObjectSpawnAccess::apply(object, params);");
            w.line("return object;");
            w.unindent();
            w.line("}");
        }
        w.line("return nullptr;");
        w.unindent();
        w.line("}");
        w.line("");

        w.line(TkString::format("void rk_destroy_%s(::toolkit::RkObject* object)", type.symbolName.getText()));
        w.line("{");
        w.indent();
        w.line(TkString::format("::toolkit::TkMemory::deletePtr(::toolkit::RkManager::getAllocator(), static_cast<%s*>(object));", type.qualifiedCppName.getText()));
        w.unindent();
        w.line("}");
        w.line("");
    }

    void Generator::emitMethodInvokes(const RkTypeModel& type, CodeWriter& w)
    {
        for (const RkMethodModel& method : type.methods)
        {
            w.line(TkString::format("bool rk_invoke_%s_%s(::toolkit::InvokeContext& ctx)", type.symbolName.getText(), method.name.getText()));
            w.line("{");
            w.indent();
            w.line(TkString::format("if (ctx.argCount != %i) return false;", method.params.count()));
            if (!method.isStatic)
                w.line(TkString::format("auto* self = static_cast<%s*>(ctx.instance);", type.qualifiedCppName.getText()));
            if (!method.isStatic)
                w.line("if (self == nullptr) return false;");
            for (i32 i = 0; i < method.params.count(); i++)
            {
                const auto& param = method.params[i];
                const TkString baseType = qualifyTypeName(param.cppTypeText, type.namespacePath);
                if (param.passing == "toolkit::ArgPassing::kPointer")
                    w.line(TkString::format("auto %s = *reinterpret_cast<%s**>(ctx.args[%i].data);", param.name.getText(), baseType.getText(), i));
                else
                    w.line(TkString::format("auto& %s = *reinterpret_cast<%s*>(ctx.args[%i].data);", param.name.getText(), baseType.getText(), i));
            }
            TkStringBuilder args;
            for (i32 i = 0; i < method.params.count(); i++)
            {
                if (i > 0) args.append(", ");
                args.append(method.params[i].name);
            }
            const TkString callPrefix = method.isStatic ? type.qualifiedCppName + "::" : "self->";
            if (method.returnKind == "toolkit::ReturnKind::kVoid")
            {
                w.line(TkString::format("%s%s(%s);", callPrefix.getText(), method.name.getText(), args.toString().getText()));
            }
            else if (method.returnKind == "toolkit::ReturnKind::kValue")
            {
                w.line("if (ctx.result == nullptr) return false;");
                w.line(TkString::format("*reinterpret_cast<%s*>(ctx.result) = %s%s(%s);", qualifyTypeName(method.cppReturnTypeText, type.namespacePath).getText(), callPrefix.getText(), method.name.getText(), args.toString().getText()));
            }
            else
            {
                w.line("if (ctx.returnValue == nullptr) return false;");
                w.line(TkString::format("ctx.returnValue->type = %s;", method.returnTypeOfText.getText()));
                w.line(TkString::format("ctx.returnValue->kind = %s;", method.returnKind.getText()));
                w.line(TkString::format("ctx.returnValue->data = &%s%s(%s);", callPrefix.getText(), method.name.getText(), args.toString().getText()));
            }
            w.line("return true;");
            w.unindent();
            w.line("}");
            w.line("");
        }
    }

    void Generator::emitFieldArray(const RkTypeModel& type, CodeWriter& w)
    {
        w.line(TkString::format("const ::toolkit::RkField s_fields_%s[] = {", type.symbolName.getText()));
        w.indent();
        if (type.fields.isEmpty())
            w.line("{},");
        for (const RkFieldModel& field : type.fields)
        {
            const TkString offset = type.kind == RkTypeModel::kStruct && field.getterName.isEmpty()
                ? TkString::format("static_cast<::toolkit::u32>(offsetof(%s, %s))", type.qualifiedCppName.getText(), field.name.getText())
                : "0";
            const TkString accessor = type.kind == RkTypeModel::kClass || !field.getterName.isEmpty() || field.isStatic
                ? TkString::format("&rk_field_accessor_%s_%s", type.symbolName.getText(), field.name.getText())
                : "nullptr";
            w.line(TkString::format("{ \"%s\", %s, %s, %s, {}, %s },", field.name.getText(), field.typeOfText.getText(), offset.getText(), fieldFlags(field, type.kind == RkTypeModel::kClass).getText(), accessor.getText()));
        }
        w.unindent();
        w.line("};");
        w.line("");
    }

    void Generator::emitMethodArray(const RkTypeModel& type, CodeWriter& w)
    {
        for (const RkMethodModel& method : type.methods)
        {
            w.line(TkString::format("const ::toolkit::RkParam s_params_%s_%s[] = {", type.symbolName.getText(), method.name.getText()));
            w.indent();
            if (method.params.isEmpty())
                w.line("{},");
            for (const RkParamModel& param : method.params)
                w.line(TkString::format("{ \"%s\", %s, %s },", param.name.getText(), param.typeOfText.getText(), param.passing.getText()));
            w.unindent();
            w.line("};");
        }
        w.line(TkString::format("const ::toolkit::RkMethod s_methods_%s[] = {", type.symbolName.getText()));
        w.indent();
        if (type.methods.isEmpty())
            w.line("{},");
        for (const RkMethodModel& method : type.methods)
        {
            w.line(TkString::format("{ \"%s\", 0, %s, %s, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_%s_%s, %i), %i, %s, &rk_invoke_%s_%s, {} },",
                method.name.getText(),
                method.returnTypeOfText.getText(),
                method.returnKind.getText(),
                type.symbolName.getText(),
                method.name.getText(),
                method.params.count(),
                method.params.count(),
                methodFlags(method).getText(),
                type.symbolName.getText(),
                method.name.getText()));
        }
        w.unindent();
        w.line("};");
        w.line("");
    }

    void Generator::emitEnumItems(const RkTypeModel& type, CodeWriter& w)
    {
        w.line(TkString::format("const ::toolkit::EnumItem s_enum_items_%s[] = {", type.symbolName.getText()));
        w.indent();
        if (type.enumItems.isEmpty())
            w.line("{},");
        for (const RkEnumItemModel& item : type.enumItems)
            w.line(TkString::format("{ \"%s\", static_cast<::toolkit::i64>(%s) },", item.name.getText(), item.valueText.getText()));
        w.unindent();
        w.line("};");
        w.line("");
    }

    void Generator::emitDescAndRegister(const RkTypeModel& type, CodeWriter& w)
    {
        w.line(TkString::format("void rkgen_register_%s(::toolkit::RkModule& module)", type.symbolName.getText()));
        w.line("{");
        w.indent();
        if (type.kind == RkTypeModel::kClass)
        {
            const TkString base = type.reflectedBaseType.isEmpty() ? "::toolkit::TypeHandle()" : type.reflectedBaseType;
            w.line("::toolkit::RkClassDesc desc{};");
            w.line(TkString::format("desc.name = \"%s\";", type.name.getText()));
            w.line(TkString::format("desc.fullName = \"%s\";", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.size = sizeof(%s);", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.align = alignof(%s);", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.baseType = %s;", base.getText()));
            if (type.derivesFromRkObject && type.hasSpawnConstructor)
            {
                w.line(TkString::format("desc.spawn = &rk_spawn_%s;", type.symbolName.getText()));
                w.line(TkString::format("desc.destroy = &rk_destroy_%s;", type.symbolName.getText()));
            }
            w.line(TkString::format("desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_%s, %i);", type.symbolName.getText(), type.fields.count()));
            w.line(TkString::format("desc.methods = ::toolkit::TkSpan<const ::toolkit::RkMethod>(s_methods_%s, %i);", type.symbolName.getText(), type.methods.count()));
            w.line("module.registerType(desc);");
        }
        else if (type.kind == RkTypeModel::kStruct)
        {
            w.line("::toolkit::RkStructDesc desc{};");
            w.line(TkString::format("desc.name = \"%s\";", type.name.getText()));
            w.line(TkString::format("desc.fullName = \"%s\";", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.size = sizeof(%s);", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.align = alignof(%s);", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_%s, %i);", type.symbolName.getText(), type.fields.count()));
            w.line("module.registerType(desc);");
        }
        else if (type.kind == RkTypeModel::kEnum)
        {
            w.line("::toolkit::RkEnumDesc desc{};");
            w.line(TkString::format("desc.name = \"%s\";", type.name.getText()));
            w.line(TkString::format("desc.fullName = \"%s\";", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.size = sizeof(%s);", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.align = alignof(%s);", type.qualifiedCppName.getText()));
            w.line(TkString::format("desc.items = ::toolkit::TkSpan<const ::toolkit::EnumItem>(s_enum_items_%s, %i);", type.symbolName.getText(), type.enumItems.count()));
            w.line("module.registerType(desc);");
        }
        w.unindent();
        w.line("}");
        w.line("");
    }

}
