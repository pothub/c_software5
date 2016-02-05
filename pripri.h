#ifndef PRIPRI_H_
#define PRIPRI_H_

int p_error(char *mes);
void PrittyPrint(int PPtoken,int newline,int StartWord);
void once_enter();
int prase();

int p_program();
int block();
int ValiableDeclaration();
int ValiableSequence(int top);
int Type();
int NomalType();
int ArrayType();
int SubProgram();
int FormalArgument();
int CompositeSentence();
int Sentence();
int AssignmentSentence();
int DivergencySentence();
int LoopSentence();
int CallSentence();
int FomulaSequence();
int InputSentence();
int OutputSentence();
int OutputSpecification();
int Value(int top);
int Fomula();
int SimplicityFomula();
int Kou();
int Factor();
int Constant();
int MultiplOperator();
int AddOperator();
int RelationalOperator();

extern int Input_f;
extern int tabnum;
#endif
