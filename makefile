
output.exe: test.cpp
	g++ -Wall --pedantic-errors -Wshadow -std=c++23 $^ -o $@

clean:
	rm -rf output.exe