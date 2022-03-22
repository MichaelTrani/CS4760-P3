LIBS =  -pthread

all: master slave

master: master.o
	g++ master.o -o master $(LIBS)
slave: slave.o
	g++ slave.o -o slave $(LIBS)
master.o: master.cpp
	g++ -c master.cpp
slave.o: slave.cpp
	g++ -c slave.cpp
clean:
	find . -type f ! -iname "*.cpp" ! -iname "makefile" ! -iname "README" ! -iname "*.h" -delete
