#ifndef _Dictionary
#define _Dictionary
typedef void** Dictionary;
Dictionary newDictionary();
int addToDictionary(Dictionary, char* key, void* value);
char* getFromDictionary(Dictionary, char* key);
void deleteDictionary(Dictionary);
#endif