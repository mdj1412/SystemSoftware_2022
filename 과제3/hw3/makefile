hw3: init.o thread.o  scheduler.o testcase.o semaphore.o
	gcc -o hw3 init.o thread.o scheduler.o testcase.o semaphore.o -lpthread
	
init.o: init.h init.c
	gcc -c init.c -lpthread

thread.o: thread.h thread.c
	gcc -c thread.c -lpthread

scheduler.o: scheduler.h scheduler.c
	gcc -c scheduler.c -lpthread

testcase.o: testcase.c
	gcc -c testcase.c -lpthread
	
semaphore.o: semaphore.h semaphore.c
	gcc -c semaphore.c -lpthread

clean:
	rm -f hw3 init.o thread.o scheduler.o testcase.o semaphore.o
