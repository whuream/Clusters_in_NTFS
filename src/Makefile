obj = main.o FR.o BootSector.o

CNTFS : $(obj)
	g++ -o CNTFS $(obj)
	
main.o : FR.h BootSector.h
FR.o : FR.h
BootSector.o : BootSector.h

.PHONY : clean
clean : 
	rm $(obj)