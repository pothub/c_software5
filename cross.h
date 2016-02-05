#ifndef CROSS_H_
#define CROSS_H_

#define TPINT 		TINTEGER
#define TPCHAR 		TCHAR
#define TPBOOL 		TBOOLEAN
#define TPARRAY		TARRAY
#define TPARRAYINT	TINTEGER + 100
#define TPARRAYCHAR	TCHAR + 100
#define TPARRAYBOOL	TBOOLEAN + 100
#define TPPROC		101

#define TOP 0
#define NEX 1

typedef struct TYPE {
	int ttype;		/* TPINT TPCHAR TPARRAY TPARRAYINT TPARRAYCHAR TPARRAYBOOL TPPROC */
	int arraysize;	/* size of array,if TPARRAY */
	struct TYPE *etp;	/* pointer to element type if TPARRAY */
	struct TYPE *paratp;	/* pointer to parameter's type list if ttype is TPPROC */
} Type_c;

typedef struct LINE {
	int reflinenum;
	struct LINE *nextlinep;
} Line_c;

typedef struct ID {
	char *name;
	char *procname;	/* procedure name which this name is defined */ /* NULL is global name */
	struct TYPE *itp;
	int ispara;		/* 1:formal parameter,0,else */
	int deflinenum;
	struct LINE *irefp;
	struct ID *nextp;
} Id_c;

extern Id_c *globalidroot;
extern char *procname_t;
extern int Head_Name_Num, Last_Name_Num;
extern int lastID;

void init_cross();
void RenoveGrobalId();
void PrintIdList(Id_c *Id);
void PrintId(Id_c *id);
void PrintIdName(Id_c *Id);
void PrintIdType(Id_c *id);
void _PrintIdType(char *str, Type_c *itp);
void PrintIdRef(Id_c *id);
void sprintParatp(char *str, Id_c *id);

void AddIdName(char *name, int top_next, int _ispara, Id_c **id);
void AddIdType(int token, Id_c **id);
void CompareID(Id_c **id);
int _CompareID(Id_c **id);
int __CompareID(Id_c **id);
Line_c *CreateLine(int reflinenum);
void SetFormalName();
void FreeProcName();
void AddFormal(int token, Id_c **id);
#endif
