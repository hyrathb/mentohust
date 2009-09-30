CC=gcc
FLAGS=-O2 -pedantic
LIB=../lib
OSRC=../other
TARGET=MentoHUST

all:	$(TARGET)

mentohust.o: mentohust.c
	@$(CC) $(FLAGS) -I$(OSRC) -o $@ -c $<

myconfig.o: myconfig.c
	@$(CC) $(FLAGS) -I$(OSRC) -o $@ -c $<

mystate.o: mystate.c
	@$(CC) $(FLAGS) -I$(OSRC) -o $@ -c $<
	
mydata.o: mydata.c
	@$(CC) $(FLAGS) -I$(OSRC) -o $@ -c $<

function.o: function.c
	@$(CC) $(FLAGS) -I$(OSRC) -o $@ -c $<
	
inireader.o: inireader.c
	@$(CC) $(FLAGS) -o $@ -c $<

md5.o: $(OSRC)/md5.c
	@$(CC) $(FLAGS) -I$(OSRC) -o $@ -c $<

$(TARGET): md5.o inireader.o function.o mydata.o mystate.o myconfig.o mentohust.o
	@echo -e 'Building target: $@'
	@rm -f $(TARGET)
	@if [ ! -f /usr/lib/libpcap.so ]; then sudo ln -s /usr/lib/libpcap.so.0.8 /usr/lib/libpcap.so; fi	#在未安装libpcap-devel的Ubuntu 64位上编译
	@$(CC) $(FLAGS) -o $@ $^ -lpcap #$(LIB)/libpcap.a
	@rm -f $ *.o
	@echo -e '\tFinished building target: $@ :-)\n'

clean:
	rm -f *.o $(TARGET)

rebuilt:
	make clean all
