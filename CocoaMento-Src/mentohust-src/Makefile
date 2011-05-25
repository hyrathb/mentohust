mentohust: mentohust.o myini.o md5.o mycheck.o dlfunc.o myfunc.o mystate.o myconfig.o 
	llvm-gcc -o $@ $^ -liconv
mentohust.o: mentohust.c myconfig.h mystate.h myfunc.h dlfunc.h 
	llvm-gcc -c $<
dlfunc.o: dlfunc.c 
	llvm-gcc -c $<
myfunc.o: myfunc.c  md5.h mycheck.h
	llvm-gcc -c $< 
..c.o:
	llvm-gcc -c $< 
