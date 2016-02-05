#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scan.h"
#include "pripri.h"
#include "cross.h"
#include "compiler.h"

// #define Cross_debug

Id_c *globalidroot;
int Head_Name_Num = 0, Last_Name_Num = 0;
char *procname_t;

void init_cross(){
	globalidroot = (Id_c *)malloc(sizeof(Id_c));
}

void RemoveGrobalId(){
	Id_c *next,*del;
	next = globalidroot->nextp;

	while(next){
		del = next;
		next = next->nextp;
		free(del);
	}
}
void c_error(char *mes){
	RemoveGrobalId();
	printf("\n Error cross: line = %d\t%s\n",get_linenum(),mes);
	end_scan();
}

void PrintIdList(Id_c *id){
#ifdef Cross_debug
	printf("%-32s%-32s%4s %-16s","Name","Type","Def.","Ref.");
	while(id != NULL){
		PrintId(id);
		id = id->nextp;
	}
	printf("\n");
#endif
}

void PrintId(Id_c *id){
	printf("\n");
	PrintIdName(id);
	PrintIdType(id);
	printf("%4d|",id->ispara);
	printf("%4d|",id->deflinenum);
	PrintIdRef(id);
}

void PrintIdName(Id_c *id){//////////////////////
	char str[MAXSTRSIZE*2];
	sprintf(str,"%s", id->name);
	if( id->procname != NULL ){
		strcat(str,":");
		strcat(str, id->procname);
	}
	printf("%-32s",str);
}
void PrintIdType(Id_c *id){
	char str[MAXSTRSIZE],buff[MAXSTRSIZE];

	if(id->itp == NULL)
		sprintf(str,"ElseType");
	else{
		if(id->itp->ttype == TPINT)
			sprintf(str,"integer");
		else if(id->itp->ttype == TPCHAR)
			sprintf(str,"char");
		else if(id->itp->ttype == TPBOOL)
			sprintf(str,"bool");
		else if(id->itp->ttype == TPARRAY)
			sprintf(str,"array[%d]",id->itp->arraysize);
		else if(id->itp->ttype == TPARRAYINT)
			sprintf(str,"array[%d] of integer",id->itp->arraysize);
		else if(id->itp->ttype == TPARRAYCHAR)
			sprintf(str,"array[%d] of char",id->itp->arraysize);
		else if(id->itp->ttype == TPARRAYBOOL)
			sprintf(str,"array[%d] of bool",id->itp->arraysize);
		else if(id->itp->ttype == TPPROC){
			sprintf(str,"procedure");
			if( id->itp->paratp != NULL ){
				sprintf(buff,"(");	
				strcat(str,buff);
				sprintParatp(buff, id);
				strcat(str,buff);
				sprintf(buff,")");	
				strcat(str,buff);
			}
		}
		else
			sprintf(str,"ElseType");
	}

	printf("%-32s",str);
}

void sprintParatp(char *str, Id_c *id){
	char buff[MAXSTRSIZE];
	Type_c *para = id->itp->paratp;
	str[0] = '\0';
	while( para != NULL ){
		_PrintIdType(buff, para);
		strcat(str,buff);
		para = para->paratp;
		if( para != NULL )	strcat(str,", ");
	}
}
void _PrintIdType(char *str, Type_c *itp){
	if( itp->ttype == TPINT)
		sprintf( str , "integer");
	else if( itp->ttype == TPCHAR)
		sprintf( str , "char");
	else if( itp->ttype == TPBOOL)
		sprintf( str , "bool");
	else if( itp->ttype == TPARRAY)
		sprintf( str , "array[%2d]", itp->arraysize);
	else if( itp->ttype == TPARRAYINT)
		sprintf( str , "array[%2d] of integer", itp->arraysize);
	else if( itp->ttype == TPARRAYCHAR)
		sprintf( str , "array[%2d] of char", itp->arraysize);
	else if( itp->ttype == TPARRAYBOOL)
		sprintf( str , "array[%2d] of bool", itp->arraysize );
	else if( itp->ttype == TPPROC)
		sprintf( str , "procedure");
	else
		sprintf( str , "Unknown");
}

void PrintIdRef(Id_c *id){
	Line_c *ref = id->irefp;
	while(ref != NULL){
		printf("%d",ref->reflinenum);
		if(ref->nextlinep != NULL)
			printf(", ");
		ref = ref->nextlinep;
	}
}

