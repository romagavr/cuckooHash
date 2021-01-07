#include<stdio.h>
#include<stdlib.h>
#include"cuckoo.h"
#include"time.h"
#include<memory.h>

#define R_STRING_LEN 10 
#define INITSIZE 10000
#define RANDS 10

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

void startTest(HashTable *ht){
	char *s = 0;
    char **strings = malloc(sizeof(void *) * INITSIZE);
    if (!strings) exit(EXIT_FAILURE);
    for(int i=0; i<INITSIZE; i++){
        s = randString(RANDS);
        strings[i] = s;
		insert(ht, s, 1);
    }

    int passed = 1;
    char **r2 = findDubles(ht);
    if (r2) {
        printf("Error: doubling exist\n");
        for (int i=0; i<ht->capasity*2; i++) if (r2[i]) {
            printf("%s\n", r2[i]);
        }
        passed =0;
    } else {
        for (int i=0; i<INITSIZE; i++) if (!lookup(ht, strings[i])){
            printf("Not inserted %s\n", strings[i]);
            passed = 0;
        } 
    }
    printf("----Insert Test: %s\n", passed ? "Passed" : "Failed");

    int k, k2=0;
    passed = 1;
    char **delStrings = malloc(sizeof(void *) * INITSIZE);
    if (!delStrings) exit(EXIT_FAILURE);
    for (int i=0; i<INITSIZE/3; i++) {
        k = rand() % INITSIZE;
        if (strings[k]){
            s = strings[k];
            strings[k] = 0;
            delete(ht, s);
            delStrings[k2++] = s;
        }
    }

    for (int i=0; i<INITSIZE; i++) 
        if (strings[i] && !lookup(ht, strings[i])) {
                printf("Key does not exists (wrong Deleted): %s\n", strings[i]);
                passed = 0;
        }
    for (int i=0; i<k2; i++) 
        if (lookup(ht, delStrings[i])) {
                printf("Key did not deleted: %s\n", delStrings[i]);
                passed = 0;
        }
    printf("----Delete Test: %s\n", passed ? "Passed" : "Failed");

    for(int i=0; i<INITSIZE; i++)
        if (strings[i]) free(strings[i]);
    for(int i=0; i<k2; i++)
        free(delStrings[i]);
    free(strings);
    free(delStrings);
}

int main(void){
	srand(time(0));
	HashTable *ht = initTable(INITSIZE);
    startTest(ht);
	//printTable(ht);
    destroyTable(ht);
    return 0;
}