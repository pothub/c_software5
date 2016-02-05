/* token-list.h  */

#ifndef SCAN_H_
#define SCAN_H_

#include <stdio.h>
#include <stdlib.h>
#define MAXSTRSIZE 1024

/* Token */
#define	TNAME		1
#define	TPROGRAM	2
#define	TVAR		3	
#define	TARRAY		4
#define	TOF			5
#define	TBEGIN		6
#define	TEND		7
#define	TIF			8
#define	TTHEN		9
#define	TELSE		10
#define	TPROCEDURE	11
#define	TRETURN		12
#define	TCALL		13
#define	TWHILE		14
#define	TDO			15
#define	TNOT		16
#define	TOR			17
#define	TDIV		18
#define	TAND		19
#define	TCHAR		20
#define	TINTEGER	21
#define	TBOOLEAN	22
#define	TREADLN		23
#define	TWRITELN	24
#define	TTRUE		25
#define	TFALSE		26
#define	TNUMBER		27
#define	TSTRING		28
#define	TPLUS		29
#define	TMINUS		30
#define	TSTAR		31
#define	TEQUAL		32
#define	TNOTEQ		33
#define	TLE			34
#define	TLEEQ		35
#define	TGR			36
#define	TGREQ		37
#define	TLPAREN		38
#define	TRPAREN		39
#define	TLSQPAREN	40
#define	TRSQPAREN	41
#define	TASSIGN		42
#define	TDOT		43
#define	TCOMMA		44
#define	TCOLON		45
#define	TSEMI		46
#define	TREAD		47
#define	TWRITE		48

#define NUMOFTOKEN	48

/* token-list.c */

#define KEYWORDSIZE	27
#define SIGNSIZE	18
extern struct KEY {
	char * keyword;
	int keytoken;
} key[KEYWORDSIZE];

struct KEY signs[SIGNSIZE];
struct KEY key[KEYWORDSIZE];

extern void error(char *mes);
int scan();
static FILE *fp;
extern int numtoken[NUMOFTOKEN+1];

/* scan.c */
extern int init_scan(char *filename);
extern int scan(void);
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern int get_linenum(void);
extern void end_scan(void);

void error_scan(char *message);
#endif /* SCAN_H_ */
