#include "minisat/simp/SimpSolver.h"
#include <memory>
#include <vector>
#include <sstream>
#include <map>
#include <string>

enum Op{
    Op_Atom,
    Op_And,
    Op_Or,
    Op_Not,
    Op_Count
};

template <typename T>
struct Formula
{
    Op op;
    Minisat::Var atom = -1;
    std::vector<std::shared_ptr<Formula>> terms;

    static std::shared_ptr<Formula> MakeAtom(Minisat::Var atom);
    static std::shared_ptr<Formula> MakeAnd(std::shared_ptr<Formula> f1, std::shared_ptr<Formula> f2);
    static std::shared_ptr<Formula> MakeOr(std::shared_ptr<Formula> f1, std::shared_ptr<Formula> f2);
    static std::shared_ptr<Formula> MakeNot(std::shared_ptr<Formula> f1);

    void AppendAsString(std::ostringstream& oss);
    std::string ToString();

    bool Eval(std::map<Minisat::Var, bool>& assignment);
};

template
struct Formula<Minisat::Var>;

using FormulaPred = Formula<Minisat::Var>;

// 適用した後、そのformulaはリテラルになるように変形する。変形したものはsubsにつっこむ。
// rootだけ自分で書き換えができないので注意。また、2回以上のNotもそのまま返している。
Minisat::Var ApplyTseitinHelp(std::shared_ptr<FormulaPred> formula, Minisat::Solver& solver, std::map<Minisat::Var, std::shared_ptr<FormulaPred>>& subs);
// こちらはできあがりをかえす。formulaは破壊される。
std::shared_ptr<FormulaPred> ApplyTseitin(std::shared_ptr<FormulaPred> formula, Minisat::Solver& solver, std::map<Minisat::Var, std::shared_ptr<FormulaPred>>& subs);

// 実行後formulaは破壊されるので注意。
void PutIntoSolver(std::shared_ptr<FormulaPred> formula, Minisat::Solver& solver);