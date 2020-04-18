#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>


void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert (elemSize > 0);
    v->elemSize = elemSize; 
    v->logLength = 0; 
    assert (initialAllocation >= 0); 
    if (initialAllocation == 0){
        initialAllocation = 20; 
    }
    v->allocLength = initialAllocation; 
    v->elems = malloc(v->allocLength * elemSize);
    v->freeFn = freeFn; 
    assert (v->elems != NULL); 
}

void VectorDispose(vector *v)
{
    if (v->freeFn != NULL){
        for (int i = 0; i < v->logLength; i++){
            v->freeFn((char *)v->elems + i * v->elemSize); 
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{ 
    return v->logLength; 
}

void *VectorNth(const vector *v, int position)
{ 
    assert (position >= 0 && position <= v->logLength - 1);
    return (void *)((char*)v->elems + position * v->elemSize); 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert (position >= 0 && position <= v->logLength - 1);
    char *replacePos = (char *)v->elems + position * v->elemSize;
    v->freeFn(replacePos);
    memcpy(replacePos, elemAddr, v->elemSize); 
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert (position >= 0 && position <= v->logLength);
    if (v->logLength == v->allocLength) {
        v->allocLength *= 2;
        v->elems = realloc(v->elems, v->allocLength * v->elemSize);
        assert (v->elems != NULL);
    }
    char *end = (char *)v->elems + v->logLength * v->elemSize;
    char *insertionPos = (char *)v->elems + position * v->elemSize;
    int sizeToMove = end - insertionPos;
    char buffer[sizeToMove];
    memcpy(buffer, insertionPos, sizeToMove);
    memmove(insertionPos, elemAddr, v->elemSize);
    memmove(insertionPos + v->elemSize, buffer, sizeToMove);
    v->logLength += 1; 
}

void VectorAppend(vector *v, const void *elemAddr)
{
    // void *destAddr;
    // if (v->logLength == v->allocLength){
    //     v->allocLength *= 2;
    //     v->elems = realloc(v->elems, v->allocLength * v->elemSize);
    //     assert (v->elems != NULL);
    // }
    // destAddr = (char*)v->elems + v->logLength * v->elemSize;
    // memcpy(destAddr, elemAddr, v->elemSize); 
    // v->logLength += 1; 
    assert (elemAddr != NULL); 
    VectorInsert(v, elemAddr, v->logLength);
}

void VectorDelete(vector *v, int position)
{
    assert (position >= 0 && position <= v->logLength - 1);
    char *deletePos = (char *)v->elems + position * v->elemSize;
    char *shiftPos = deletePos + v->elemSize;
    char *end = (char *)v->elems + v->logLength * v->elemSize; 
    int shiftSize = end - shiftPos; 
    v->freeFn(deletePos);
    memmove(deletePos, shiftPos, shiftSize); 
    v->logLength -= 1; 
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert (compare != NULL);
    qsort(v->elems, v->logLength, v->elemSize, compare); 
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert (mapFn != NULL);
    for (int i = 0; i < v->logLength; i++) {
        mapFn(VectorNth(v, i), auxData); 
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
    assert (startIndex >= 0 && startIndex <= v->logLength); 
    assert (key != NULL && searchFn != NULL); 
    void *base = (char *)v->elems + startIndex * v->elemSize;
    void *found;
    if (isSorted) {
        found = bsearch(key, base, v->logLength - startIndex, v->elemSize, searchFn);
    } else {
        size_t nMemb = v->logLength - startIndex;
        found = lfind(key, base, &nMemb, v->elemSize, searchFn); 
    }
    if (found == NULL) return kNotFound; 
    return ((char *)found - (char *)v->elems) / v->elemSize; 
} 
