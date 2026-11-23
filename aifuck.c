#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define AIFUCK_MAX_FILE  (1024 * 1024)
#define AIFUCK_HEAVY_FILE  (1024 * 16)
#define AIFUCK_MAX_JMPMAP (256) //~64KB
#define AIFUCK_MAX_JMPST  (256)

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
	
	vm_incn,//inc 1010
	vm_decn,//dec 1010
	vm_ujmp,//
	vm_jmp,//
} aifuck_t;

typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef struct AFfilebuf{
	char* source;
	uchar_t* runcode;
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
uint_t g_jmpmap[AIFUCK_MAX_JMPMAP][2];//end , start
uint_t g_jmpid = 0;
const int JMPEND = 0;
const int JMPSTART = 1;

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
	case _gt: return "m_inc";// agi >
	case _lt: return "m_dec";// asi <
	case _add: return "inc"; // llm + 
	case _sub: return "dec"; // decels -
	case _dot: return "put"; //hype .
	case _com: return "get"; //slop ,
	case _arr: return "["; //singularity [
	case _arr_e: return "]"; //doom ]
	case _comment: return "#";
	case _eof: return "eof";
	case vm_jmp: return "vm_jmp";
	case vm_ujmp: return "vm_ujmp";
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
bool aifuck_parse(AFfilebuf* fb){
	//puts(fb->source);
	int jmpst[AIFUCK_MAX_JMPST];
	memset(jmpst, 0, sizeof(int) * AIFUCK_MAX_JMPST);
	uint_t jmplen = 0;
	
	const char* org = fb->source;
	fb->runcode = (uchar_t*)calloc(fb->len + AIFUCK_MAX_JMPST, sizeof(char));
	printf("%llX>>\n", fb->runcode);
	fb->rlen = 0;
	if(fb->runcode == NULL) {
		perror(">:( memory alloced failed. whats wrong with your RAM? \n");
		return false;
	}
	aifuck_t tok;
	while(tok = aifuck_gettok(&fb->source), tok!=_eof){
		
		switch(tok){
		case _err:
			printf("[token error]\n");
			goto l_err;
		case _comment:
			while(1){
				switch(*fb->source){
				case '\0': goto l_fin;
				case '\n': fb->source++; goto l_redo;
				default: fb->source++; break;
				}
			}
			break;
		case _arr:
			if(jmplen >= AIFUCK_MAX_JMPST){
				goto l_err;
			}
			//ujmp
			fb->runcode[fb->rlen++]=vm_ujmp;
			fb->runcode[fb->rlen++]=g_jmpid;
			g_jmpmap[g_jmpid][JMPSTART] = fb->rlen;//[[][]] 
			fb->rlen++;
			
			
			jmpst[jmplen++]=g_jmpid;			
			g_jmpid++;
			break;
		case _arr_e:
			if(jmplen == 0){
				printf("[no '[' or 'singularity']\n");
			}
			const int id = jmpst[--jmplen];
			fb->runcode[fb->rlen++]=vm_jmp;
			fb->runcode[fb->rlen++]=id;
			g_jmpmap[id][JMPEND] = fb->rlen;
			break;
		default:
			if(fb->rlen<256){
				printf("%s\t", aifuck_tokey(tok));
			}
			fb->runcode[fb->rlen++] = (uchar_t)tok;
			break;
		};
		
	l_redo: ;
	}
l_fin:
	fb->runcode[fb->rlen++] = _eof;
	fb->source = org;
	return true;
l_err:
	free(fb->runcode);
	fb->source = org;
	return false;
}
void aifuck_dis_dump(AFfilebuf fb){
	for(int i=0;i<fb.rlen;i++){
		printf("%03d\t", i);
		switch(fb.runcode[i]){
		case vm_ujmp:{
			const uchar_t id = fb.runcode[i+1];
			printf("ujmp id=%d (%d)\n", id, g_jmpmap[id][JMPEND]);
			i++;
			break;
		}
		case vm_jmp:{
			const uchar_t id = fb.runcode[i+1];
			printf("jmp id=%d (%d)\n", id, g_jmpmap[id][JMPSTART]);
			i++;
			break;
		}
		default:
			printf("%s\n", aifuck_tokey(fb.runcode[i]));
		}
	}
}
bool aifuck_run(AFfilebuf fb){
	const int MAX_MEM = 256;
	const int MAX_STACK = 256;
	char mem[MAX_MEM];
	int currmem = 0;
	memset(mem, 0, MAX_MEM * sizeof(char));
	char* pc = fb.runcode;
	
	aifuck_dis_dump(fb);
	
	while(1){
		switch(*pc){
		case _gt:
			currmem++;
			if(currmem>=MAX_MEM){
				printf("runtime error >:( max mem");
				goto l_err;
			}
		break;
		case _lt:
			currmem--;
			if(currmem<0){
				printf("runtime error >:( mem < 0");
				goto l_err;
			}
			
		break;
		case _dot:
			putchar(mem[currmem]);
		break;
		case _com:
			mem[currmem] = getchar();
		break;
		case vm_ujmp:
			pc++;
			if(mem[currmem]==0){
				pc = fb.runcode + g_jmpmap[*pc][0];
				goto l_redo;
			}
			//[currmem]
		break;
		case vm_jmp:
			pc++;
			pc = fb.runcode + g_jmpmap[*(pc)][1];
			goto l_redo;
		case _eof:
			goto l_fin;
		}
		pc++;
	l_redo: ;
	}
l_fin:
	//fb.runcode = org_rcode;
	return true;
l_err:
	return false;
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
	
	if(!aifuck_parse(&fb)){
		perror(">:( your file is so wired to parse haha \n");
		free(fb.source);
		return 1;
	}
	if(!aifuck_run(fb)){
		puts("FAIL");
	}
	puts("SUCC");
	free(fb.runcode);
	free(fb.source);
	return 0;
}
//fuck 
