#include <stdio.h>
#include <process.h>
#include <windows.h>

#if defined(CC)
	#define C "gcc.exe"
	#define E "MPICC"
#elif defined(CXX)
	#define C "g++.exe"
	#define E "MPICXX"
#elif defined(FC)
	#define C "gfortran.exe"
	#define E "MPIFORT"
#else
	#error
#endif

#define SZ 32767

int main(int argc, char** argv) {
	// Get executable path
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	int s = strlen(path);
	// Strip exe filename and move up one folder to go out of the "bin" directory
	while (path[s] != '\\' && path[s] != '/') --s; --s;
	while (path[s] != '\\' && path[s] != '/') --s;
	path[s] = '\0';
	// Force forward slash as directory separator
	s = strlen(path);
	while (s >= 0) if (path[s--] == '\\') path[s + 1] = '/';
	// Set library and include directories
	char* lpath = malloc(SZ * sizeof(char));
	snprintf(lpath, SZ, "-L%s/lib", path);
	char* ipath = malloc(SZ * sizeof(char));
	snprintf(ipath, SZ, "-I%s/include", path);
	// Get compiler name and check for the presence of the "show" argument
	char* comp = malloc(SZ * sizeof(char));
	int show = (argc == 2 && strcmp(argv[1], "-show") == 0);
	// Fill compiler arguments
	char** args = malloc(SZ * sizeof(char*));
	int i = 0;
	args[i++] = GetEnvironmentVariable(E, comp, SZ) > 0 ? comp : C;
	args[i++] = ipath;
#ifdef FC
	// Workaround for GFortran
	args[i++] = "-fno-range-check";
#if __GNUC__ >= 10
	// More workarounds for GFortran 10 increased strickness
	args[i++] = "-fallow-invalid-boz";
	args[i++] = "-fallow-argument-mismatch";
#endif
#endif
	if(!show) for(int x = 1; x < argc; ++x) args[i++] = argv[x];
	args[i++] = lpath;
	args[i++] = "-lmsmpi";
	args[i] = NULL;
	if(show) {
		for(int x = 0; args[x]; ++x) printf("%s ", args[x]);
		printf("\n");
		fflush(stdout);
		return 0;
	} else {
		return _spawnvp(_P_WAIT, args[0], (const char* const*)args);
	}
};
