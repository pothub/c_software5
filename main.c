#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include "scan.h"
#include "pripri.h"
#include "cross.h"

int main(int nc, char *np[]) {

	if(nc != 2) {
		printf("File name is not specified.\n");
		return -1;
	}

	if(init_scan(np[1]) < 0) {
		printf("File %s can not open.\n", np[1]);
		return -1;
	}

	if(initCompiler(np[1]) < 0){
		printf("Can't create csl file.\n");
		return -1;
	}

	init_cross();
	if(prase() == 0){
		PrintIdList(globalidroot->nextp);
		RemoveGrobalId();
	}
	return 0;
}
