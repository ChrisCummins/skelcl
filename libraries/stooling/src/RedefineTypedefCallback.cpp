#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-promo"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wshift-sign-overflow"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wduplicate-enum"

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>

#pragma GCC diagnostic pop

#include <string>

#include "RedefineTypedefCallback.h"

using namespace clang;
using namespace clang::tooling;

namespace stooling {

RedefineTypedefCallback::RedefineTypedefCallback(
    tooling::Replacements& replacements,
    const std::string& typedefName,
    const std::string& newType)
  : _replacements(replacements), _typedefName(typedefName), _newType(newType)
{
}

void RedefineTypedefCallback::run(
    const ast_matchers::MatchFinder::MatchResult& result)
{
  const TypedefNameDecl* typedefDecl
    = result.Nodes.getDeclAs<TypedefNameDecl>("decl");
  if (typedefDecl && typedefDecl->getName() == _typedefName) {
    _replacements.insert(Replacement(*result.SourceManager, typedefDecl,
          "typedef "
          + _newType
          + " "
          + _typedefName));
  }
}

} // namespace stooling

