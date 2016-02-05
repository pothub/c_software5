#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scan.h"
#include "pripri.h"
#include "cross.h"
#include "compiler.h"

// #define Compiler

FILE *csl_fp;
char str_C[MAXSTRSIZE*3];
int label_num=2;

int initCompiler(char *mplname){

	int filename_len = strlen(mplname)+1;
	int index_comma = filename_len;
	char *filename;
	int i;

	for( i=0 ; i<filename_len ; i++ )
		if( mplname[i] == '.') index_comma = i;

	filename = malloc(index_comma+5);
	strcpy(filename,mplname);

	filename[ index_comma+1 ] = 'c';
	filename[ index_comma+2 ] = 's';
	filename[ index_comma+3 ] = 'l';
	filename[ index_comma+4 ] = '\0';

	if ((csl_fp = fopen(filename, "w")) == NULL) return -1;

	return 0;
}

void CompilePrintStack(char *str){
	if(str_C[0] == 0) sprintf(str_C,"%s",str);
	else strcat(str_C,str);
}
void CompileOutputCSL(char *str){
	fprintf(csl_fp,"%s",str);
}
void CompileOutputLine(){
	if(str_C[0] != 0){
#ifdef Compiler
		printf("%s",str_C);
#endif
		fprintf(csl_fp,"%s",str_C);
	}
	str_C[0] = 0;
}
int CompileGetLabel(){
	return label_num++;
}

char StringStack[MAXSTRSIZE*2]={0};
void CompileStack(int l, char *str, int num_string){
	char StringStack_t[MAXSTRSIZE]={0};
	if(num_string)
		sprintf(StringStack_t,"L%04d\tDC\t'%s'\n",l,str);
	else
		sprintf(StringStack_t,"L%04d\tDC\t%s\n",l,str);
	if(StringStack[0] == 0) sprintf(StringStack,"%s",StringStack_t);
	else strcat(StringStack,StringStack_t);
}
void CompileOutputString(){
#ifdef Compiler
	printf("%s",StringStack);
#endif
	fprintf(csl_fp,"%s",StringStack);
}

void CompileDeclareStack(Id_c **id){
	int i;
	char DC_tmp[MAXSTRSIZE];
	for(i=0;i<Head_Name_Num;i++) id = &((*id)->nextp);
	for(i=0;i<Last_Name_Num - Head_Name_Num;i++){
		id = &((*id)->nextp);
		if((*id)->itp->ttype == TPINT || (*id)->itp->ttype == TPCHAR || (*id)->itp->ttype == TPBOOL){
			if((*id)->procname == NULL) sprintf(DC_tmp,"$%s\tDC\t0\n",(*id)->name);
			else sprintf(DC_tmp,"$%s%%%s\tDC\t0\n",(*id)->name,(*id)->procname);
		}else{
			if((*id)->procname == NULL) sprintf(DC_tmp,"$%s\tDS\t%d\n",(*id)->name,(*id)->itp->arraysize);
			else sprintf(DC_tmp,"$%s%%%s\tDS\t%d\n",(*id)->name,(*id)->procname,(*id)->itp->arraysize);
		}
		CompilePrintStack(DC_tmp);
	}
}
void CompileFormalPS(Id_c **id){
	char PS_tmp[MAXSTRSIZE];
	char PS_tmp2[MAXSTRSIZE];
	while((*id)->procname == NULL) id = &((*id)->nextp);

	while(strcmp((*id)->procname,procname_t) && id != NULL){
		id = &((*id)->nextp);
		while((*id)->procname == NULL) id = &((*id)->nextp);
	}
	sprintf(PS_tmp2,"\tPOP\tgr1\n");
	// CompilePrintStack("\tPOP\tgr1\n");
	sprintf(PS_tmp,"\tST\tgr1,\t$%s%%%s\n",(*id)->name,procname_t);
	strcat(PS_tmp2,PS_tmp);
	// CompilePrintStack(PS_tmp);
	while((*id)->nextp != NULL){
		id = &((*id)->nextp);
		if((*id)->ispara == 0) break;
		CompilePrintStack("\tPOP\tgr1\n");
		sprintf(PS_tmp,"\tST\tgr1,\t$%s%%%s\n",(*id)->name,procname_t);
		CompilePrintStack(PS_tmp);
	}
	CompilePrintStack(PS_tmp2);
}




