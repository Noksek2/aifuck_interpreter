#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
const int AIFUCK_MAX_FILE = 1024 * 1024;
const int AIFUCK_HEAVY_FILE = 1024 * 16;

typedef enum aifuck_t{
	_err,
	_gt, // agi <
	_lt, // asi >
	_add, // llm + 
	_sub, // decels -
	_dot, //hype .
	_com, //slop ,
	_arr, //singularity [
	_arr_e, //doom ]
} aifuck_t;
aifuck_t aifuck_findmap(const char* key, size_t keylen){
	switch(keylen){
	case 3:
		if(strncmp(key, "agi", keylen) == 0)
			return _gt;
		else if(strncmp(key, "asi", keylen) == 0)
			return _gt;
		else if(strncmp(key, "llm", keylen) == 0)
			return _add;
	break;
	case 4:
		if(strncmp(key, "hype", keylen) == 0)
			return _dot;
		if(strncmp(key, "slop", keylen) == 0)
			return _com;
	break;
	case 6:
		if(strncmp(key, "decels", keylen) == 0)
			return _sub;
	break;
	case 11:
		if(strncmp(key, "singularity", keylen) == 0)
			return _arr;
	break;
	}
	return _err;
}
#define GET 
aifuck_t aifuck_gettok(char** ch){
	return _err;
}
#undef GET
struct AFfilebuf{
	char* source;
};
bool aifuck_getfilecontent(char** ch, size_t len, char* filename){
	FILE* fp;
	fp = fopen(filename, "rb");
	if(fp == NULL){
		perror(">:( wrong file you asihole \n");
		return false;
	}
	fseek(fp,0, SEEK_END);
	const long szfile = ftell(fp);
	if(szfile > AIFUCK_MAX_FILE){
		printf(">:( Hey stop i hate heavy file (%d Byte > MAX %d Byte)\n", szfile, AIFUCK_MAX_FILE);
		return false;
	}
	else if(szfile > AIFUCK_HEAVY_FILE){
		printf("|:/ why this code so heavy? (%d Byte > HEAVY %d Byte)\n", szfile, AIFUCK_HEAVY_FILE);
	}
	*ch = (char*)calloc(1, szfile + 1);
	if(*ch == NULL){
		perror(">:( memory alloced failed. whats wrong with your RAM? \n");
		return false;
	}
	fseek(fp, 0, SEEK_SET),
	fread(*ch, szfile, 1, fp);
	fclose(fp);
	return true;
}
int main(int argc, char** argv){
	
	if(argc==1){
		perror(">:( you should input file you .pussikat \n");
		return 1;
	}
	char* source;
	if(!aifuck_getfilecontent(&source, argv[1])){
		return 1;
	}
	if(!aifuck_parse(source)){
		perror(">:( haha your file is so wired to parse \n");
		return 1;
	}
	
	free(source);
	
	return 0;
}
