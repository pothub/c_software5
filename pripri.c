#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scan.h"
#include "pripri.h"
#include "cross.h"
#include "compiler.h"

#define P_SUCCESS 0
#define P_FAIL 1

// #define PrittyPrint_debag

int token = 0;
int pre_token2 = 0;
int tabnum = 0, once_enter_t = 0;
int ProcFlag = 0;
int only_variable=0,isargument=0;
char str_c[MAXSTRSIZE];

int p_error(char *mes){
	static int once_end = 0;
	if(once_end == 0){
		printf("\n Error prase: line = %d\t%s\n",get_linenum(),mes);
		end_scan();
		once_end = 1;
	}
	return P_FAIL;
}
void PrittyPrint(int PPtoken, int NextLine, int StartWord){
	int i,tabnum_t;
	char str_tmp[MAXSTRSIZE*2] = {0};
	if(StartWord){
		strcat(str_tmp,";");
		for(tabnum_t = 0; tabnum_t < tabnum; tabnum_t++)
			strcat(str_tmp,"    ");
	}

	if(PPtoken == TNUMBER){
		char n_t[10];
		sprintf(n_t,"%d",num_attr);
		strcat(str_tmp,n_t);
	}
	else if(PPtoken == TSTRING) strcat(str_tmp,string_attr);
	else if(PPtoken == TNAME) strcat(str_tmp,string_attr);
	else{
		for(i=0;i<KEYWORDSIZE;i++){
			if( token == key[i].keytoken ){
				strcat(str_tmp, key[i].keyword);
				break;
			}
		}

		for(i=0; i<SIGNSIZE; i++){
			if( token == signs[i].keytoken ){
				strcat(str_tmp, signs[i].keyword);
				break;
			}
		}
	}
	if(NextLine){
		strcat(str_tmp,"\n");
		once_enter_t = 1;
	}
	else{
		strcat(str_tmp," ");
		once_enter_t = 0;
	}

#ifdef PrittyPrint_debag
	printf("%s",str_tmp);

	CompileOutputCSL(str_tmp);
	if(NextLine)
		CompileOutputLine();
#endif
}
void once_enter(){
#ifdef PrittyPrint_debag
	if(!once_enter_t) printf("\n");
	CompileOutputCSL("\n");
#endif
	CompileOutputLine();
}

int prase(){
	token = scan();
	if(p_program() != 0) return -1;
	return P_SUCCESS;
}

int p_program(){
	if(token != TPROGRAM) return(p_error("Keyword 'program' is not found"));
	PrittyPrint(token,0,1);
	token = scan();

	if(token != TNAME) return (p_error("Program name is not found"));
	PrittyPrint(token,0,0);
	sprintf(str_c,"$$%s\tSTART\n",string_attr);
	CompilePrintStack(str_c);
	CompilePrintStack("\tLAD\tgr0,0\n");
	CompilePrintStack("\tCALL\tL0001\n");
	CompilePrintStack("\tCALL\tFLUSH\n");
	CompilePrintStack("\tSVC\t0\n");
	token = scan();

	if(token != TSEMI) return (p_error("Semicolon is not found"));
	PrittyPrint(token,1,0);
	token = scan();

	if(block() != P_SUCCESS) return (p_error("block error"));
	if(token != TDOT) return (p_error("Period is not found at the end of program"));
	PrittyPrint(token,1,0);

	CompileOutputString();
	CompilerLibrary();
	CompileOutputCSL("\tEND");
	return P_SUCCESS;
}

int block(){
	int label;

	while(ValiableDeclaration() == P_SUCCESS || SubProgram() == P_SUCCESS);
	if(CompositeSentence(1) != P_SUCCESS) return P_FAIL;
	CompileOutputCSL("\tRET\n");
	return P_SUCCESS;
}

