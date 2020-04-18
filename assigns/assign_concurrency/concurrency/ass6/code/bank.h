#ifndef _BANK_H
#define _BANK_H

#include <pthread.h>

typedef enum {false, true} bool; 

typedef struct Bank {
  unsigned int numberBranches;
  struct       Branch  *branches;
  struct       Report  *report;

  //reporting part
  int num_active_workers;
  //bool last_thread_finished_report; 
  pthread_mutex_t report_mutex; 
  pthread_cond_t report_cond;

  //transfer lock part
  //This lock is used for making two locks atomic
  pthread_mutex_t meta_lock;

  //branch balance
  pthread_mutex_t bank_balance_mutex; 
  pthread_mutex_t branch_update_mutex; 
  
} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);



#endif /* _BANK_H */
