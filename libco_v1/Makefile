all: main

main: main.cpp coro.cpp
	g++-11 -std=c++20 -g -o main main.cpp coro.cpp 

clean:
	rm main

.PHONY: all clean