int ValiableDeclaration(){
	if(token != TVAR) return P_FAIL;
	tabnum++;
	PrittyPrint(token,1,1);
	tabnum++;
	token = scan();
	if(ValiableSequence(1) != P_SUCCESS) return P_FAIL;
	do{
		if(token != TCOLON) return(p_error("Keyword 'colon' is not found"));
		PrittyPrint(token,0,0);
		token = scan();
		if(Type() != P_SUCCESS) return P_FAIL;
		if(token != TSEMI) return(p_error("Keyword 'semi' is not found"));
		CompileDeclareStack(&globalidroot);
		PrittyPrint(token,1,0);
		token = scan();
	}while(ValiableSequence(1) == P_SUCCESS);
	tabnum-=2;
	return P_SUCCESS;
}
int ValiableSequence(int top){
	Id_c *id;
	if(token == TBEGIN || token == TPROCEDURE) return P_FAIL;	//end
	else if(token != TNAME) return P_FAIL;
	PrittyPrint(token,0,top);
	AddIdName(string_attr,TOP,!top,&globalidroot);
	token = scan();

	while(token == TCOMMA){
		PrittyPrint(token,0,0);
		token = scan();
		if(token == TBEGIN || token == TPROCEDURE) return 1;	//end
		else if(token != TNAME) return (p_error("ValiableSequence2 ValueName is not found"));
		PrittyPrint(token,0,0);
		AddIdName(string_attr,NEX,!top,&globalidroot);
		token = scan();
	}
	return P_SUCCESS;
}
int NomalFlag = 0;
int Type(){
	num_attr = 0;
	NomalFlag = 1;
	if(NomalType() == P_SUCCESS || ArrayType() == P_SUCCESS) return P_SUCCESS;
	else return P_FAIL;
}
int NomalType(){
	if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		PrittyPrint(token,0,0);
		if(NomalFlag == 1){
			AddIdType(token,&globalidroot);
			pre_token2 = token;
		}
		token = scan();
		return P_SUCCESS;
	}
	return P_FAIL;
}
int ArrayType(){
	if(token != TARRAY) return P_FAIL;
	PrittyPrint(token,0,0);
	token = scan();
	if(token != TLSQPAREN) return p_error("ArrayType '[' is not found");
	PrittyPrint(token,0,0);
	token = scan();
	if(token != TNUMBER) return p_error("ArrayType num is not found");
	PrittyPrint(token,0,0);
	token = scan();
	if(token != TRSQPAREN) return p_error("ArrayType ']' is not found");
	PrittyPrint(token,0,0);
	token = scan();
	if(token != TOF) return p_error("ArrayType 'of' is not found");
	PrittyPrint(token,0,0);
	token = scan();
	if(NomalType() != P_SUCCESS) return P_FAIL;
	return P_SUCCESS;
}
int FormalArgument(){
	if(token != TLPAREN) return P_FAIL;
	PrittyPrint(token,0,0);
	SetFormalNum_t();
	token = scan();
	if(ValiableSequence(0) != P_SUCCESS) return P_FAIL;
	if(token != TCOLON) return p_error("FomalArgument ':' is not found");
	PrittyPrint(token,0,0);
	token = scan();
	if(Type() != P_SUCCESS) return P_FAIL;
	AddFormal(pre_token2,&globalidroot);
	CompileDeclareStack(&globalidroot);

	while(token == TSEMI){
		PrittyPrint(token,1,0);
		token = scan();
		if(ValiableSequence(0) != P_SUCCESS) return P_FAIL;
		if(token != TCOLON) return p_error("FomalArgument ':' is not found");
		PrittyPrint(token,0,0);
		token = scan();
		if(Type() != P_SUCCESS) return P_FAIL;
		CompileDeclareStack(&globalidroot);
	}

	if(token != TRPAREN) return p_error("FomalArgument ')' is not found");
	PrittyPrint(token,0,0);
	token = scan();
	return P_SUCCESS;
}
int SubProgram(){
	int F_flag=0;
	if(token != TPROCEDURE) return P_FAIL;
	tabnum++;
	PrittyPrint(token,0,1);
	token = scan();
	if(token != TNAME) return(p_error("SubProgram 'name' is not found"));
	PrittyPrint(token,0,0);
	num_attr = 0;
	AddIdName(string_attr,TOP,0,&globalidroot);
	AddIdType(TPPROC,&globalidroot);
	SetProcName();
	token = scan();
	if(token == TLPAREN){
		F_flag=1;
		if(FormalArgument() != P_SUCCESS) return P_FAIL;
	}
	if(token != TSEMI) return(p_error("SubProgram 'semi' is not found"));
	PrittyPrint(token,1,0);
	token = scan();
	if(token == TVAR){
		if(ValiableDeclaration() != P_SUCCESS) return P_FAIL;
	}
	if((tabnum == 0) || (tabnum == 1) && (ProcFlag == 1) ){
		sprintf(str_c,"$%s\n",procname_t);
		CompilePrintStack(str_c);
	}
	if(F_flag){
		CompilePrintStack("\tPOP\tgr2\n");
		CompileFormalPS(&globalidroot);
		CompilePrintStack("\tPUSH\t0,gr2\n");
	}
	if(CompositeSentence(1) != P_SUCCESS) return P_FAIL;
	if(token != TSEMI) return(p_error("SubProgram 'semi' is not found"));
	PrittyPrint(token,1,0);
	FreeProcName();
	token = scan();
	tabnum--;
	return P_SUCCESS;
}
int CompositeSentence(int begin_tab){
	if(token != TBEGIN) return P_FAIL;
	if(!begin_tab) tabnum--;	//else,do,then tab--
	if(tabnum == 0) CompilePrintStack("L0001\n");
	PrittyPrint(token,1,1);
	tabnum++;
	token = scan();

	do{
		if(token == TSEMI){
			PrittyPrint(token,1,0);
			token = scan();
		}
		if(Sentence() != P_SUCCESS) return P_FAIL;
	}while(token == TSEMI);

	if(token != TEND) return P_FAIL;

	once_enter();
	tabnum--;
	PrittyPrint(token,0,1);

	if((tabnum == 0) || (tabnum == 1) && (ProcFlag == 1) ) CompilePrintStack("\tRET\n");

	if(!begin_tab) tabnum++;
	token = scan();

	return P_SUCCESS;
}
int Sentence(){
	if(AssignmentSentence() == P_SUCCESS) return P_SUCCESS;
	else if(DivergencySentence() == P_SUCCESS) return P_SUCCESS;
	else if(LoopSentence() == P_SUCCESS) return P_SUCCESS;
	else if(CallSentence() == P_SUCCESS) return P_SUCCESS;
	else if(token == TRETURN){
		token = scan();
		return P_SUCCESS;
	}
	else if(InputSentence() == P_SUCCESS) return P_SUCCESS;
	else if(OutputSentence() == P_SUCCESS) return P_SUCCESS;
	else if(CompositeSentence(0) == P_SUCCESS) return P_SUCCESS;
	else if(token == TSEMI || token == TEND) return P_SUCCESS;
	return (p_error("Sentence error"));
}
int AssignmentSentence(){
	if(Value(1) != P_SUCCESS) return P_FAIL;
	CompilePrintStack("\tPUSH\t0,gr1\n");
	// CompileLD_LAD(&globalidroot);

	if(token != TASSIGN) return p_error("Assignment ':=' is not found");
	PrittyPrint(token,0,0);
	token = scan();

	if(Fomula() != P_SUCCESS) return p_error("Assignment Fomula error");
	CompilePrintStack("\tPOP\tgr2\n");
	CompilePrintStack("\tST\tgr1,0,gr2\n");
	return P_SUCCESS;
}
int DivergencySentence(){
	static int D_top=1;
	int else_label,end_label;
	char DS_tmp[MAXSTRSIZE];
	if(token != TIF) return P_FAIL;
	else_label = CompileGetLabel();
	PrittyPrint(token,0,D_top);
	token = scan();
	if(Fomula() != P_SUCCESS) return p_error("Divergency Fomula error");
	CompilePrintStack("\tCPA\tgr1,gr0\n");
	sprintf(DS_tmp,"\tJZE\tL%04d\n",else_label);
	CompilePrintStack(DS_tmp);
	if(token != TTHEN) return p_error("Divergency 'then' is not found");
	PrittyPrint(token,1,0);
	token = scan();
	tabnum++;
	if(Sentence() != P_SUCCESS) return p_error("Divergency Sentence error");
	tabnum--;

	if(token == TELSE){
		end_label = CompileGetLabel();
		sprintf(DS_tmp,"\tJUMP\tL%04d\n",end_label);
		CompilePrintStack(DS_tmp);
		sprintf(DS_tmp,"L%04d\n",else_label);
		CompilePrintStack(DS_tmp);
		once_enter();
		PrittyPrint(token,0,1);
		token = scan();
		if(token == TIF){
			D_top = 0;
			if(Sentence() != P_SUCCESS) return p_error("Divergency Sentence error");
		}
		else{
			tabnum++;
			D_top = 1;
			once_enter();
			if(Sentence() != P_SUCCESS) return p_error("Divergency Sentence error");
			tabnum--;
		}
		sprintf(DS_tmp,"L%04d\n",end_label);
		CompilePrintStack(DS_tmp);
	}
	else{
		sprintf(DS_tmp,"L%04d\n",else_label);
		CompilePrintStack(DS_tmp);
	}
	return P_SUCCESS;
}
int LoopSentence(){
	char LP_tmp[MAXSTRSIZE];
	int start_l=0,end_l=0;
	if(token != TWHILE) return P_FAIL;

	start_l = CompileGetLabel();
	end_l = CompileGetLabel();
	sprintf(LP_tmp,"L%04d\n",start_l);
	CompilePrintStack(LP_tmp);

	PrittyPrint(token,0,1);
	token = scan();
	if(Fomula() != P_SUCCESS) return p_error("Loop Fomula error");
	if(token != TDO) return p_error("Loop 'do' is not found");
	// CompilePrintStack("\tPUSH\t0,gr1\n");

	CompilePrintStack("\tCPA\tgr1,gr0\n");
	sprintf(LP_tmp,"\tJZE\tL%04d\n",end_l);
	CompilePrintStack(LP_tmp);

	PrittyPrint(token,1,0);
	tabnum++;
	token = scan();
	if(Sentence() != P_SUCCESS) return P_FAIL;
	tabnum--;

	sprintf(LP_tmp,"\tJUMP\tL%04d\n",start_l);
	CompilePrintStack(LP_tmp);
	sprintf(LP_tmp,"L%04d\n",end_l);
	CompilePrintStack(LP_tmp);

	return P_SUCCESS;
}
int CallSentence(){
	char CS_tmp[MAXSTRSIZE],CS_name[MAXSTRSIZE];
	if(token != TCALL) return P_FAIL;
	PrittyPrint(token,0,1);
	token = scan();
	if(token != TNAME) return p_error("Call 'name' is not found");
	strcpy(CS_name,string_attr);
	PrittyPrint(token,0,0);
	num_attr = 0;
	if(!__CompareID(&globalidroot)) c_error("Recall");
	// CompareID(&globalidroot);
	token = scan();

	if(token == TLPAREN){
		PrittyPrint(token,0,0);
		token = scan();
		isargument=1;
		if(FomulaSequence() != P_SUCCESS) return P_FAIL;
		isargument=0;
		if(token != TRPAREN) return p_error("CallSentence ')' is not found");
		PrittyPrint(token,0,0);
		token = scan();
	}
	sprintf(CS_tmp,"\tCALL\t$%s\n",CS_name);
	CompilePrintStack(CS_tmp);
	return P_SUCCESS;
}
int FomulaSequence(){
	int l=0;
	char FS_tmp[MAXSTRSIZE];
	if(Fomula() != P_SUCCESS) return P_FAIL;

	if(only_variable) CompilePrintStack("\tPUSH\t0,gr1\n");
	else{
		l=CompileGetLabel();
		sprintf(FS_tmp,"\tLAD\tgr2,\tL%04d\n",l);
		CompilePrintStack(FS_tmp);
		CompilePrintStack("\tST\tgr1,\t0,gr2\n");
		CompilePrintStack("\tPUSH\t0,gr2\n");
		CompileStack(l,"0",0);
	}

	while(token == TCOMMA){
		PrittyPrint(token,0,0);
		token = scan();
		if(Fomula() != P_SUCCESS) return p_error("FomulaSequence Fomula error");
		if(only_variable) CompilePrintStack("\tPUSH\t0,gr1\n");
		else{
			l=CompileGetLabel();
			sprintf(FS_tmp,"\tLAD\tgr2,\tL%04d\n",l);
			CompilePrintStack(FS_tmp);
			CompilePrintStack("\tST\tgr1,\t0,gr2\n");
			CompilePrintStack("\tPUSH\t0,gr2\n");
		}
	}
	return P_SUCCESS;
}
int Input_f=0;
int InputSentence(){
	int ln_f=0;
	if(token != TREAD && token != TREADLN) return P_FAIL;
	if(token == TREADLN) ln_f=1;
	PrittyPrint(token,0,1);
	token = scan();
	Input_f = 1;

	if(token == TLPAREN){
		do{
			PrittyPrint(token,0,0);
			token = scan();
			if(Value(0) != P_SUCCESS) return p_error("InputSentence Value error");
		}while(token == TCOMMA);
	}
	Input_f = 0;
	if(token != TRPAREN)return p_error("InputSentence ')' error");
	PrittyPrint(token,0,0);
	CompilePrintStack("\tCALL\tREADINT\n");
	if(ln_f) CompilePrintStack("\tCALL\tREADLINE\n");
	token = scan();
	return P_SUCCESS;
}
int OutputSentence(){
	int OS_f=0;
	if(token != TWRITE && token != TWRITELN) return P_FAIL;
	if(token == TWRITELN) OS_f = 1;
	PrittyPrint(token,0,1);
	token = scan();

	if(token == TLPAREN){
		do{
			PrittyPrint(token,0,0);
			token = scan();
			if(OutputSpecification() != P_SUCCESS) return p_error("OutputSentence Specification error");
		}while(token == TCOMMA);
	}
	if(token != TRPAREN)return P_FAIL;
	PrittyPrint(token,0,0);
	if(OS_f) CompilePrintStack("\tCALL\tWRITELINE\n");
	token = scan();
	return P_SUCCESS;
}
int OutputSpecification(){
	char OS_tmp[MAXSTRSIZE];
	if(token == TSTRING){
		sprintf(OS_tmp,"\tLAD\tgr1,\tL%04d\n",label_num);
		CompilePrintStack(OS_tmp);
		CompilePrintStack("\tLD\tgr2,gr0\n");
		CompilePrintStack("\tCALL\tWRITESTR\n");
		CompileStack(CompileGetLabel(),string_attr,1);
		PrittyPrint(token,0,0);
		token = scan();
	}else{
		if(Fomula() != P_SUCCESS) return P_FAIL;
		CompilePrintStack("\tLD\tgr2,gr0\n");
		CompilePrintStack("\tCALL\tWRITEINT\n");
		while(token == TCOLON){
			PrittyPrint(token,0,0);
			token = scan();
			if(token != TNUMBER) return p_error("OutputSpecification 'number' is not found");
			PrittyPrint(token,0,0);
			token = scan();
		}
	}
	return P_SUCCESS;
}
int Value(int top){
	if(token != TNAME) return P_FAIL;
	PrittyPrint(token,0,top);
	CompareID(&globalidroot);
	token = scan();
	if(token != TLSQPAREN) return P_SUCCESS;	//nothing []
	else{
		PrittyPrint(token,0,0);
		token = scan();
		if(Fomula() != P_SUCCESS) return p_error("Value 'fomula' error");
		if(token != TRSQPAREN) return p_error("Value ']' is not found");
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}
}
int Fomula(){
	int start_l,end_l,opr;
	char F_tmp[MAXSTRSIZE];
	only_variable = 1;
	if(SimplicityFomula() != P_SUCCESS) return P_FAIL;
	// CompilePrintStack("\tPOP\tgr2\n");
	opr = token;
	while(RelationalOperator() == P_SUCCESS){
		CompilePrintStack("\tPUSH\t0,gr1\n");
		if(SimplicityFomula() != P_SUCCESS) return P_FAIL;
		CompilePrintStack("\tPOP\tgr2\n");
		CompilePrintStack("\tCPA\tgr2,gr1\n");
		start_l = CompileGetLabel();
		end_l = CompileGetLabel();
		if(opr == TEQUAL)
			sprintf(F_tmp,"\tJZE\tL%04d\n",start_l);
		else if(opr == TNOTEQ)
			sprintf(F_tmp,"\tJNZ\tL%04d\n",start_l);
		else if(opr == TLE)
			sprintf(F_tmp,"\tJMI\tL%04d\n",start_l);
		else if(opr == TLEEQ){
			sprintf(F_tmp,"\tJMI\tL%04d\n",start_l);
			sprintf(F_tmp,"\tJZE\tL%04d\n",start_l);
		}
		else if(opr == TGR)
			sprintf(F_tmp,"\tJPL\tL%04d\n",start_l);
		else if(opr == TGREQ){
			sprintf(F_tmp,"\tJPL\tL%04d\n",start_l);
			sprintf(F_tmp,"\tJZE\tL%04d\n",start_l);
		}
		CompilePrintStack(F_tmp);
		CompilePrintStack("\tLD\tgr1,gr0\n");
		sprintf(F_tmp,"\tJUMP\tL%04d\n",end_l);
		CompilePrintStack(F_tmp);
		sprintf(F_tmp,"L%04d\n",start_l);
		CompilePrintStack(F_tmp);
		CompilePrintStack("\tLAD\tgr1,1\n");
		sprintf(F_tmp,"L%04d\n",end_l);
		CompilePrintStack(F_tmp);
	}
	return P_SUCCESS;
}
int SimplicityFomula(){
	int opr = token;
	if(token == TPLUS){
		PrittyPrint(token,0,0);
		token = scan();
	}
	else if(token == TMINUS){
		CompilePrintStack("\tLAD\tgr1,\t0\n");
		CompilePrintStack("\tPUSH\t0,gr1\n");
		PrittyPrint(token,0,0);
		token = scan();
	}

	if(Kou() != P_SUCCESS) return P_FAIL;

	if(opr == TMINUS){
		CompilePrintStack("\tPOP\tgr2\n");
		CompilePrintStack("\tSUBA\tgr2,gr1\n");
		CompilePrintStack("\tJOV\tEOVF\n");
		CompilePrintStack("\tLD\tgr1,gr2\n");
	}
	opr = token;
	while(AddOperator() == P_SUCCESS){
		CompilePrintStack("\tLD\tgr1,0,gr1\n");////
		CompilePrintStack("\tPUSH\t0,gr1\n");
		if(Kou() != P_SUCCESS) return P_FAIL;
		CompilePrintStack("\tPOP\tgr2\n");
		if(opr == TPLUS){
			CompilePrintStack("\tADDA\tgr1,gr2\n");
			CompilePrintStack("\tJOV\tEOVF\n");
		}
		else if(opr == TMINUS){
			CompilePrintStack("\tSUBA\tgr2,gr1\n");
			CompilePrintStack("\tJOV\tEOVF\n");
			CompilePrintStack("\tLD\tgr1,gr2\n");
		}
		else if(opr == TOR)
			CompilePrintStack("\tOR\tgr1,gr2\n");
	}
	return P_SUCCESS;
}
int Kou(){
	int opr;
	if(Factor() != P_SUCCESS) return P_FAIL;
	opr = token;
	while(MultiplOperator() == P_SUCCESS){
		only_variable = 0;
		CompilePrintStack("\tLD\tgr1,0,gr1\n");
		CompilePrintStack("\tPUSH\t0,gr1\n");
		if(Factor() != P_SUCCESS) return P_FAIL;
		CompilePrintStack("\tPOP\tgr2\n");
		if(opr == TSTAR){
			CompilePrintStack("\tMULA\tgr1,gr2\n");
			CompilePrintStack("\tJOV\tEOVF\n");
		}
		else if(opr == TDIV){
			CompilePrintStack("\tDIVA\tgr2,gr1\n");
			CompilePrintStack("\tJOV\tE0DIV\n");
			CompilePrintStack("\tLD\tgr1,gr2\n");
		}
		else if(opr == TAND)
			CompilePrintStack("\tAND\tgr1,gr2\n");
	}
	return P_SUCCESS;
}
int Factor(){
	NomalFlag = 0;
	if(Value(0) == P_SUCCESS) return P_SUCCESS;
	else if(Constant() == P_SUCCESS) return P_SUCCESS;
	else if(token == TLPAREN){
		PrittyPrint(token,0,0);
		token = scan();
		if(Fomula() != P_SUCCESS)
			return p_error("Factor fomula error");
		if(token != TRPAREN) return p_error("Factor ')' is not found");
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}
	else if(token == TNOT){
		PrittyPrint(token,0,0);
		token = scan();
		if(Factor() != P_SUCCESS) return p_error("Factor not factor error");
		return P_SUCCESS;
	}
	else if(NomalType() == P_SUCCESS){
		if(token != TLPAREN) return p_error("Factor '(' error");
		PrittyPrint(token,0,0);
		token = scan();
		if(Fomula() != P_SUCCESS) return p_error("Factor fomula error");
		if(token != TRPAREN) return p_error("Factor ')' is not found");
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}
	else return P_FAIL;
}
int Constant(){
	char C_tmp[MAXSTRSIZE];
	if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
		if(token == TNUMBER)
			sprintf(C_tmp,"\tLAD\tgr1,\t%d\n",num_attr);
		else if(token == TFALSE)
			sprintf(C_tmp,"\tLAD\tgr1,\t0\n");
		else if(token == TTRUE)
			sprintf(C_tmp,"\tLAD\tgr1,\t1\n");
		else if(token == TSTRING){
			if(strlen(string_attr) == 1)
				sprintf(C_tmp,"\tLAD\tgr1,\t%d\n",string_attr[0]);
		}
		CompilePrintStack(C_tmp);
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}
	else return P_FAIL;
}
int MultiplOperator(){
	if(token == TSTAR || token == TDIV || token == TAND){
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}else P_FAIL;
}
int AddOperator(){
	if(token == TPLUS || token == TMINUS || token == TOR){
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}else P_FAIL;
}
int RelationalOperator(){
	if(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
		CompilePrintStack("\tLD\tgr1,0,gr1\n");
		PrittyPrint(token,0,0);
		token = scan();
		return P_SUCCESS;
	}else P_FAIL;
}
