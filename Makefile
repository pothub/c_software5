OBJ 	= main.o scan.o pripri.o cross.o compiler.o
PROGRAM	= out
FILE = sample/sample11pp.mpl

#all : $(PROGRAM)

.PHONY: $(PROGRAM)
$(PROGRAM) : $(OBJ)
	gcc -o $(PROGRAM) $(OBJ)
	@ touch result.txt
	@ echo "\n----------program start!----------"
	@ ./$(PROGRAM) $(FILE)
#@ ./$(PROGRAM) $(FILE) > $(FILE).txt
	@ ./$(PROGRAM) $(FILE) > result.txt
	@ echo "-----------program end!-----------"

make :
	./out
clean :
	rm -f $(OBJ)

