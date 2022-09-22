

mysmt: main.cpp
	make -C minisat
	g++ -o mysmt -I minisat main.cpp minisat/build/release/lib/libminisat.a

clean:
	rm mysmt