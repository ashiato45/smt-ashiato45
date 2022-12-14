#include "formula.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <iostream>

void AddingLog(std::string s)
{
    // std::cout << "Adding: " << s << std::endl;
}

FormulaPtr Formula::MakeAtom(Minisat::Var atom)
{
    return FormulaPtr{new Formula{Op::Op_Atom, atom, {}}};
}
FormulaPtr Formula::MakeAnd(FormulaPtr f1, FormulaPtr f2)
{
    return FormulaPtr{new Formula{Op::Op_And, {}, {f1, f2}}};
}
FormulaPtr Formula::MakeOr(FormulaPtr f1, FormulaPtr f2)
{
    return FormulaPtr{new Formula{Op::Op_Or, {}, {f1, f2}}};
}
FormulaPtr Formula::MakeNot(FormulaPtr f1)
{
    return FormulaPtr{new Formula{Op::Op_Not, {}, {f1}}};
}

void Formula::AppendAsString(std::ostringstream &oss)
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
        assert(0);
    }
}

std::string Formula::ToString()
{
    std::ostringstream oss;
    AppendAsString(oss);
    return oss.str();
}

bool Formula::Eval(std::map<Minisat::Var, bool> &assignment)
{
    switch (op)
    {
    case Op::Op_And:
    {
        return std::accumulate(terms.begin(), terms.end(), true,
                               [&assignment](bool l, FormulaPtr r)
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
                               [&assignment](bool l, FormulaPtr r)
                               {
                                   return l || r->Eval(assignment);
                               });
    }
    break;
    default:
        assert(0);
    }
}

