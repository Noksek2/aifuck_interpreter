#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
const int AIFUCK_MAX_FILE = 1024 * 1024;
const int AIFUCK_HEAVY_FILE = 1024 * 16;

typedef enum aifuck_t{
	_err,
	_gt, // agi >
	_lt, // asi <
	_add, // llm + 
	_sub, // decels -
	_dot, //hype .
	_com, //slop ,
	_arr, //singularity [
	_arr_e, //doom ]
	_comment,
	_eof,
} aifuck_t;


typedef struct AFfilebuf{
	char* source;
	char* runcode;
	size_t len;
	size_t rlen;
} AFfilebuf;


char g_buf[128]; 
static aifuck_t g_smap[128]={
	['>'] = _gt,
	['<'] = _lt,
	['+'] = _add,
	['-'] = _sub,
	['.'] = _dot,
	[','] = _com,
	['['] = _arr,
	[']'] = _arr_e,
	['#'] = _comment,
};
aifuck_t aifuck_findmap(const char* key, size_t keylen){
	switch(keylen){
	case 2:
		if(strncmp(key, "ai", keylen) == 0)
			return _comment;
	break;
	case 3:
		if(strncmp(key, "agi", keylen) == 0)
			return _gt;
		else if(strncmp(key, "asi", keylen) == 0)
			return _lt;
		else if(strncmp(key, "llm", keylen) == 0)
			return _add;
	break;
	case 4:
		if(strncmp(key, "hype", keylen) == 0)
			return _dot;
		else if(strncmp(key, "slop", keylen) == 0)
			return _com;
		else if(strncmp(key, "doom", keylen) == 0)
			return _arr_e;
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
const char* aifuck_tokey(aifuck_t t){
	switch(t){	
	case _gt: return ">";// agi >
	case _lt: return "<";// asi <
	case _add: return "+"; // llm + 
	case _sub: return "-"; // decels -
	case _dot: return "."; //hype .
	case _com: return ","; //slop ,
	case _arr: return "["; //singularity [
	case _arr_e: return "]"; //doom ]
	case _comment: return "#";
	case _eof: return "eof";
	}
	return "err";
}

bool aifuck_getfilecontent(char** ch, size_t* len, char* filename){
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
	*ch = (char*)calloc(szfile + 1, sizeof(char));
	if(*ch == NULL){
		perror(">:( memory alloced failed. whats wrong with your RAM? \n");
		return false;
	}
	fseek(fp, 0, SEEK_SET),
	fread(*ch, szfile, 1, fp);
	fclose(fp);
	
	*len = (size_t)szfile;
	return true;
}
#define GET 
#define ch (*buf)
aifuck_t aifuck_gettok(char** pch){
	char* buf = *pch;
	aifuck_t tok;
	while(isspace(ch)) buf++;
	if(ch == '\0'){
		*pch = buf;
		return _eof;
	}
	else if(ch>='a' && ch<='z'){
		const char* cch = buf; //better die
		buf++;
		
		while(ch>='a' && ch <='z') {
			buf++;
		}
		const size_t chlen = buf - cch;
		tok = aifuck_findmap(cch, chlen);
		*pch = buf;
		return tok;
	}
	else if(tok=g_smap[ch], tok != _err){
		buf++;
		*pch = buf;
		return tok;
	}
	*pch = buf;
	return _err;
}
#undef GET
#undef ch
bool aifuck_parse(AFfilebuf fb){
	//puts(fb.source);
	const char* org = fb.source;
	fb.runcode = (char*)calloc(fb.len, sizeof(char));
	fb.rlen = 0;
	if(fb.runcode == NULL) {
		perror(">:( memory alloced failed. whats wrong with your RAM? \n");
		return false;
	}
	aifuck_t tok;
	while(tok = aifuck_gettok(&fb.source), tok!=_eof){
		if(tok==_err){
			printf("[token error]\n");
			free(fb.runcode);
			return false;
		}
		else if(tok==_comment){
			while(1){
				switch(*fb.source){
				case '\0': goto l_fin;
				case '\n': fb.source++; goto l_redo;
				default: fb.source++; break;
				}
			}
		}
		printf("tok type %d => %s\n", tok, aifuck_tokey(tok));
		fb.runcode[fb.rlen++] = (char)tok;
	l_redo: ;
	}
l_fin:
	fb.runcode[fb.rlen++] = _eof;
	fb.source = org;
	puts("NON");
	return true;
}
bool aifuck_run(AFfilebuf fb){
	//const char* org_rcode = fb.runcode;
	const int MAX_MEM = 256;
	const int MAX_STACK = 256;
	char mem[MAX_MEM];
	int retpc[MAX_STACK];
	int currmem = 0;
	int retlen = 0;
	memset(mem, 0, MAX_MEM * sizeof(char));
	memset(retpc, 0, MAX_STACK * sizeof(int));
	char* pc = fb.runcode;
	while(1){
	switch(*pc){
	case _gt:
		currmem++;
		if(currmem>=MAX_MEM){
			printf("runtime error >:( max mem");
			goto l_fin;
		}
	break;
	case _lt:
		currmem--;
		if(currmem<0){
			printf("runtime error >:( mem < 0");
			goto l_fin;
		}
		
	break;
	case _dot:
		putchar(mem[currmem]);
	break;
	case _com:
	break;
	case _arr:
		//[currmem]
	break;
	case _arr_e:
	break;
	case _eof:
		goto l_fin;
	}
	pc++;
	}
l_fin:
	//fb.runcode = org_rcode;
	return true;
}
int main(int argc, char** argv){
	
	if(argc==1){
		perror(">:( you should input file you .pssikat \n");
		return 1;
	}
	AFfilebuf fb;
	
	if(!aifuck_getfilecontent(&fb.source, &fb.len, argv[1])){
		return 1;
	}
	printf("filesize : %llu\n", fb.len);
	
	if(!aifuck_parse(fb)){
		perror(">:( your file is so wired to parse haha \n");
		free(fb.source);
		return 1;
	}
	//if(!aifuck_run(fb)){
	//	return 1;
	//}
	puts("NON");
	free(fb.runcode);
	puts("NON");
	free(fb.source);
	puts("NON");
	return 0;
}
//fuck 
