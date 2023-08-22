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

struct Atom{
    bool equality = true;  // trueのとき等式
    EufTerm left;
    EufTerm right;
};


template <typename T>
concept AtomContainer = std::ranges::input_range<T> && std::same_as<std::ranges::range_value_t<T>, Atom>;

template<AtomContainer Container>
bool IsSatisfiable(const Container& atoms){
    // return false;
    EufPool pool;
    std::vector<Atom> unsats;

    for(const Atom& atom: atoms){
        if(atom.equality){
            auto nLeft = pool.Add(atom.left);
            auto nRight = pool.Add(atom.right);
            pool.Merge(nLeft, nRight);
        }else{
            unsats.push_back(atom);
        }
    }

    for(auto i&: unsats){
        if(pool.IsSame(i.left, i.right)){
            return false;
        }
    }
    
    return true;
}

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