void CompilerLibrary(){
	CompileOutputCSL("EOVF\n  CALL  WRITELINE\n  LAD  gr1, EOVF1\n  LD  gr2, gr0\n  CALL  WRITESTR\n  CALL  WRITELINE\n  SVC  1  ;  overflow error stop\nEOVF1    DC  \'***** Run-Time Error : Overflow *****\'\nE0DIV\n  JNZ  EOVF\n  CALL  WRITELINE\n  LAD  gr1, E0DIV1\n  LD  gr2, gr0\n  CALL  WRITESTR\n  CALL  WRITELINE\n  SVC  2  ;  0-divide error stop\nE0DIV1    DC  \'***** Run-Time Error : Zero-Divide *****\'\nEROV\n  CALL  WRITELINE\n  LAD  gr1, EROV1\n  LD  gr2, gr0\n  CALL  WRITESTR\n  CALL  WRITELINE\n  SVC  3  ;  range-over error stop\nEROV1    DC  \'***** Run-Time Error : Range-Over in Array Index *****\'\nWRITECHAR\n  RPUSH\n  LD  gr6, SPACE\n  LD  gr7, OBUFSIZE\nWC1\n  SUBA  gr2, ONE  ; while(--c > 0) {\n  JZE  WC2\n  JMI  WC2\n  ST  gr6, OBUF,gr7  ;  *p++ = \' \';\n  CALL  BOVFCHECK\n  JUMP  WC1  ; }\nWC2\n  ST  gr1, OBUF,gr7  ; *p++ = gr1;\n  CALL  BOVFCHECK\n  ST  gr7, OBUFSIZE\n  RPOP\n  RET\nWRITESTR\n  RPUSH\n  LD  gr6, gr1  ; p = gr1;\nWS1\n  LD  gr4, 0,gr6  ; while(*p != \'\\0\') {\n  JZE  WS2\n  ADDA  gr6, ONE  ;  p++;\n  SUBA  gr2, ONE  ;  c--;\n  JUMP  WS1  ; }\nWS2\n  LD  gr7, OBUFSIZE  ; q = OBUFSIZE;\n  LD  gr5, SPACE\nWS3\n  SUBA  gr2, ONE  ; while(--c >= 0) {\n  JMI  WS4\n  ST  gr5, OBUF,gr7  ;  *q++ = \' \';\n  CALL  BOVFCHECK\n  JUMP  WS3  ; }\nWS4\n  LD  gr4, 0,gr1  ; while(*gr1 != \'\\0\') {\n  JZE  WS5\n  ST  gr4, OBUF,gr7  ;  *q++ = *gr1++;\n  ADDA  gr1, ONE\n  CALL  BOVFCHECK\n  JUMP  WS4  ; }\nWS5\n  ST  gr7, OBUFSIZE  ; OBUFSIZE = q;\n  RPOP\n  RET\nBOVFCHECK\n    ADDA  gr7, ONE\n    CPA   gr7, BOVFLEVEL\n    JMI  BOVF1\n    CALL  WRITELINE\n    LD gr7, OBUFSIZE\nBOVF1\n    RET\nBOVFLEVEL  DC 256\nWRITEINT\n  RPUSH\n  LD  gr7, gr0  ; flag = 0;\n  CPA  gr1, gr0  ; if(gr1>=0) goto WI1;\n  JPL  WI1\n  JZE  WI1\n  LD  gr4, gr0  ; gr1= - gr1;\n  SUBA  gr4, gr1\n  CPA  gr4, gr1\n  JZE  WI6\n  LD  gr1, gr4\n  LD  gr7, ONE  ; flag = 1;\nWI1\n  LD  gr6, SIX  ; p = INTBUF+6;\n  ST  gr0, INTBUF,gr6  ; *p = \'\\0\';\n  SUBA  gr6, ONE  ; p--;\n  CPA  gr1, gr0  ; if(gr1 == 0)\n  JNZ  WI2\n  LD  gr4, ZERO  ;  *p = \'0\';\n  ST  gr4, INTBUF,gr6\n  JUMP  WI5  ; }\nWI2      ; else {\n  CPA  gr1, gr0  ;  while(gr1 != 0) {\n  JZE  WI3\n  LD  gr5, gr1  ;   gr5 = gr1 - (gr1 / 10) * 10;\n  DIVA  gr1, TEN  ;   gr1 /= 10;\n  LD  gr4, gr1\n  MULA  gr4, TEN\n  SUBA  gr5, gr4\n  ADDA  gr5, ZERO  ;   gr5 += \'0\';\n  ST  gr5, INTBUF,gr6  ;   *p = gr5;\n  SUBA  gr6, ONE  ;   p--;\n  JUMP  WI2  ;  }\nWI3\n  CPA  gr7, gr0  ;  if(flag != 0) {\n  JZE  WI4\n  LD  gr4, MINUS  ;   *p = \'-\';\n  ST  gr4, INTBUF,gr6\n  JUMP  WI5  ;  }\nWI4\n  ADDA  gr6, ONE  ;  else p++;\n    ; }\nWI5\n  LAD  gr1, INTBUF,gr6  ; gr1 = p;\n  CALL  WRITESTR  ; WRITESTR();\n  RPOP\n  RET\nWI6\n  LAD  gr1, MMINT\n  CALL  WRITESTR  ; WRITESTR();\n  RPOP\n  RET\nMMINT    DC  \'-32768\'\nWRITEBOOL\n  RPUSH\n  CPA  gr1, gr0  ; if(gr1 != 0)\n  JZE  WB1\n  LAD  gr1, WBTRUE  ;  gr1 = \"TRUE\";\n  JUMP  WB2\nWB1      ; else\n  LAD  gr1, WBFALSE  ;  gr1 = \"FALSE\";\nWB2\n  CALL  WRITESTR  ; WRITESTR();\n  RPOP\n  RET\nWBTRUE    DC  \'TRUE\'\nWBFALSE    DC  \'FALSE\'\nWRITELINE\n  RPUSH\n  LD  gr7, OBUFSIZE\n  LD  gr6, NEWLINE\n  ST  gr6, OBUF,gr7\n  ADDA  gr7, ONE\n  ST  gr7, OBUFSIZE\n  OUT  OBUF, OBUFSIZE\n  ST  gr0, OBUFSIZE\n  RPOP\n  RET\nFLUSH\n  RPUSH\n  LD gr7, OBUFSIZE\n  JZE FL1\n  CALL WRITELINE\nFL1\n  RPOP\n  RET\nREADCHAR\n  RPUSH\n  LD  gr5, RPBBUF  ; if(RPBBUF != \'\\0\') {\n  JZE  RC0\n  ST  gr5, 0,gr1  ;  *gr1 = RPBBUF;\n  ST  gr0, RPBBUF  ;  RPBBUF = \'\\0\'\n  JUMP  RC3  ;  return; }\nRC0\n  LD  gr7, INP  ; inp = INP;\n  LD  gr6, IBUFSIZE  ; if(IBUFSIZE == 0) {\n  JNZ  RC1\n  IN  IBUF, IBUFSIZE  ;  IN();\n  LD  gr7, gr0  ;  inp = 0;\n    ; }\nRC1\n  CPA  gr7, IBUFSIZE  ; if(inp == IBUFSIZE) {\n  JNZ  RC2\n  LD  gr5, NEWLINE  ;  *gr1 = \'\\n\';\n  ST  gr5, 0,gr1\n  ST  gr0, IBUFSIZE  ;  IBUFSIZE = INP = 0;\n  ST  gr0, INP\n  JUMP  RC3  ; }\nRC2      ; else {\n  LD  gr5, IBUF,gr7  ;  *gr1 = *inp++;\n  ADDA  gr7, ONE\n  ST  gr5, 0,gr1\n  ST  gr7, INP  ;  INP = inp;\nRC3      ; }\n  RPOP\n  RET\nREADINT\n  RPUSH\nRI1      ; do {\n  CALL  READCHAR  ;  ch = READCHAR();\n  LD  gr7, 0,gr1\n  CPA  gr7, SPACE  ; } while(ch == \' \' || ch == \'\\t\' || ch == \'\\n\');\n  JZE  RI1\n  CPA  gr7, TAB\n  JZE  RI1\n  CPA  gr7, NEWLINE\n  JZE  RI1\n  LD  gr5, ONE  ; flag = 1\n  CPA  gr7, MINUS  ; if(ch == \'-\') {\n  JNZ  RI4\n  LD  gr5, gr0  ;  flag = 0;\n  CALL  READCHAR  ;  ch = READCHAR();\n  LD  gr7, 0,gr1\nRI4      ; }\n  LD  gr6, gr0  ; v = 0;\nRI2\n  CPA  gr7, ZERO  ; while(\'0\' <= ch && ch <= \'9\') {\n  JMI  RI3\n  CPA  gr7, NINE\n  JPL  RI3\n  MULA  gr6, TEN  ;  v = v*10+ch-\'0\';\n  ADDA  gr6, gr7\n  SUBA  gr6, ZERO\n  CALL  READCHAR  ;  ch = READSCHAR();\n  LD  gr7, 0,gr1\n  JUMP  RI2  ; }\nRI3\n  ST  gr7, RPBBUF  ; ReadPushBack();\n  ST  gr6, 0,gr1  ; *gr1 = v;\n  CPA  gr5, gr0  ; if(flag == 0) {\n  JNZ  RI5\n  SUBA  gr5, gr6  ;  *gr1 = -v;\n  ST  gr5, 0,gr1\nRI5      ; }\n  RPOP\n  RET\nREADLINE\n  ST  gr0, IBUFSIZE\n  ST  gr0, INP\n  ST  gr0, RPBBUF\n  RET\nONE    DC  1\nSIX    DC  6\nTEN    DC  10\nSPACE    DC  #0020  ; \' \'\nMINUS    DC  #002D  ; \'-\'\nTAB    DC  #0009  ; \'\\t\'\nZERO    DC  #0030  ; \'0\'\nNINE    DC  #0039  ; \'9\'\nNEWLINE    DC  #000A  ; \'\\n\'\nINTBUF    DS  8\nOBUFSIZE  DC  0\nIBUFSIZE  DC  0\nINP    DC  0\nOBUF    DS  257\nIBUF    DS  257\nRPBBUF    DC  0\n");
}
