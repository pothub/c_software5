#include "scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define scan_debug
/* keyword list */
struct KEY key[KEYWORDSIZE] = {
	{"and", 	TAND	},
	{"array",	TARRAY	},
	{"begin",	TBEGIN	},
	{"boolean",	TBOOLEAN},
	{"call",	TCALL	},
	{"char",	TCHAR	},
	{"div",		TDIV	},
	{"do",		TDO	},
	{"else",	TELSE	},
	{"end",		TEND	},
	{"false",	TFALSE	},
	{"if",		TIF	},
	{"integer",	TINTEGER},
	{"not",		TNOT	},
	{"of",		TOF	},
	{"or",		TOR	},
	{"procedure", TPROCEDURE},
	{"program",	TPROGRAM},
	{"read",	TREAD	},
	{"readln",	TREADLN	},
	{"return", TRETURN	},
	{"then",	TTHEN	},
	{"true",	TTRUE	},
	{"var",		TVAR	},
	{"while",	TWHILE	},
	{"write",	TWRITE  },
	{"writeln",	TWRITELN}
};
struct KEY signs[SIGNSIZE] = {
	{"+", 	TPLUS	},
	{"-",	TMINUS	},
	{"*",	TSTAR	},
	{"=",	TEQUAL	},
	{"<>",	TNOTEQ	},
	{"<",	TLE		},
	{"<=",	TLEEQ	},
	{">",	TGR		},
	{">=",	TGREQ	},
	{"(",	TLPAREN	},
	{")",	TRPAREN	},
	{"[",	TLSQPAREN},
	{"]",	TRSQPAREN},
	{":=",	TASSIGN	},
	{".",	TDOT	},
	{",",	TCOMMA	},
	{":",   TCOLON	},
	{";",	TSEMI	}
};
/* Token counter */
int numtoken[NUMOFTOKEN+1];

int linenum=0,token_num=0;
int init_scan(char *filename){
	if((fp=fopen(filename,"r"))==NULL)
		return -1;
	linenum=0;
	return 0;
}
char string_attr[MAXSTRSIZE];
int num_attr;
int scan(){
	static char cbuf[MAXSTRSIZE];
	int num=0,end_flag=0,int_flag=0,string_flag=0,aps_flag=0,aps_flag2=0,com_flag1=0,com_flag2=0;

	if(linenum==0)
		linenum=1;

	while(1){
		if(fread(&cbuf[num],sizeof(char),1,fp)==0)
			return EOF;
		if(fread(&cbuf[num+1],sizeof(char),1,fp)==0){
			if(aps_flag>0 || com_flag1>0 || com_flag2>0)
				error_scan("not enough ' */ } ");
			return EOF;
		}
		fseek(fp,-1L,SEEK_CUR);
		//printf("%d\t%c\t",ftell(fp),cbuf[num]);

		//STRING
		if(cbuf[num]=='\'' && aps_flag == 0)
			aps_flag = 1;
		else if(aps_flag == 1){
			if(cbuf[num]!='\'' || aps_flag2 == 1){
				num++;
				aps_flag2 = 0;
			}
			else if(cbuf[num]=='\'' && cbuf[num+1]=='\''){
				num++;
				aps_flag2 = 1;
			}
			else if(cbuf[num]=='\'')
				end_flag = 1;
		}

		//comment
		else if(cbuf[num]=='{' || com_flag1>0){
			com_flag1++;
			if(cbuf[num]=='}' && com_flag1>1)
				end_flag=1;
			num++;
		}
		else if((cbuf[num]=='/' && cbuf[num+1]=='*') || com_flag2>0){
			com_flag2++;
			if((cbuf[num-1]=='*' && cbuf[num]=='/') && com_flag2>1)
				end_flag=1;
			num++;
		}
		else if(cbuf[num]=='\r' || cbuf[num]=='\n' || cbuf[num]==' ' || cbuf[num]=='\t'){
			if(cbuf[num]=='\n')
				linenum++;
			end_flag=1;
		}
		//control charactor
		else if(cbuf[num]<0x20 || 0x7E<cbuf[num]){
			error_scan("control charactor\n");
			return -1;
		}
		//sign
		else if(('('<=cbuf[num] && cbuf[num]<='.') || (':'<=cbuf[num] && cbuf[num]<='>') ||
				cbuf[num]=='[' || cbuf[num]==']'){
			num++;
			if(!(cbuf[num]=='>' || cbuf[num]=='='))
				end_flag=1;
		}
		else if(('A'<=cbuf[num] && cbuf[num]<='Z') ||  ('a'<=cbuf[num] && cbuf[num]<='z') ||
				string_flag==1){
			num++;
			string_flag=1;
			if(!(('A'<=cbuf[num] && cbuf[num]<='Z') ||  ('a'<=cbuf[num] && cbuf[num]<='z') ||
						('0'<=cbuf[num] && cbuf[num]<='9')))
				end_flag=1;
		}
		//number
		else if('0'<=cbuf[num] && cbuf[num]<='9'){
			num++;
			int_flag=1;
			if(!('0'<=cbuf[num] && cbuf[num]<='9'))
				end_flag=1;
		}
		else{
		}

		if(end_flag==1)
			break;
	}
	if(num>0){
		cbuf[num]='\0';

		if(int_flag==1){			//if int
			num_attr=atoi(cbuf);
			if(num_attr>32767){	
				error_scan("num error\n");
				return -1;
			}
#ifdef scan_debug
			printf("%d\tnum\n",num_attr);
#endif
			return TNUMBER;	
		}
		strcpy(string_attr,cbuf);
		if(aps_flag>0){		//if STRING
#ifdef scan_debug
			printf("%s\tSTRING\n",string_attr);
#endif
			return TSTRING;
		}
		else if((com_flag1)>0 || (com_flag2)>0){		//if COMMENT
#ifdef scan_debug
			printf("%s\tCOMMENT\n",string_attr);
#endif
			return scan();
		}
		else if(string_flag==1){
			for(token_num=KEYWORDSIZE-1;token_num>=0; token_num--){
				if(strcmp(string_attr,key[token_num].keyword)==0){	//if keyword
#ifdef scan_debug
					printf("%s\tkeyword\n",string_attr);
#endif
					return key[token_num].keytoken;
				}
			}
#ifdef scan_debug
			printf("%s\tNAME\n",string_attr);			//if NAME
#endif
			return TNAME;
		}
		else{						//if sing
#ifdef scan_debug
			printf("%s\tsign\n",string_attr);
#endif
			for(token_num=SIGNSIZE-1;token_num>=0; token_num--){
				if(strcmp(string_attr,signs[token_num].keyword)==0)
					return signs[token_num].keytoken;
			}
		}
	}
	return scan();
}
int get_linenum(){
	return linenum;
}
void error_scan(char *message){
	printf("%d scan error :\t%s\n",get_linenum(),message);
	end_scan();
}

void end_scan(){
	fclose(fp);
	printf("end_scan");
	exit(0);
}

