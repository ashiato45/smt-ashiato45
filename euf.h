#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <concepts>
#include <ranges>


#include "gtest/gtest.h"

#include "formula.h"

class EufTerm;

class EufSymbol{
    public:
    std::string name;
    int numOp = 0;  // 0は定数

    static EufSymbol MakeAtom(std::string name);
    static EufSymbol MakeFunction(std::string name, int operand);

    EufTerm Apply2(EufTerm x, EufTerm y);

    bool operator==(const EufSymbol& rhs) const{
        return this->name == rhs.name && this->numOp == rhs.numOp;
    }

    bool operator!=(const EufSymbol& rhs) const{
        return !(*this == rhs);
    }
};

class EufTerm{
    public:
    EufSymbol symbol;
    std::vector<EufTerm> args;

    EufTerm(): symbol(EufSymbol::MakeAtom("")), args(0){};

    explicit EufTerm(EufSymbol x): 
    symbol(x)
    {
        assert(x.numOp == 0);
    }

    EufTerm(EufSymbol f, std::vector<EufTerm> args):
    symbol(f),
    args(args)
    {
    }

    std::string Print() const;

    bool operator==(const EufTerm& rhs) const{
        if(this->args.size() != rhs.args.size()){
            return false;
        }
        if(this->symbol != rhs.symbol){
            return false;
        }
        for(int i=0; i < args.size(); i++){
            if(args[i] != rhs.args[i]){
                return false;
            }
        }

        return true;
    }

    bool operator!=(const EufTerm& rhs) const{
        return !(*this == rhs);
    }
};


template<> struct std::hash<EufTerm>{
    std::size_t operator()(EufTerm const& s) const noexcept
    {
        return std::hash<std::string>{}(s.Print());
    }
};


namespace boost
{
    std::size_t hash_value(const EufTerm& et);
}

struct EufPoolNode{
    std::shared_ptr<EufTerm> term;
    std::vector<std::shared_ptr<EufPoolNode>> children;
    std::shared_ptr<EufPoolNode> unionArrow;
    std::unordered_set<EufTerm> parents;
    int unionRank = 0;
};


// term -> p(term)をつくって、p(term)→poolnodeをつくるべきだったのか
class EufPool{
    public:
    std::unordered_map<EufTerm, std::shared_ptr<EufTerm>> terms;
    std::unordered_map<std::shared_ptr<EufTerm>, std::shared_ptr<EufPoolNode>> nodes;

    std::shared_ptr<EufPoolNode> Add(const EufTerm& term);
    void AddEquality(const EufTerm& left, const EufTerm& right);
    bool Equals(const EufTerm& left, const EufTerm& right);
    void Merge(const EufTerm& left, const EufTerm& right);
    void Merge(std::shared_ptr<EufPoolNode> left, std::shared_ptr<EufPoolNode> right);

    friend std::ostream& operator<<(std::ostream& ostr, EufPool pool){
        ostr << "digraph graphname{" << std::endl;
        // ノードをつくる
        for(auto& i: pool.nodes){
            auto& node = i.second;
            int n = 1;
            for(auto& j: node->children){
                ostr << "\"" << node->term->Print() << "\" -> \"" << j->term->Print() << "\" [label=" << n << "];" << std::endl;
                n++;
            }

            if(node->unionArrow != node){
                ostr << "\"" << node->term->Print() << "\" -> \"" << node->unionArrow->term->Print() << "\" [style=dotted];" << std::endl;
            }
        }
        ostr << "}";

        return ostr;
    }

    std::unordered_set<std::shared_ptr<EufPoolNode>> CalcPredecessor(std::shared_ptr<EufPoolNode> node);

    FRIEND_TEST(EufTest, UnionFind);
    private:
    std::shared_ptr<EufPoolNode> FindRoot(std::shared_ptr<EufPoolNode> node);
    void Union(std::shared_ptr<EufPoolNode> a, std::shared_ptr<EufPoolNode> b);
    bool IsSame(std::shared_ptr<EufPoolNode> a, std::shared_ptr<EufPoolNode> b);
    bool Congruent(std::shared_ptr<EufPoolNode> a, std::shared_ptr<EufPoolNode> b);
};

struct EufAtom{
    bool equality = true;  // trueのとき等式
    EufTerm left;
    EufTerm right;

