#include <iostream>
#include "minisat/simp/SimpSolver.h"

int main(){
    std::cout << "Hello" << std::endl;

    Minisat::SimpSolver solver;
    solver.newVar();
    solver.newVar();
    solver.addClause(Minisat::mkLit(0), ~Minisat::mkLit(1));
    solver.addClause(Minisat::mkLit(0), Minisat::mkLit(1));

    auto ret = solver.solve();
    if (ret){
        printf("SAT\n");
        for (int i = 0; i < solver.nVars(); i++)
            if (solver.model[i] != Minisat::l_Undef)
                printf("%s%s%d", (i==0)?"":" ", (solver.model[i]==Minisat::l_True)?"":"-", i+1);
        printf(" 0\n");
    }else
        printf("UNSAT\n");



    return 0;
}