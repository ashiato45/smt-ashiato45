

mysmt: main.cpp formula.cpp
	make -C minisat
	g++ -std=c++20 --debug -o mysmt -I minisat main.cpp formula.cpp minisat/build/release/lib/libminisat.a
	
runtest: test.cpp formula.cpp euf.cpp googletest/build/lib/libgtest.a euf.h formula.h
	make -C minisat
	g++ -std=c++20 --debug -o runtest -pthread  -I minisat -I googletest/googletest/include test.cpp formula.cpp -I boost euf.cpp googletest/build/lib/libgtest.a  minisat/build/release/lib/libminisat.a

googletest/build/lib/libgtest.a: 
	cd googletest && mkdir build && cd build && cmake .. -DBUILD_GMOCK=OFF -DGTEST_HAS_PTHREAD=0 && make

clean:
	-rm -r minisat/build
	-rm runtest
	-rm mysmt
	-rm -r googletest/build