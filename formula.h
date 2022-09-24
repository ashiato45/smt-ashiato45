#include "minisat/simp/SimpSolver.h"
#include <memory>
#include <vector>
#include <sstream>
#include <map>

enum Op{
    Op_Atom,
    Op_And,
    Op_Or,
    Op_Not,
    Op_Count
};

struct Formula;
using FormulaPtr = std::shared_ptr<Formula>;

struct Formula
{
    Op op;
    Minisat::Var atom;
    std::vector<std::shared_ptr<Formula>> terms;

    static FormulaPtr MakeAtom(Minisat::Var atom);
    static FormulaPtr MakeAnd(FormulaPtr f1, FormulaPtr f2);
    static FormulaPtr MakeOr(FormulaPtr f1, FormulaPtr f2);
    static FormulaPtr MakeNot(FormulaPtr f1);

    void AppendAsString(std::ostringstream& oss);
    bool Eval(std::map<Minisat::Var, bool>& assignment);
};

// 適用した後、そのformulaはリテラルになるように変形する。変形したものはsubsにつっこむ。
// rootだけ自分で書き換えができないので注意。また、2回以上のNotもそのまま返している。
Minisat::Var ApplyTseitinHelp(FormulaPtr formula, Minisat::Solver& solver, std::map<Minisat::Var, FormulaPtr>& subs);
// こちらはできあがりをかえす。formulaは破壊される。
FormulaPtr ApplyTseitin(FormulaPtr formula, Minisat::Solver& solver, std::map<Minisat::Var, FormulaPtr>& subs);

// 実行後formulaは破壊されるので注意。
void PutIntoSolver(FormulaPtr formula, Minisat::Solver& solver);