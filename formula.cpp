#include "formula.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <iostream>

void AddingLog(std::string s)
{
    // std::cout << "Adding: " << s << std::endl;
}

template<>
void PropFormula::AppendAsString(std::ostringstream &oss) const
{
    switch (op)
    {
    case Op::Op_And:
    {
        oss << "And[";
        for (int i = 0; i < terms.size(); i++)
        {
            terms[i]->AppendAsString(oss);
            if (i != terms.size() - 1)
            {
                oss << ", ";
            }
        }
        oss << "]";
    }
    break;
    case Op::Op_Atom:
    {
        assert(terms.size() == 0);
        oss << static_cast<char>('a' + atom);
    }
    break;
    case Op::Op_Not:
    {
        assert(terms.size() == 1);
        oss << "Not[";
        terms[0]->AppendAsString(oss);
        oss << "]";
    }
    break;
    case Op::Op_Or:
    {
        oss << "Or[";
        for (int i = 0; i < terms.size(); i++)
        {
            terms[i]->AppendAsString(oss);
            if (i != terms.size() - 1)
            {
                oss << ", ";
            }
        }
        oss << "]";
    }
    break;
    default:
        std::cout << op << std::endl;
        assert(0);
    }
}



template<>
bool PropFormula::Eval(std::map<Minisat::Var, bool> &assignment)
{
    switch (op)
    {
    case Op::Op_And:
    {
        return std::accumulate(terms.begin(), terms.end(), true,
                               [&assignment](bool l, std::shared_ptr<PropFormula> r)
                               {
                                   return l && r->Eval(assignment);
                               });
    }
    break;
    case Op::Op_Atom:
    {
        assert(terms.size() == 0);
        return assignment[atom];
    }
    break;
    case Op::Op_Not:
    {
        assert(terms.size() == 1);
        return !terms[0]->Eval(assignment);
    }
    break;
    case Op::Op_Or:
    {
        return std::accumulate(terms.begin(), terms.end(), false,
                               [&assignment](bool l, std::shared_ptr<PropFormula> r)
                               {
                                   return l || r->Eval(assignment);
                               });
    }
    break;
    default:
        assert(0);
    }
}

Minisat::Var ApplyTseitinHelp(std::shared_ptr<PropFormula> formula, Minisat::Solver &solver,
                              std::map<Minisat::Var, std::shared_ptr<PropFormula>> &subs)
{
    switch (formula->op)
    {
    case Op::Op_Atom:
    {
        return -1;
    }
    break;
    case Op::Op_And:
    case Op::Op_Or:
    {
        // まず子どもたちに適用して子どもたちをリテラルにする
        for (int i = 0; i < formula->terms.size(); i++)
        {
            auto replacedVar =
                ApplyTseitinHelp(formula->terms[i], solver, subs);
            if (replacedVar >= 0)
            {
                formula->terms[i] = PropFormula::MakeAtom(replacedVar);
            }
        }
        // このAnd節を置換して1文字にする
        auto newVar = solver.newVar();
        subs[newVar] = formula;
        return newVar;
    }
    break;
    case Op::Op_Not:
    {
        assert(formula->terms.size() == 1);
        auto term = formula->terms[0];
        switch (term->op)
        {
        case Op::Op_And:
        case Op::Op_Or:
        {
            auto var = ApplyTseitinHelp(term, solver, subs);
            if (var >= 0)
            {
                formula->terms[0] = PropFormula::MakeAtom(var);
            }
            return -1;
        }
        break;
        case Op::Op_Atom:
        {
            // Not Atomはプリミティブ
            return -1;
        }
        break;
        case Op::Op_Not:
        {
            assert(formula->terms.size() == 1);
            assert(ApplyTseitinHelp(term, solver, subs) == -1);
            return -1;
        }
        break;
        default:
        {
            assert(0);
        }
        }
    }
    break;
    default:
        assert(0);
        break;
    }
}

