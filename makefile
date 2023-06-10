CXX = clang-14
CXXFLAGS = -Wall -Wextra -std=c++17

.PHONY: all clean

all: proact_server

load_library:
	export LD_LIBRARY_PATH="."

proact_server: proact_server.cpp libst_proactor.so
	$(CXX) $(CXXFLAGS) -o proact_server proact_server.cpp -L. -lst_proactor -lstdc++ -ldl -pthread

libst_proactor.so: st_proactor.cpp st_proactor.hpp
	$(CXX) $(CXXFLAGS) -fPIC -shared -o libst_proactor.so st_proactor.cpp

clean:
	rm -f proact_server libst_proactor.so
