#ifndef COMPILER_H_
#define COMPILER_H_

int initCompiler(char *mplname);
void CompilePrintStack(char *str);
void CompileOutputLine();
void CompileOutputCSL(char *str);
int CompileGetLabel();
void CompileStack(int l, char *str, int num_string);
void CompileOutputString();
void CompileDeclareStack(Id_c **id);
void CompileLD_LAD(Id_c **id);
void CompileFormalPS(Id_c **id);
void CompilerLibrary();

extern int label_num;
#endif
