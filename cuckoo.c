#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<errno.h>
#include<math.h>
#include<assert.h>
#include"cuckoo.h"

static HashTable* initTable_s(int capasity){
	HashTable *hash = CALLOC(HashTable,1);
	EXIT_IF(!hash);
	hash->size = 0;
	hash->capasity = capasity;
	hash->t1 = CALLOC(Data, capasity);
	EXIT_IF(!hash->t1);
	hash->t2 = CALLOC(Data, capasity);
	EXIT_IF(!hash->t2);
	hash->h1 =MALLOC(int, FUNCTION_SIZE);
	EXIT_IF(!hash->h1);
	hash->h2 = MALLOC(int, FUNCTION_SIZE);
	EXIT_IF(!hash->h2);

	FUNC_INIT(hash->h1);
	FUNC_INIT(hash->h2);
	
	hash->max_chain = 4 + (int)(4 * log(capasity) / log(2) + 0.5);
	hash->shift = 32 - (int)(log(capasity) / log(2) + 0.5);

	return hash;
}

HashTable* initTable(int capasity) {
	srand(time(0));
	return initTable_s(capasity);
}

void printTable(HashTable *ht){
	for (int i = 0; i < ht->capasity; i++) {
		if (ht->t1[i].key)
			printf("%s\t%d\n", ht->t1[i].key, ht->t1[i].value);

		if (ht->t2[i].key)
			printf("%s\t%d\n", ht->t2[i].key, ht->t2[i].value);
	}
}

//The XOR function is 50% 0 and 50% 1, therefore it is good for combining uniform probability distributions.
static unsigned long hash(unsigned int h[], unsigned int shift, unsigned int capasity, const char *key) {
	unsigned long hash = 0UL;
	int i = 0;
	for (; key[i]; i++)
		hash ^= (unsigned int)(h[(i % FUNCTION_SIZE)] * key[i]);
	return ((unsigned int)hash >> shift) % capasity;
}

static int rehashInsert(HashTable *ht, char *key, int value) {
	unsigned long hk,hk2;
	Data x;
	for (int i=0; i<ht->max_chain; i++) {
		hk = hash(ht->h1, ht->shift, ht->capasity, key);
		if (!ht->t1[hk].key) {
			ht->t1[hk].key = key;
			ht->t1[hk].value = value;
			return 1;
		}
		hk2 = hash(ht->h2, ht->shift, ht->capasity, ht->t1[hk].key);
		if (!ht->t2[hk2].key) {
			ht->t2[hk2] = ht->t1[hk];
			ht->t1[hk].key = key;
			ht->t1[hk].value = value;
			return 1;
		}
		x = ht->t2[hk2];
		ht->t2[hk2] = ht->t1[hk];
		ht->t1[hk].key = key;
		ht->t1[hk].value = value;
		key = x.key;
		value = x.value;
	}
	memset(ht->t1, 0, sizeof(Data) * ht->capasity);
	memset(ht->t2, 0, sizeof(Data) * ht->capasity);
	FUNC_INIT(ht->h1);
	FUNC_INIT(ht->h2);
	return 0;
}

static void rehash(HashTable **ht, float sizeRatio) {
	HashTable *htt = *ht;
	HashTable *htn = initTable_s(htt->capasity * sizeRatio);
	for (int i=0; i < htt->capasity; i++) {
		if (htt->t1[i].key && !rehashInsert(htn, htt->t1[i].key, htt->t1[i].value)){
			i = -1;
			continue;
		}
		if (htt->t2[i].key && !rehashInsert(htn, htt->t2[i].key, htt->t2[i].value))
			i = -1;
	}
	free(htt->t1);
	free(htt->t2);
	free(htt->h1);
	free(htt->h2);
	htn->size = htt->size;
	**ht = *htn;
	free(htn);
}

int insert(HashTable *ht, const char *key, int value) {
	unsigned int hk = hash(ht->h1, ht->shift, ht->capasity, key);
	if (ht->t1[hk].key && !KEY_CMP(ht->t1[hk].key, key)) {
		ht->t1[hk].value = value;
		return 0;
	}

	unsigned int hk2 = hash(ht->h2, ht->shift, ht->capasity, key);
	if (ht->t2[hk2].key && !KEY_CMP(ht->t2[hk2].key, key)) {
		ht->t2[hk2].value = value;
		return 0;
	}

	Data x, tmp;
	int keyLen = strlen((const char *)key)+1;
	x.key = MALLOC(char, keyLen);
	EXIT_IF(!x.key);
	strncpy(x.key, (const char *)key, keyLen);
	x.value = value;
	
	for (int i=0; i < ht->max_chain; i++) {
		if (!ht->t1[hk].key){
			ht->t1[hk] = x;
			if (ht->capasity < ++ht->size)	
				rehash(&ht, REALLOC_RATIO_2);
			return 1;
		}

		hk2 = hash(ht->h2, ht->shift, ht->capasity, ht->t1[hk].key);
		if (!ht->t2[hk2].key){
			ht->t2[hk2] = ht->t1[hk];
			ht->t1[hk] = x;
			if (ht->capasity < ++ht->size)	
				rehash(&ht, REALLOC_RATIO_2);
			return 1;
		}

		tmp = ht->t1[hk];
		ht->t1[hk] = x;
		x = ht->t2[hk2];
		ht->t2[hk2] = tmp;
		hk = hash(ht->h1, ht->shift, ht->capasity, x.key);
	}
	// < 5 * (2 * ht->capasity) / 12
	if (ht->size < 5/6 * ht->capasity)
		rehash(&ht, REALLOC_RATIO_1);
	else rehash(&ht, REALLOC_RATIO_2);
	insert(ht, x.key, x.value);
	free(x.key);
	return 1;	
}

