

mysmt: main.cpp
	make -C minisat
	g++ --debug -o mysmt -I minisat main.cpp minisat/build/release/lib/libminisat.a

clean:
	rm mysmt