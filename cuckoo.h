#ifndef _cuckoo_H
#define _cuckoo_H

#define REALLOC_RATIO_1 1
#define REALLOC_RATIO_2 2
#define REALLOC_RATIO_3 0.5f
#define CALLOC(type, n) calloc((n), sizeof(type))
#define MALLOC(type, n) malloc((n) * sizeof(type))
#define REALLOC(data, type, n) realloc(data, n * sizeof(type))
#define KEY_CMP(s1, s2) strcmp((const char *)s1, (const char *)s2)
#define EXIT_IF(expr) \
	do { \
		if (expr) { \
			fprintf(stderr, "\n%s:%d: In function '%s': %s\n\n", \
				__FILE__, __LINE__, __FUNCTION__, strerror(errno)); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

#define FUNCTION_SIZE 128
#define FUNC_INIT(arr) \
	{ \
		for (int i=0; i<FUNCTION_SIZE; i++) \
			arr[i] = ((int)rand() << 1) + 1; \
	} 

typedef struct Data Data;
typedef struct HashTable HashTable;

struct Data{
	char *key;
	int value;
};

struct HashTable {
	Data *t1;  				/*  Pointer to hash table 1 */
	Data *t2;  				/*  Pointer to hash table 2 */
	unsigned int size; 		/*  Actual size */
	unsigned int capasity;  /*  Capasity of hash table */
	unsigned int max_chain; /*  Max iterations in insertion */
	unsigned int shift;
	unsigned int *h1;				/*  Hash function 1 */
	unsigned int *h2;				/*  Hash function 2 */
}; 

HashTable* initTable(int capasity);
int insert(HashTable *ht, const char *key, int value);
void printTable(HashTable *ht);
int lookup(HashTable *ht, const char *key);
int delete(HashTable *ht, const char *key);
Data* get(HashTable *ht, const char *key);

char* lookup2(HashTable *ht, const char *key);
char** findDubles(HashTable *ht);

#endif  /* _cuckoo_H */