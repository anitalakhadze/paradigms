#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert (elemSize > 0 && numBuckets > 0);
	assert (hashfn != NULL && comparefn != NULL); 
	h->nBuckets = numBuckets;
	h->elemSize = elemSize;
	h->nEntries = 0;
	h->hashFn = hashfn;
	h->compFn = comparefn;
	h->freeFn = freefn; 
	h->hashTable = malloc(numBuckets * sizeof(vector *));
	for (int i = 0; i < h->nBuckets; i++){
		vector *v = malloc(sizeof(vector));
		VectorNew(v, h->elemSize, h->freeFn, 10);
		h->hashTable[i] = v;
	}
}

void HashSetDispose(hashset *h)
{
	if (h->freeFn != NULL){
		for (int i = 0; i < h->nBuckets; i++){
			VectorDispose(h->hashTable[i]);
			free(h->hashTable[i]);
		}
	}
	free(h->hashTable);
}

int HashSetCount(const hashset *h)
{ 
	return h->nEntries; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert (mapfn != NULL);
	for (int i = 0; i < h->nBuckets; i++) {
		VectorMap(h->hashTable[i], mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert (elemAddr != NULL);
	int bucketIdx = h->hashFn(elemAddr, h->nBuckets);
	assert (bucketIdx >= 0 && bucketIdx < h->nBuckets);
	int insertPos = VectorSearch(h->hashTable[bucketIdx], elemAddr, h->compFn, 0, true);
	if (insertPos == -1){
		VectorAppend(h->hashTable[bucketIdx], elemAddr);
		VectorSort(h->hashTable[bucketIdx],h->compFn);
		h->nEntries++; 
	} else {
		VectorReplace(h->hashTable[bucketIdx], elemAddr, insertPos);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	assert (elemAddr != NULL);
	int bucketIdx = h->hashFn(elemAddr, h->nBuckets);
	assert (bucketIdx >= 0 && bucketIdx < h->nBuckets); 
	int position = VectorSearch(h->hashTable[bucketIdx], elemAddr, h->compFn, 0, true); 
	if (position != -1) {
		return VectorNth(h->hashTable[bucketIdx], position);
	}
	return NULL; 
}
