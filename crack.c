
#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <crypt.h>

struct crackStruct {
	
	int threadNum;
	int size;
	int numThreads;
	char* _target;
	char* _salt;
	int j;
	char test[8];
	struct crypt_data cdata;
	char* ret;
	
};

void* cracker(void* args) {
	struct crackStruct* arg_ptr = (struct crackStruct*) args;
	
	for (arg_ptr->j ; arg_ptr -> j < arg_ptr->size; arg_ptr->j++) {
		if (arg_ptr->j == 0) arg_ptr-> test[arg_ptr->j] = 'a' + arg_ptr->threadNum;
		else arg_ptr-> test[arg_ptr->j] = 'a';
	}
	
	while (true) {
			
		bool allZ = true;
		
		arg_ptr -> j = 0;
		for (arg_ptr-> j; arg_ptr-> j < arg_ptr->size; arg_ptr-> j++) {
			if ( arg_ptr-> test[arg_ptr-> j] != 122) allZ = false;
		}
		if ( arg_ptr-> test[0] > 122) allZ = true;
			
		char* hash = crypt_r(arg_ptr-> test, arg_ptr-> _salt, &(arg_ptr->cdata) );
		
		if ( strcmp(hash, arg_ptr->_target) == 0 ) {
			printf("Password: %s\n", arg_ptr-> test);
			exit(0);
			//arg_ptr-> ret = arg_ptr-> test;
			//pthread_exit((void*)arg_ptr-> test);
			//pthread_exit(NULL);
		}

		if (allZ) pthread_exit(NULL);
				
		else {
			arg_ptr-> j = 1;
			bool done = false;
			while (!done) {
				if (arg_ptr-> j == arg_ptr->size) {
					arg_ptr-> test[0] += arg_ptr->numThreads;
					if (arg_ptr-> test[0] > 122) {
						pthread_exit(NULL);
					}
					done = true;
				}
				else if ( arg_ptr-> test[ arg_ptr->size - arg_ptr-> j ] < 122 ){
					arg_ptr-> test[ arg_ptr->size - arg_ptr-> j ] += 1;
					done = true;
				}
				else {
					arg_ptr-> test[ arg_ptr->size - arg_ptr-> j ] = 97;
					arg_ptr-> j++;
				}
			}	
		}
	} 	
}

int main( int argc, char* argv[] ) {
	int i;

	if (argc != 4) {
		printf("Usage: <threads> <keysize> <target>\n");
		return -1;
	}

	if ( atoi(argv[2]) > 8 ) {
		printf("Keysize cannot exceed 8\n");
		return -1;
	}

	int numThreads = atoi(argv[1]);
	int keySize = atoi(argv[2]);
	char* target = argv[3];
	
	char salt[2];
	memmove(salt, target, sizeof(salt) );
		
	pthread_t threads[8][numThreads];
	struct crackStruct myStructs[numThreads];
	struct crackStruct* struct_ptr[numThreads];
	int k;
	for (k = 1; k <= keySize; k++) {
		for (i = 0; i < numThreads; i++) {
			struct_ptr[i] = &myStructs[i];
			struct_ptr[i] -> threadNum = i;
			struct_ptr[i] -> size = k;
			struct_ptr[i] -> _target = target;
			struct_ptr[i] -> _salt = salt;
			struct_ptr[i] -> numThreads = numThreads;
			struct_ptr[i] -> j = 0;
			struct_ptr[i] -> cdata.initialized = 0;
			struct_ptr[i] -> ret = NULL;
			if ( pthread_create(&threads[k][i], NULL, cracker, struct_ptr[i]) != 0 ) {
				printf("error: thread not created\n");
				return -1;
			}
		}
	
		void* ret;
		for (i = 0; i < numThreads; i++) {
			pthread_join(threads[k][i], NULL );
		}
	}
	return 0;
}


