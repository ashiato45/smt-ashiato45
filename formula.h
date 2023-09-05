#pragma once

#include "minisat/simp/SimpSolver.h"
#include <memory>
#include <vector>
#include <sstream>
#include <map>
#include <string>
#include <functional>
#include <iostream>

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
    T atom;
    std::vector<std::shared_ptr<Formula>> terms;

    static std::shared_ptr<Formula> MakeAtom(T atom);
    static std::shared_ptr<Formula> MakeAnd(std::shared_ptr<Formula> f1, std::shared_ptr<Formula> f2);
    static std::shared_ptr<Formula> MakeOr(std::shared_ptr<Formula> f1, std::shared_ptr<Formula> f2);
    static std::shared_ptr<Formula> MakeNot(std::shared_ptr<Formula> f1);
    void Walk(std::function<void(Formula<T>&)> f);

    void AppendAsString(std::ostringstream& oss);
    std::string ToString();

    bool Eval(std::map<T, bool>& assignment);
};

template <typename T>
std::shared_ptr<Formula<T>> Formula<T>::MakeAtom(T atom)
{
    return std::shared_ptr<Formula<T>>{new Formula<T>{Op::Op_Atom, atom, {}}};
}

template <typename T>
std::shared_ptr<Formula<T>> Formula<T>::MakeAnd(std::shared_ptr<Formula<T>> f1, std::shared_ptr<Formula<T>> f2)
{
    return std::shared_ptr<Formula<T>>{new Formula<T>{Op::Op_And, {}, {f1, f2}}};
}

template <typename T>
std::shared_ptr<Formula<T>> Formula<T>::MakeOr(std::shared_ptr<Formula<T>> f1, std::shared_ptr<Formula<T>> f2)
{
    return std::shared_ptr<Formula<T>>{new Formula<T>{Op::Op_Or, {}, {f1, f2}}};
}

template <typename T>
std::shared_ptr<Formula<T>> Formula<T>::MakeNot(std::shared_ptr<Formula<T>> f1)
{
    return std::shared_ptr<Formula<T>>{new Formula<T>{Op::Op_Not, {}, {f1}}};
}

template <typename T>
std::string Formula<T>::ToString()
{
    std::ostringstream oss;
    AppendAsString(oss);
    return oss.str();
}

template <typename T>
void Formula<T>::Walk(std::function<void(Formula<T>&)> f){
    f(*this);
    switch(this->op){
        case Op_Atom:
        break;
        case Op_And:
        case Op_Or:
        {
            assert(this->terms.size() == 2);
            this->terms.at(0)->Walk(f);
            this->terms.at(1)->Walk(f);
        }
        break;
        case Op_Not:
        {
            this->terms[0]->Walk(f);
        }
        break;
        default:
        {
            std::cout << this->ToString() << std::endl;
            assert(0);
        }
        
    }
}


using PropAtom = Minisat::Var;

template
struct Formula<Minisat::Var>;

using PropFormula = Formula<Minisat::Var>;  // TODO: Predじゃないのであとでなんとかする

// 適用した後、そのformulaはリテラルになるように変形する。変形したものはsubsにつっこむ。
// rootだけ自分で書き換えができないので注意。また、2回以上のNotもそのまま返している。
Minisat::Var ApplyTseitinHelp(std::shared_ptr<PropFormula> formula, Minisat::Solver& solver, std::map<Minisat::Var, std::shared_ptr<PropFormula>>& subs);
// こちらはできあがりをかえす。formulaは破壊される。
std::shared_ptr<PropFormula> ApplyTseitin(std::shared_ptr<PropFormula> formula, Minisat::Solver& solver, std::map<Minisat::Var, std::shared_ptr<PropFormula>>& subs);

// 実行後formulaは破壊されるので注意。
void PutIntoSolver(std::shared_ptr<PropFormula> formula, Minisat::Solver& solver);