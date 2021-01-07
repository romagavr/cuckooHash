#include<stdio.h>
#include<stdlib.h>
#include"cuckoo.h"
#include"time.h"
#include<memory.h>
// For STRING TESTING

#define R_STRING_LEN 10 

static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";

char* randString(size_t len) {
    char *s = 0;
    if (len && (s = malloc(len +1))) {
	    memset(s, '\0', len+1);
	    for (int n = 0;n < len;n++)
		s[n] = charset[rand() % (int)(sizeof(charset) -1)];
    }
    return s;
}
/*
int testStrings(HashTable *hash) {
	srand(time(0));
	char *s = 0;
	int hashVal=0;
	for (int i=0; i<hash->capasity; i++){
		s = randString(R_STRING_LEN);
		hashVal = hashFunc(FIRST_HASHF, s, strlen(s));	
		tableInsert(hash, hashVal, s, "abc");
	}
}
*/
// For INT TESTING

int k1[] = {20, 50, 53, 75, 100, 67, 105, 3, 36, 39}; 
int k2[] = {20, 50, 53, 75, 100, 67, 105, 3, 36, 39, 6}; 

//

#define INITSIZE 10000
#define RANDS 10
void startTest(HashTable *ht){
	char *s = 0;
    char **strings = malloc(sizeof(void *) * INITSIZE);
    if (!strings) exit(EXIT_FAILURE);
    for(int i=0; i<INITSIZE; i++){
        s = randString(RANDS);
        strings[i] = malloc(RANDS + 1);
        memcpy(strings[i], s, RANDS + 1);
		insert(ht, s, 1);
        free(s);
    }

    char **r2 = findDubles(ht);
    if (r2) {
        printf("Error: Dubles exist\n");
        for (int i=0; i<ht->capasity*2; i++) if (r2[i]) {
            printf("%s\n", r2[i]);
        }
    }
    int res;
    int passed = 1;
    for (int i=0; i<INITSIZE; i++){
        res = lookup(ht, strings[i]);
        if (!res){
            printf("Not inserted %s\n", strings[i]);
            passed = 0;
        } 
    }
    printf("----Insert Test: ");
    if (passed) printf("Passed\n");
    else printf("Failed\n");

    int k, k2=0;
    char **delStrings = malloc(sizeof(void *) * INITSIZE);
    for (int i=0; i<INITSIZE/3; i++) {
        k = rand() % INITSIZE;
        if (strings[k]){
            s = strings[k];
            delete(ht, s);
            delStrings[k2] = malloc(RANDS + 1);
            memcpy(delStrings[k2], s, RANDS + 1);
            free(s);
            strings[k] = 0;
            k2++;
        }
    }

    {
        int passed = 1;
        for (int i=0; i<INITSIZE; i++) 
            if (strings[i] && !lookup(ht, strings[i])) {
                    printf("Key does not exists: %s\n", strings[i]);
                    passed = 0;
            }
        for (int i=0; i<k2; i++) 
            if (lookup(ht, delStrings[i])) {
                    printf("Key did not deleted: %s\n", delStrings[i]);
                    passed = 0;
            }
        printf("----Delete Test: ");
        if (passed) printf("Passed\n");
        else printf("Failed\n");
    }

	//pprintf("----First Test-----\n");
	/*int k1s = sizeof k1 / sizeof(int);
	for (int i=0; i<k1s; i++) {
		tableInsertInt(hash,k1[i]);
	}*/
}

int main(void){
	srand(time(0));
	HashTable *ht = initTable(INITSIZE);
    startTest(ht);
	//printTable(ht);
    return 0;
}