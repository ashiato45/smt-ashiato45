

mysmt: main.cpp
	g++ -o mysmt -I minisat main.cpp  -Wliteral-suffix

clean:
	rm mysmt