std::shared_ptr<PropFormula> RemoveDoubleNegationHelp(std::shared_ptr<PropFormula> formula)
{
    if (formula->op == Op::Op_Not)
    {
        assert(formula->terms.size() == 1);
        auto term = formula->terms[0];
        if (term->op == Op::Op_Not)
        {
            // 二重否定
            assert(term->terms.size() == 1);
            return RemoveDoubleNegationHelp(term->terms[0]);
        }
    }
    return formula;
}

std::shared_ptr<PropFormula> RemoveDoubleNegation(std::shared_ptr<PropFormula> formula)
{
    for (int i = 0; i < formula->terms.size(); i++)
    {
        formula->terms[i] = RemoveDoubleNegationHelp(formula->terms[i]);
    }
    return formula;
}

std::shared_ptr<PropFormula> ApplyTseitin(std::shared_ptr<PropFormula> formula, Minisat::Solver &solver,
                        std::map<Minisat::Var, std::shared_ptr<PropFormula>> &subs)
{
    auto lastVar = ApplyTseitinHelp(formula, solver, subs);
    auto res = (lastVar >= 0) ? PropFormula::MakeAtom(lastVar) : formula;
    res = RemoveDoubleNegation(res);
    for (auto i : subs)
    {
        i.second = RemoveDoubleNegation(i.second);
    }

    return res;
}

// namespace {
// struct HelpTseitinData {
//     std::shared_ptr<PropFormula> freshPos;
//     std::shared_ptr<PropFormula> freshNeg;
//     std::vector<std::shared_ptr<PropFormula>> insidePos;
//     std::vector<std::shared_ptr<PropFormula>> insideNeg;
// };

// HelpTseitinData HelpTseitin(std::shared_ptr<PropFormula> f, Minisat::Solver& solver) {
//     assert(f->op == Op::Op_And || f->op == Op::Op_Or);
//     auto freshPos = PropFormula::MakeAtom(solver.newVar());
//     auto freshNeg = PropFormula::MakeNot(freshPos);
//     std::vector<std::shared_ptr<PropFormula>> insidePos(f->terms.size());
//     std::vector<std::shared_ptr<PropFormula>> insideNeg(f->terms.size());
//     std::transform(f->terms.begin(), f->terms.end(), insidePos.begin(),
//                    [&solver](std::shared_ptr<PropFormula> i) { return ApplyTseitin(i, solver);
//                    });
//     std::transform(f->terms.begin(), f->terms.end(), insideNeg.begin(),
//                    [&solver](std::shared_ptr<PropFormula> i) {
//                        return ApplyTseitin(PropFormula::MakeNot(i), solver);
//                    });

//     return HelpTseitinData{freshPos, freshNeg, insidePos, insideNeg};
// }
// }  // namespace

Minisat::Lit PropFormula2MinisatLit(std::shared_ptr<PropFormula> formula)
{
    switch (formula->op)
    {
    case Op::Op_Atom:
    {
        auto a = formula->atom;
        assert(a >= 0);
        char hoge[100];
        hoge[0] = a + 'a';
        hoge[1] = 0;
        return Minisat::mkLit(a);
    }
    break;
    case Op::Op_Not:
    {
        assert(formula->terms.size() == 1);
        auto a = formula->terms[0]->atom;
        assert(a >= 0);
        char hoge[100];
        hoge[0] = a + 'a';
        hoge[1] = 0;

        return ~Minisat::mkLit(a);
    }
    break;
    default:
        assert(0);
    }
}

