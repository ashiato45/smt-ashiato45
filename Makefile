

mysmt: main.cpp
	make -C minisat
	g++ --debug -o mysmt -I minisat main.cpp minisat/build/release/lib/libminisat.a
	
runtest: test.cpp googletest/build/lib/libgtest.a
	g++ --debug -o runtest -pthreadake  -I minisat -I googletest/googletest/include test.cpp googletest/build/lib/libgtest.a

googletest/build/lib/libgtest.a: 
	cd googletest && mkdir build && cd build && cmake .. -DBUILD_GMOCK=OFF -DGTEST_HAS_PTHREAD=0 && make

clean:
	-rm mysmt
	-rm -r googletest/build