extern int ProcFlag;
void AddIdName(char *name, int top_next, int _ispara, Id_c **id){
	if(top_next == TOP) Head_Name_Num = Last_Name_Num;
	Last_Name_Num++;

	Id_c *newnode;
	newnode = (Id_c *)malloc(sizeof(Id_c));
	newnode->itp = (Type_c *)malloc(sizeof(Type_c));

	newnode->name = malloc(strlen(name));
	strcpy(newnode->name,name);
	if(ProcFlag == 1){
		newnode->procname = malloc(strlen(procname_t));
		strcpy(newnode->procname,procname_t);
	}
	newnode->ispara = _ispara;
	newnode->deflinenum = get_linenum();
	newnode->irefp = NULL;
	newnode->nextp = NULL;

	if(!_CompareID(&globalidroot)) c_error("Double decralation");

	while(*id != NULL){
		id = &((*id)->nextp);
	}
	*id = newnode;
}
int lastID=0;
void CompareID(Id_c **id){
	int l_flag=0,g_flag=0,match=0;
	int lastID_t=0;
	Id_c **secondid = id;
	Id_c **thirdid = id;
	while((*id)->nextp != NULL){
		id = &((*id)->nextp);
		lastID_t++;
		if(!strcmp(string_attr, (*id)->name)){		// if match word
			Line_c *L = (*id)->irefp;
			if((*id)->procname == NULL && ProcFlag == 0){	//if global
				if((*id)->irefp == NULL){
					(*id)->irefp = CreateLine(get_linenum());
				}
				else{
					while(L->nextlinep != NULL) L=L->nextlinep;
					L->nextlinep = CreateLine(get_linenum());
				}
				g_flag = 1;
				match=1;
				lastID = lastID_t;
			}
			else if((*id)->procname != NULL){				//if local
				if(!strcmp(procname_t, (*id)->procname)){
					if((*id)->irefp == NULL) (*id)->irefp = CreateLine(get_linenum());
					else{
						while(L->nextlinep != NULL) L=L->nextlinep;
						L->nextlinep = CreateLine(get_linenum());
					}
					l_flag = 1;
					match=1;
					lastID = lastID_t;
				}
				else if((*id)->itp->ttype == TPPROC){
					if(!strcmp(procname_t, (*id)->procname)){
						if((*id)->irefp == NULL) (*id)->irefp = CreateLine(get_linenum());
						else{
							while(L->nextlinep != NULL) L=L->nextlinep;
							L->nextlinep = CreateLine(get_linenum());
						}
						match=1;
						lastID = lastID_t;
					}
				}
			}
		}
	}
	if(g_flag == 0 && l_flag == 0){	//can't find
		while((*secondid)->nextp != NULL){
			secondid = &((*secondid)->nextp);
			if(!strcmp(string_attr, (*secondid)->name)){		// if match word
				Line_c *L = (*secondid)->irefp;
				if((*secondid)->procname == NULL && ProcFlag == 1){
					if((*secondid)->irefp == NULL){
						(*secondid)->irefp = CreateLine(get_linenum());
					}
					else{
						while(L->nextlinep != NULL) L=L->nextlinep;
						L->nextlinep = CreateLine(get_linenum());
					}
					match=1;
					lastID = lastID_t;
				}
			}
		}
	}
	if(match != 1) c_error("Undeclared value");

	char comp_tmp[MAXSTRSIZE],comp2_tmp[MAXSTRSIZE];
	while((lastID--) != 0) thirdid = &((*thirdid)->nextp);
	if(l_flag) sprintf(comp_tmp,"$%s%%%s",string_attr,procname_t);
	else sprintf(comp_tmp,"$%s",string_attr);
	if((*id)->itp->ttype == TPARRAYINT || (*id)->itp->ttype == TPARRAYCHAR 
			||(*id)->itp->ttype == TPARRAYBOOL ||(*id)->itp->ttype == TPARRAY){
		if(ProcFlag && !Input_f)
			sprintf(comp2_tmp,"\tLD\tgr1,\t%s,\tgr1\n",comp_tmp);
		else
			sprintf(comp2_tmp,"\tLAD\tgr1,\t%s,\tgr1\n",comp_tmp);
	}
	else{
		if((ProcFlag && !Input_f) || tabnum == 2)
			sprintf(comp2_tmp,"\tLD\tgr1,\t%s\n",comp_tmp);
		else
			sprintf(comp2_tmp,"\tLAD\tgr1,\t%s\n",comp_tmp);
	}

	CompilePrintStack(comp2_tmp);
}
int _CompareID(Id_c **id){
	int match=0;
	while((*id)->nextp != NULL){
		id = &((*id)->nextp);
		if(!strcmp(string_attr, (*id)->name)){		// if match word
			if((*id)->procname == NULL && ProcFlag == 0){	//if global
				match = 1;
			}
			else if((*id)->procname != NULL){				//if local
				if(!strcmp(procname_t, (*id)->procname)){
					match = 1;
				}
			}
		}
	}
	if(match) return 0;
	else return 1;
}
int __CompareID(Id_c **id){
	if(!strcmp(procname_t,string_attr)) return 0;
	else return 1;
}
Line_c *CreateLine(int reflinenum){
	Line_c *l;
	l = (Line_c *)malloc(sizeof(Line_c));
	l->nextlinep = NULL;
	l->reflinenum = reflinenum;
	return l;
}
void AddIdType(int token, Id_c **id){
	Type_c *newtype;
	newtype = (Type_c *)malloc(sizeof(Type_c));
	if(num_attr == 0){
		newtype->ttype = token;
	}
	else{
		newtype->ttype = token+100;
		newtype->arraysize = num_attr;
	}

	int i;
	for(i=0;i<Head_Name_Num;i++) id = &((*id)->nextp);
	for(i=0;i<Last_Name_Num - Head_Name_Num;i++){
		id = &((*id)->nextp);
		(*id)->itp = newtype;
	}
}
void SetProcName(){
	procname_t = (char *)malloc(strlen(string_attr)+1);
	strcpy(procname_t, string_attr);
	ProcFlag = 1;
}
void FreeProcName(){
	free(procname_t);
	ProcFlag = 0;
}
int SetProcNum = 0;
void SetFormalNum_t(){
	SetProcNum = Last_Name_Num;
}
void AddFormal(int token, Id_c **id){
	Type_c *newtype;
	newtype = (Type_c *)malloc(sizeof(Type_c));
	newtype->ttype = token;
	int i;
	for(i=0;i<SetProcNum;i++) id = &((*id)->nextp);
	Type_c *T = (*id)->itp;
	for(i=0;i<Last_Name_Num - SetProcNum;i++){
		T->paratp = newtype;
		newtype = (Type_c *)malloc(sizeof(Type_c));
		newtype->ttype = token;
		T = T->paratp;
	}
}
