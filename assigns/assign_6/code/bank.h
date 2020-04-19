#ifndef _BANK_H
#define _BANK_H

#include <semaphore.h>

typedef struct Bank {
  unsigned int numberBranches;
  struct       Branch  *branches;
  struct       Report  *report;

  //reporting part
  int numWorkersHasToFinish; 
  sem_t for_check; 
  sem_t let_me_start_next_day;
  sem_t lock_for_report_transfer; 
} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);



#endif /* _BANK_H */
