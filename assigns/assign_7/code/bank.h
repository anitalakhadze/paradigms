#ifndef _BANK_H
#define _BANK_H

#include <pthread.h>

typedef struct Bank {
  unsigned int numberBranches;
  struct       Branch  *branches;
  struct       Report  *report;

  //reporting 
  int num_active_workers;
  pthread_mutex_t report_mutex;
  pthread_cond_t report_cond; 

  //meta lock
  pthread_mutex_t meta_lock; 
} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);



#endif /* _BANK_H */