Minisat::Var ApplyTseitinHelp(FormulaPtr formula, Minisat::Solver &solver,
                              std::map<Minisat::Var, FormulaPtr> &subs)
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
        // ???????????????????????????????????????????????????????????????????????????
        for (int i = 0; i < formula->terms.size(); i++)
        {
            auto replacedVar =
                ApplyTseitinHelp(formula->terms[i], solver, subs);
            if (replacedVar >= 0)
            {
                formula->terms[i] = Formula::MakeAtom(replacedVar);
            }
        }
        // ??????And??????????????????1???????????????
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
                formula->terms[0] = Formula::MakeAtom(var);
            }
            return -1;
        }
        break;
        case Op::Op_Atom:
        {
            // Not Atom?????????????????????
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

FormulaPtr RemoveDoubleNegationHelp(FormulaPtr formula)
{
    if (formula->op == Op::Op_Not)
    {
        assert(formula->terms.size() == 1);
        auto term = formula->terms[0];
        if (term->op == Op::Op_Not)
        {
            // ????????????
            assert(term->terms.size() == 1);
            return RemoveDoubleNegationHelp(term->terms[0]);
        }
    }
    return formula;
}

FormulaPtr RemoveDoubleNegation(FormulaPtr formula)
{
    for (int i = 0; i < formula->terms.size(); i++)
    {
        formula->terms[i] = RemoveDoubleNegationHelp(formula->terms[i]);
    }
    return formula;
}

FormulaPtr ApplyTseitin(FormulaPtr formula, Minisat::Solver &solver,
                        std::map<Minisat::Var, FormulaPtr> &subs)
{
    auto lastVar = ApplyTseitinHelp(formula, solver, subs);
    auto res = (lastVar >= 0) ? Formula::MakeAtom(lastVar) : formula;
    res = RemoveDoubleNegation(res);
    for (auto i : subs)
    {
        i.second = RemoveDoubleNegation(i.second);
    }

    return res;
}

// namespace {
// struct HelpTseitinData {
//     FormulaPtr freshPos;
//     FormulaPtr freshNeg;
//     std::vector<FormulaPtr> insidePos;
//     std::vector<FormulaPtr> insideNeg;
// };

// HelpTseitinData HelpTseitin(FormulaPtr f, Minisat::Solver& solver) {
//     assert(f->op == Op::Op_And || f->op == Op::Op_Or);
//     auto freshPos = Formula::MakeAtom(solver.newVar());
//     auto freshNeg = Formula::MakeNot(freshPos);
//     std::vector<FormulaPtr> insidePos(f->terms.size());
//     std::vector<FormulaPtr> insideNeg(f->terms.size());
//     std::transform(f->terms.begin(), f->terms.end(), insidePos.begin(),
//                    [&solver](FormulaPtr i) { return ApplyTseitin(i, solver);
//                    });
//     std::transform(f->terms.begin(), f->terms.end(), insideNeg.begin(),
//                    [&solver](FormulaPtr i) {
//                        return ApplyTseitin(Formula::MakeNot(i), solver);
//                    });

//     return HelpTseitinData{freshPos, freshNeg, insidePos, insideNeg};
// }
// }  // namespace

Minisat::Lit Formula2MinisatLit(FormulaPtr formula)
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

void AddSubstitutionToSolver(Minisat::Var freshVar, FormulaPtr formula,
                             Minisat::Solver &solver)
{
    switch (formula->op)
    {
    case Op::Op_And:
    {
        // (a???b)??????fresh variable c????????????(c ???
        // (c<->a???b))?????????????????????(c->a???b ??? a???b->c)???????????????(??c???(a???b) ???
        // (??(a???b)???c))??????
        // ???????????????(??c???a ??? ??c???b ??? ??a?????b???c)??????????????????????????????(c ??? ??c???a ???
        // ??c???b ??? ??a?????b???c)???????????????????????????????????????????????????

        // c??????
        // AddingLog(Formula::MakeAtom(freshVar)->ToString());
        // solver.addClause(Minisat::Lit{freshVar});

        // ??c???a ?????c???b
        // TODO: ??????a???neg?????????????????????????????????????????????
        for (auto i : formula->terms)
        {
            AddingLog(Formula::MakeOr(Formula::MakeNot(Formula::MakeAtom(freshVar)), i)->ToString());
            solver.addClause(~Minisat::mkLit(freshVar),
                             Formula2MinisatLit(i));
        }

        // ??a?????b???c ??????
        AddingLog("[start] ??a?????b???c part");
        Minisat::vec<Minisat::Lit> lits;
        lits.push(Minisat::mkLit(freshVar));
        AddingLog(Formula::MakeAtom(freshVar)->ToString());
        for (auto i : formula->terms)
        {
            lits.push(~Formula2MinisatLit(i));
            AddingLog(Formula::MakeNot(i)->ToString());
        }
        solver.addClause(lits);
        AddingLog("[end] ??a?????b???c part");
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
        // (a???b)??????fresh variable c????????????(c ???
        // (c<->a???b))?????????????????????(c->a???b ??? a???b->c)???????????????(??c???a???b ???
        // (??a?????b)???c))??????
        // ???????????????(??c???a???b ??? ??a???c ???
        // ??b???c)???????????????????????????????????????????????????

        // c??????
        // AddingLog(Formula::MakeAtom(freshVar)->ToString());
        // solver.addClause(Minisat::Lit{freshVar});

        // ??a???c ??? ??b???c ??????
        for (auto i : formula->terms)
        {
            AddingLog(Formula::MakeOr((Formula::MakeAtom(freshVar)), Formula::MakeNot(i))->ToString());
            solver.addClause(Minisat::mkLit(freshVar),
                             ~Formula2MinisatLit(i));
        }

        // ??c???a???b ??????
        AddingLog("[start] ??c???a???b part");

        Minisat::vec<Minisat::Lit> lits;
        lits.push(~Minisat::mkLit(freshVar));
        AddingLog(Formula::MakeNot(Formula::MakeAtom(freshVar))->ToString());
        for (auto i : formula->terms)
        {
            lits.push(Formula2MinisatLit(i));
            AddingLog(i->ToString());
        }
        solver.addClause(lits);
        AddingLog("[end] ??c???a???b part");
    }
    break;
    default:
        assert(0);
    }
}

void PutIntoSolver(FormulaPtr formula, Minisat::Solver &solver)
{
    std::map<Minisat::Var, FormulaPtr> subs;
    auto transformed = ApplyTseitin(formula, solver, subs);

    // formula???????????????????????????????????????????????????
    AddingLog(transformed->ToString());
    solver.addClause(Formula2MinisatLit(transformed));

    // ?????????????????????
    for (auto i : subs)
    {
        AddSubstitutionToSolver(i.first, i.second, solver);
    }
    // solver.addClause(Minisat::Lit<)
}