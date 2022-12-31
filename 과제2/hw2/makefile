hw2: init.o thread.o TestCase1.o TestCase2.o TestCase3.o scheduler.o testcase.o
	gcc -o hw2 init.o thread.o TestCase1.o TestCase2.o TestCase3.o scheduler.o testcase.o -lpthread
	
init.o: init.h init.c
	gcc -c init.c -lpthread

thread.o: thread.h thread.c
	gcc -c thread.c -lpthread

TestCase1.o: TestCase1.h TestCase1.c
	gcc -c TestCase1.c -lpthread

scheduler.o: scheduler.h scheduler.c
	gcc -c scheduler.c -lpthread

testcase.o: testcase.c
	gcc -c testcase.c -lpthread
	
TestCase2.o: TestCase2.h TestCase2.c
	gcc -c TestCase2.c -lpthread
	
TestCase3.o: TestCase3.h TestCase3.c
	gcc -c TestCase3.c -lpthread

clean:
	rm -f hw2 init.o thread.o TestCase1.o TestCase2.o TestCase3.o scheduler.o testcase.o