void AddSubstitutionToSolver(Minisat::Var freshVar, std::shared_ptr<PropFormula> formula,
                             Minisat::Solver &solver)
{
    switch (formula->op)
    {
    case Op::Op_And:
    {
        // (a∧b)を、fresh variable cをつかい(c ∧
        // (c<->a∧b))にする。後半は(c->a∧b ∧ a∧b->c)で、さらに(¬c∨(a∧b) ∧
        // (¬(a∧b)∨c))で、
        // 分配すると(¬c∨a ∧ ¬c∨b ∧ ¬a∨¬b∨c)になる。まとめると、(c ∧ ¬c∨a ∧
        // ¬c∨b ∧ ¬a∨¬b∨c)になる。これを複数個に一般化する。

        // c部分
        // AddingLog(PropFormula::MakeAtom(freshVar)->ToString());
        // solver.addClause(Minisat::Lit{freshVar});

        // ¬c∨a ∧¬c∨b
        // TODO: まずaがnegかもしれんというのを忘れてない
        for (auto i : formula->terms)
        {
            AddingLog(PropFormula::MakeOr(PropFormula::MakeNot(PropFormula::MakeAtom(freshVar)), i)->ToString());
            solver.addClause(~Minisat::mkLit(freshVar),
                             PropFormula2MinisatLit(i));
        }

        // ¬a∨¬b∨c 部分
        AddingLog("[start] ¬a∨¬b∨c part");
        Minisat::vec<Minisat::Lit> lits;
        lits.push(Minisat::mkLit(freshVar));
        AddingLog(PropFormula::MakeAtom(freshVar)->ToString());
        for (auto i : formula->terms)
        {
            lits.push(~PropFormula2MinisatLit(i));
            AddingLog(PropFormula::MakeNot(i)->ToString());
        }
        solver.addClause(lits);
        AddingLog("[end] ¬a∨¬b∨c part");
    }
    break;
    case Op::Op_Atom:
    {
        assert(0);
    }
    break;
    case Op::Op_Not:
    {
        assert(0);
        solver.addClause(Minisat::mkLit(freshVar));
    }
    break;
    case Op::Op_Or:
    {
        // (a∨b)を、fresh variable cをつかい(c ∧
        // (c<->a∨b))にする。後半は(c->a∨b ∧ a∨b->c)で、さらに(¬c∨a∨b ∧
        // (¬a∧¬b)∨c))で、
        // 分配すると(¬c∨a∨b ∧ ¬a∨c ∧
        // ¬b∨c)になる。これを複数個に一般化する。

        // c部分
        // AddingLog(PropFormula::MakeAtom(freshVar)->ToString());
        // solver.addClause(Minisat::Lit{freshVar});

        // ¬a∨c ∧ ¬b∨c 部分
        for (auto i : formula->terms)
        {
            AddingLog(PropFormula::MakeOr((PropFormula::MakeAtom(freshVar)), PropFormula::MakeNot(i))->ToString());
            solver.addClause(Minisat::mkLit(freshVar),
                             ~PropFormula2MinisatLit(i));
        }

        // ¬c∨a∨b 部分
        AddingLog("[start] ¬c∨a∨b part");

        Minisat::vec<Minisat::Lit> lits;
        lits.push(~Minisat::mkLit(freshVar));
        AddingLog(PropFormula::MakeNot(PropFormula::MakeAtom(freshVar))->ToString());
        for (auto i : formula->terms)
        {
            lits.push(PropFormula2MinisatLit(i));
            AddingLog(i->ToString());
        }
        solver.addClause(lits);
        AddingLog("[end] ¬c∨a∨b part");
    }
    break;
    default:
        assert(0);
    }
}

void PutIntoSolver(std::shared_ptr<PropFormula> formula, Minisat::Solver &solver)
{
    std::map<Minisat::Var, std::shared_ptr<PropFormula>> subs;
    auto transformed = ApplyTseitin(formula, solver, subs);

    // formula本体を登録。これはリテラルのはず。
    AddingLog(transformed->ToString());
    solver.addClause(PropFormula2MinisatLit(transformed));

    // 置換部分を登録
    for (auto i : subs)
    {
        AddSubstitutionToSolver(i.first, i.second, solver);
    }
    // solver.addClause(Minisat::Lit<)
}