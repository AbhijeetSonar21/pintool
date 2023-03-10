#include "pin.H"

#define MAIN "main"
#define FILENO "fileno"

// Taint the memory if the source of input is stdin
#define FGETS "fgets"
#define GETS "gets"

// Propagate if the src is tainted
#define STRCPY "strcpy@plt"
#define STRNCPY "strncpy@plt"
#define STRCAT "strcat@plt"
#define STRNCAT "strncat@plt"
#define MEMCPY "memcpy@plt"

// Reset tainted memory
#define BZERO "bzero@plt"
#define MEMSET "memset@plt"

using namespace std;

typedef int ( *FP_FILENO )(FILE*);
FP_FILENO org_fileno;

INT32 Usage()
{
		return -1;
}

bool isStdin(FILE *fd)
{
		int ret = org_fileno(fd);
		if(ret == 0) return true;
		return false;
}

bool fgets_stdin = false;
VOID fgetsTail(char* ret)
{
		if(fgets_stdin) {
				printf("fgetsTail: ret %p\n", ret);
		}
		fgets_stdin = false;
}

VOID fgetsHead(char* dest, int size, FILE *stream)
{
		if(isStdin(stream)) {
				printf("fgetsHead: dest %p, size %d, stream: stdin)\n", dest, size);
				fgets_stdin = true;
		} 
}

VOID getsTail(char* dest)
{
		printf("getsTail: dest %p)\n", dest);
  printf("size of dest: %d\n", strlen(dest));
}

VOID mainHead(int argc, char** argv)
{

}

VOID strcpyHead(char* dest, char* src)
{

}

VOID bzeroHead(void* dest, int n)
{

}


VOID Image(IMG img, VOID *v) {
		RTN rtn;

		rtn = RTN_FindByName(img, FGETS);
		if(RTN_Valid(rtn)) {
				RTN_Open(rtn);
				RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)fgetsHead, 
								IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
								IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
								IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
								IARG_END);

				RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)fgetsTail, 
								IARG_FUNCRET_EXITPOINT_VALUE,
								IARG_END);
				RTN_Close(rtn);
		}

		rtn = RTN_FindByName(img, GETS);
		if(RTN_Valid(rtn)) {
				RTN_Open(rtn);
				RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)getsTail, 
								//IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
								IARG_FUNCRET_EXITPOINT_VALUE,
								IARG_END);
				RTN_Close(rtn);
		}

		rtn = RTN_FindByName(img, STRCPY);
		if(RTN_Valid(rtn)) {
				RTN_Open(rtn);
				RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)strcpyHead, 
								IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
								IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
								IARG_END);
				RTN_Close(rtn);
		}

	rtn = RTN_FindByName(img, BZERO);
		if(RTN_Valid(rtn)) {
				RTN_Open(rtn);
				RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)bzeroHead, 
								IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
								IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
								IARG_END);
				RTN_Close(rtn);
		}

		rtn = RTN_FindByName(img, MAIN);
		if(RTN_Valid(rtn)) {
				RTN_Open(rtn);
				RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)mainHead, 
								IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
								IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
								IARG_END);
				RTN_Close(rtn);
		}


		rtn = RTN_FindByName(img, FILENO);
		if(RTN_Valid(rtn)) {
				RTN_Open(rtn);
				AFUNPTR fptr = RTN_Funptr(rtn);
				org_fileno = (FP_FILENO)(fptr);
				RTN_Close(rtn);
		}
}

int main(int argc, char *argv[])
{
  PIN_InitSymbols();

		if(PIN_Init(argc, argv)){
				return Usage();
		}
		
  IMG_AddInstrumentFunction(Image, 0);
		PIN_StartProgram();

		return 0;
}

