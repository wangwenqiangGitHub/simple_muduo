.PHONY: xx

"":
	if [ -d "build" ]; then \
		cd build && cmake ..&& make -j4; \
	else\
		mkdir build;\
		cd build && cmake -DCMAKE_CXX_COMPILER:FILEPATH=${shell which g++} -DCMAKE_C_COMPILER:FILEPATH=${shell which gcc} ..; \
	fi

%:
	if [ -d "build" ]; then \
		cd build && make %@; \
	else\
		mkdir build;\
		cd build && cmake -DCMAKE_CXX_COMPILER:FILEPATH=${shell which g++} -DCMAKE_C_COMPILER:FILEPATH=${shell which gcc} ..; \
	fi
clean:
	cd build && rm -rf *
r:
	cd build && rm -rf * && cmake .. && make

n:
	cd build && cmake -DNinja .. && ninja
