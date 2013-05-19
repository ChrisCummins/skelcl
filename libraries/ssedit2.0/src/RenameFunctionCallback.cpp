#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wc++11-long-long"

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

#include "RenameFunctionCallback.h"

using namespace clang;
using namespace clang::tooling;

namespace ssedit2 {

RenameFunctionCallback::RenameFunctionCallback(
    tooling::Replacements& replacements,
    const std::string& newName)
  : _replacements(replacements), _newName(newName)
{
}

void RenameFunctionCallback::run(
    const ast_matchers::MatchFinder::MatchResult& result)
{
  renameFunctionDeclaration( result.Nodes.getDeclAs<FunctionDecl>("decl"),
                            *result.SourceManager);
  renameCallExpression( result.Nodes.getStmtAs<CallExpr>("call"),
                       *result.SourceManager);
}

void RenameFunctionCallback::renameFunctionDeclaration(
    const FunctionDecl* funcDecl,
    SourceManager& sourceManager)
{
  if (funcDecl) {
    _replacements.insert(Replacement(sourceManager,
                          CharSourceRange::getTokenRange(
                            SourceRange(funcDecl->getLocation())),
                          _newName));
  }
}

void RenameFunctionCallback::renameCallExpression(
    const CallExpr* callExpr,
    SourceManager& sourceManager)
{
  if (callExpr) {
    // travers children
    for ( clang::CallExpr::const_child_iterator
            i  = callExpr->child_begin(),
            e  = callExpr->child_end();
            i != e;
          ++i ) {
      // the name is wrapped in an implicit cast expression
      const ImplicitCastExpr* ice = dyn_cast<ImplicitCastExpr>(*i);
      if (ice) {
        for ( clang::CallExpr::const_child_iterator
                i2  = ice->child_begin(),
                e2  = ice->child_end();
                i2 != e2;
              ++i2 ) {
          const DeclRefExpr* declRefExpr = dyn_cast<DeclRefExpr>(*i2);
          if (declRefExpr) {
            _replacements.insert(Replacement(sourceManager,
                                  declRefExpr,
                                  _newName));
          }
        }
      }
    }
  }
}

} // namespace ssedit2