int lookup(HashTable *ht, const char *key) {
	unsigned long hk = hash(ht->h1, ht->shift, ht->capasity, key);
	if (ht->t1[hk].key && !KEY_CMP(ht->t1[hk].key, key))
		return 1;
	hk = hash(ht->h2, ht->shift, ht->capasity, key);
	if (ht->t2[hk].key && !KEY_CMP(ht->t2[hk].key, key))
		return 1;
	return 0;
}

Data* get(HashTable *ht, const char *key) {
	unsigned long hk = hash(ht->h1, ht->shift, ht->capasity, key);
	if (ht->t1[hk].key && !KEY_CMP(ht->t1[hk].key, key))
		return &ht->t1[hk];
	hk = hash(ht->h2, ht->shift, ht->capasity, key);
	if (ht->t2[hk].key && !KEY_CMP(ht->t2[hk].key, key))
		return &ht->t2[hk];
	return 0;
}

int delete(HashTable *ht, const char *key) {
	unsigned long hk = hash(ht->h1, ht->shift, ht->capasity, key);
	if (ht->t1[hk].key && !KEY_CMP(ht->t1[hk].key, key)){
		free(ht->t1[hk].key);
		memset(&ht->t1[hk], 0, sizeof(ht->t1[hk]));
		if (--ht->size < (2 * ht->capasity)/5) rehash(&ht, REALLOC_RATIO_3);
		return 1;
	}
	hk = hash(ht->h2, ht->shift, ht->capasity, key);
	if (ht->t2[hk].key && !KEY_CMP(ht->t2[hk].key, key)){
		free(ht->t2[hk].key);
		memset(&ht->t2[hk], 0, sizeof(ht->t2[hk]));
		if (--ht->size < (2 * ht->capasity)/5) rehash(&ht, REALLOC_RATIO_3);
		return 1;
	}
	return 0;
}

void destroyTable(HashTable *ht) {
	for (int i=0; i < ht->capasity; i++) {
		if (ht->t1[i].key) free(ht->t1[i].key);
		if (ht->t2[i].key) free(ht->t2[i].key);
	}
	free(ht->t1);
	free(ht->t2);
	free(ht->h1);
	free(ht->h2);
	free(ht);
}

char* lookup2(HashTable *ht, const char *key) {
	unsigned long hk = hash(ht->h1, ht->shift, ht->capasity, key);
	if (ht->t1[hk].key && !KEY_CMP(ht->t1[hk].key, key))
		return ht->t1[hk].key;
	hk = hash(ht->h2, ht->shift, ht->capasity, key);
	if (ht->t2[hk].key && !KEY_CMP(ht->t2[hk].key, key))
		return ht->t2[hk].key;
	return 0;
}

char** findDubles(HashTable *ht){
	char **res = malloc(sizeof(void *) * ht->capasity * 2);
	int k = 0;
	char *srch = 0;
	memset(res, 0, sizeof(void *) * ht->capasity*2);
	for(int i=0; i<ht->capasity; i++){
		if (ht->t1[i].key){
			srch = ht->t1[i].key;
			for (int j = 0; j < ht->capasity; j++) {
				if (i != j && ht->t1[j].key && !strcmp(srch, ht->t1[j].key)) {
					res[k] = malloc(strlen(srch) + 1);
					memcpy(res[k], srch, strlen(srch) + 1);
					k++;
					break;
				}
				if (ht->t2[j].key && !strcmp(srch, ht->t2[j].key)) {
					res[k] = malloc(strlen(srch) + 1);
					memcpy(res[k], srch, strlen(srch) + 1);
					k++;
					break;
				}
			}
		}
		if (ht->t2[i].key){
			srch = ht->t2[i].key;
			for (int j = 0; j < ht->capasity; j++) {
				if (i != j && ht->t2[j].key && !strcmp(srch, ht->t2[j].key)) {
					res[k] = malloc(strlen(srch) + 1);
					memcpy(res[k], srch, strlen(srch) + 1);
					k++;
					break;
				}
				if (ht->t1[j].key && !strcmp(srch, ht->t1[j].key)) {
					res[k] = malloc(strlen(srch) + 1);
					memcpy(res[k], srch, strlen(srch) + 1);
					k++;
					break;
				}
			}
		}
	}
	return (k>0) ? res : 0;
}