CC = g++
CFLAGS = -c
VALG = valgrind -v

all:part3 run

part3: part3.o fileSystem.o
	$(CC) part3.o fileSystem.o -o fileSystemOper

part3.o: part3.cpp part3.h
	$(CC) $(CFLAGS) part3.cpp 

fileSystem.o: fileSystem.cpp part3.h
	$(CC) $(CFLAGS) fileSystem.cpp 

run:
	./fileSystemOper fileSystem.data mkdir "/usr"
	./fileSystemOper fileSystem.data mkdir "/usr/ysa"
	./fileSystemOper fileSystem.data mkdir "/bin/ysa"
	./fileSystemOper fileSystem.data write "/usr/ysa/file1" fileSystem.cpp
	./fileSystemOper fileSystem.data write "/usr/file2" Makefile
	./fileSystemOper fileSystem.data write "/file3" Makefile
	./fileSystemOper fileSystem.data dir "/"
	./fileSystemOper fileSystem.data del "/usr/ysa/file1"
	./fileSystemOper fileSystem.data dumpe2fs
	./fileSystemOper fileSystem.data read "/usr/file2" linuxFile2.data
	./fileSystemOper fileSystem.data dir "/usr"
	./fileSystemOper fileSystem.data write "/usr/linkedfile2" Makefile
	./fileSystemOper fileSystem.data dumpe2fs
	./fileSystemOper fileSystem.data dir "/usr"
	./fileSystemOper fileSystem.data del "/usr/file2"
	./fileSystemOper fileSystem.data dir "/usr"
	./fileSystemOper fileSystem.data rmdir "/"
	./fileSystemOper fileSystem.data dumpe2fs
	./fileSystemOper fileSystem.data dir "/"
	

clean:
	rm -rf *o fileSystemOper