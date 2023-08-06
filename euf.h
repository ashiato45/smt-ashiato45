#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

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
    std::vector<std::shared_ptr<EufTerm>> children;
    std::shared_ptr<EufPoolNode> unionArrow;
    std::unordered_set<EufTerm> parents;
};


// term -> p(term)をつくって、p(term)→poolnodeをつくるべきだったのか
class EufPool{
    public:
    std::unordered_map<EufTerm, std::shared_ptr<EufTerm>> terms;
    std::unordered_map<std::shared_ptr<EufTerm>, std::shared_ptr<EufPoolNode>> nodes;

    std::shared_ptr<EufPoolNode> Add(const EufTerm& term);
    void AddEquality(const EufTerm& left, const EufTerm& right);
    bool Equals(const EufTerm& left, const EufTerm& right);

    friend std::ostream& operator<<(std::ostream& ostr, EufPool pool){
        ostr << "digraph graphname{" << std::endl;
        for(auto& i: pool.nodes){
            for(auto& j: i.second->children){
                ostr << "'" << i.first->Print() << "' -> '" << j->Print() << "';" << std::endl;
            }
        }
        ostr << "}";

        return ostr;
    }

    private:
};

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