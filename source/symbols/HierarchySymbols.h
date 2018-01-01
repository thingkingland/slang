//------------------------------------------------------------------------------
// HierarchySymbols.h
// Contains hierarchy-related symbol definitions.
//
// File is under the MIT license; see LICENSE for details.
//------------------------------------------------------------------------------
#pragma once

#include "binding/ConstantValue.h"
#include "symbols/Definition.h"
#include "symbols/SemanticFacts.h"
#include "symbols/StatementBodiedScope.h"
#include "symbols/Symbol.h"

namespace slang {

/// The root of a single compilation unit.
class CompilationUnitSymbol : public Symbol, public Scope {
public:
    explicit CompilationUnitSymbol(Compilation& compilation) :
        Symbol(SymbolKind::CompilationUnit, "$unit", SourceLocation()),
        Scope(compilation, this) {}

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::CompilationUnit; }
};

/// A SystemVerilog package construct.
class PackageSymbol : public Symbol, public Scope {
public:
    PackageSymbol(Compilation& compilation, string_view name, SourceLocation loc) :
        Symbol(SymbolKind::Package, name, loc),
        Scope(compilation, this) {}

    static PackageSymbol& fromSyntax(Compilation& compilation, const ModuleDeclarationSyntax& syntax);
    static bool isKind(SymbolKind kind) { return kind == SymbolKind::Package; }
};

/// Base class for module, interface, and program instance symbols.
class InstanceSymbol : public Symbol, public Scope {
public:
    static void fromSyntax(Compilation& compilation, const HierarchyInstantiationSyntax& syntax,
                           const Scope& scope, SmallVector<const Symbol*>& results);

protected:
    InstanceSymbol(SymbolKind kind, Compilation& compilation, string_view name, SourceLocation loc) :
        Symbol(kind, name, loc),
        Scope(compilation, this) {}
};

class ModuleInstanceSymbol : public InstanceSymbol {
public:
    ModuleInstanceSymbol(Compilation& compilation, string_view name, SourceLocation loc) :
        InstanceSymbol(SymbolKind::ModuleInstance, compilation, name, loc) {}

    static ModuleInstanceSymbol& instantiate(Compilation& compilation, string_view name, SourceLocation loc,
                                             const Definition& definition);

    struct ParameterMetadata {
        const Definition::ParameterDecl* decl;
        const Type* type;
        ConstantValue value;
    };

    static ModuleInstanceSymbol& instantiate(Compilation& compilation, string_view name, SourceLocation loc,
                                             const Definition& definition, span<const ParameterMetadata> parameters);

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::ModuleInstance; }
};

class SequentialBlockSymbol : public Symbol, public StatementBodiedScope {
public:
    SequentialBlockSymbol(Compilation& compilation, SourceLocation loc) :
        Symbol(SymbolKind::SequentialBlock, "", loc),
        StatementBodiedScope(compilation, this) {}

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::SequentialBlock; }
};

class ProceduralBlockSymbol : public Symbol, public StatementBodiedScope {
public:
    ProceduralBlockKind procedureKind;

    ProceduralBlockSymbol(Compilation& compilation, SourceLocation loc, ProceduralBlockKind procedureKind) :
        Symbol(SymbolKind::ProceduralBlock, "", loc),
        StatementBodiedScope(compilation, this),
        procedureKind(procedureKind) {}

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::ProceduralBlock; }
};

/// Represents blocks that are instantiated by a loop generate or conditional
/// generate construct. These blocks can contain a bunch of members, or just
/// a single item. They can also contain an implicit parameter representing
/// the loop iteration value.
class GenerateBlockSymbol : public Symbol, public Scope {
public:
    GenerateBlockSymbol(Compilation& compilation, string_view name, SourceLocation loc) :
        Symbol(SymbolKind::GenerateBlock, name, loc),
        Scope(compilation, this) {}

    /// Creates a generate block from the given if-generate syntax node. Note that
    /// this can return null if the condition is false and there is no else block.
    static GenerateBlockSymbol* fromSyntax(Compilation& compilation, const IfGenerateSyntax& syntax,
                                           const Scope& parent);

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::GenerateBlock; }
};

/// Represents an array of generate blocks, as generated by a loop generate construct.
class GenerateBlockArraySymbol : public Symbol, public Scope {
public:
    GenerateBlockArraySymbol(Compilation& compilation, string_view name, SourceLocation loc) :
        Symbol(SymbolKind::GenerateBlockArray, name, loc),
        Scope(compilation, this) {}

    /// Creates a generate block array from the given loop-generate syntax node.
    static GenerateBlockArraySymbol& fromSyntax(Compilation& compilation,
                                                const LoopGenerateSyntax& syntax, const Scope& parent);

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::GenerateBlockArray; }
};

/// Represents the entirety of a design, along with all contained compilation units.
class RootSymbol : public Symbol, public Scope {
public:
    span<const ModuleInstanceSymbol* const> topInstances;
    span<const CompilationUnitSymbol* const> compilationUnits;

    explicit RootSymbol(Compilation& compilation) :
        Symbol(SymbolKind::Root, "$root", SourceLocation()), Scope(compilation, this) {}

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::Root; }
};

}