    bool operator==(const EufAtom& rhs) const{
        return equality == rhs.equality && left == rhs.left && right == rhs.right;
    }

    EufAtom operator!() const{
        return {!equality, left, right};
    }
};


template<> struct std::hash<EufAtom>{
    std::size_t operator()(EufAtom const& a) const noexcept;
};

template <typename T>
concept EufAtomContainer = std::ranges::input_range<T> && std::same_as<std::ranges::range_value_t<T>, EufAtom>;

template <typename T>
concept EufAtomPtrContainer = std::ranges::input_range<T> && std::same_as<std::ranges::range_value_t<T>, std::shared_ptr<EufAtom>>;

template<EufAtomContainer Container>
std::pair<bool, EufPool> IsSatisfiable(const Container& atoms){
    // return false;
    EufPool pool;
    std::vector<EufAtom> unsats;

    // 先に全部のtermを追加しておかないと伝播がうまくいかない
    for(const EufAtom& atom: atoms){
        pool.Add(atom.left);
        pool.Add(atom.right);
    }


    for(const EufAtom& atom: atoms){
        auto nLeft = pool.Add(atom.left);
        auto nRight = pool.Add(atom.right);
        if(atom.equality){
            pool.Merge(nLeft, nRight);
        }else{
            unsats.push_back(atom);
        }
    }

    for(auto& i: unsats){
        if(pool.Equals(i.left, i.right)){
            return {false, pool};
        }
    }

    return {true, pool};
}


template
struct Formula<EufAtom>;

using EufFormula = Formula<EufAtom>;

struct EufModel{
    bool satisfiable;
    std::unordered_map<EufAtom, bool> assignment;

    EufModel operator!() const{
        return EufModel{!satisfiable, assignment};
    }
};


template<EufAtomContainer Container>
std::pair<std::unordered_map<EufAtom, PropAtom>, std::unordered_map<PropAtom, EufAtom>> 
MakeEufAtomDictionary(Container& atoms, Minisat::Solver& solver){
    std::unordered_map<EufAtom, PropAtom> euf2prop;
    std::unordered_map<PropAtom, EufAtom> prop2euf;

    std::unordered_set<EufAtom> temp;
    for(auto& atom: atoms){
        temp.insert(atom);
    }

    for(auto& atom: temp){
        auto var = solver.newVar();
                std::cout << "atomdict: added one dict" << var << std::endl;
        euf2prop.insert({atom, var});
        // euf2prop[temp] = var;
        // prop2euf[var] = temp;
        prop2euf.insert({var, atom});

        assert(euf2prop[prop2euf[var]] == var);
        assert(prop2euf[euf2prop[atom]] == atom);
    }
    return {euf2prop, prop2euf};
}


// template<EufAtomPtrContainer Container>
// std::pair<std::unordered_map<std::shared_ptr<EufAtom>, PropAtom>, std::unordered_map<PropAtom, std::shared_ptr<EufAtom>>> 
// MakeEufAtomPtrDictionary(Container formulae, Minisat::SimpSolver& solver){
//     std::unordered_map<std::shared_ptr<EufAtom>, PropAtom> euf2prop;
//     std::unordered_map<PropAtom, std::shared_ptr<EufAtom>> prop2euf;

//     std::unordered_set<std::shared_ptr<EufAtom>> temp;
//     for(auto& i: formulae){
//         temp.insert(i);
//     }

//     for(auto& i: temp){
//         auto var = solver.newVar();
//         euf2prop.insert({i, var});
//         // euf2prop[temp] = var;
//         // prop2euf[var] = temp;
//         prop2euf.insert({var, i});
//     }
//     return {euf2prop, prop2euf};
// }

EufModel EufSolveNaive(EufFormula& formula);
EufModel EufSolve(EufFormula& formula);

// class EufTermTree{
//     public:

// };

// termからそれに対応するノードを引くにひは？
// 文字列のhashをつかおう。違う引数の同じシンボルが2度はでないようにする。

    // EufTermTree pool;
    // auto f = pool.MakeEufFunc("f", 2);
    // auto a = pool.MakeEufAtom("a");
    // auto b = pool.MakeEufAtom("b");
    // auto term1 = MakeEufEq(f.apply2(a, b), a);
    // // auto term2 = MakeEufNeq(f.apply2(f.apply2(a, b)), b);
    // pool.ApplyClosure({term1});