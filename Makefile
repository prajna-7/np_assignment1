# Compiler and Preprocessor Flags
CC_FLAGS= -Wall -I.
# Linker Flags
LD_FLAGS= -Wall -L./ 

#CXX= g++

# Targets
all: libcalc test client server

# Compile server main file
servermain.o: servermain.cpp
	$(CXX)  $(CC_FLAGS) $(CFLAGS) -c servermain.cpp 

# Compile client main file
clientmain.o: clientmain.cpp
	$(CXX) $(CC_FLAGS) $(CFLAGS) -c clientmain.cpp 
	
# Compile main test file
main.o: main.cpp
	$(CXX) $(CC_FLAGS) $(CFLAGS) -c main.cpp 


# Link test executable
test: main.o calcLib.o
	$(CXX) $(LD_FLAGS) -o test main.o -lcalc


# Link client executable
client: clientmain.o calcLib.o
	$(CXX) $(LD_FLAGS) -o client clientmain.o -lcalc


# Link server executable
server: servermain.o calcLib.o
	$(CXX) $(LD_FLAGS) -o server servermain.o -lcalc

# Compile calcLib object file
calcLib.o: calcLib.c calcLib.h
	gcc -Wall -fPIC -c calcLib.c
	
	
# Create static library
libcalc: calcLib.o
	ar -rc libcalc.a -o calcLib.o

# Clean up generated files
clean:
	rm *.o *.a test server client
