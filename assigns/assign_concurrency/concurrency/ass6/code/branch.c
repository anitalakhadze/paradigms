#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "error.h"
#include "debug.h"

// #include "account.h"

#include "branch.h"
#include <pthread.h>

pthread_mutex_t update_balance_m = PTHREAD_MUTEX_INITIALIZER;

/*
 * allocate and initialize each branch.
 */
int
Branch_Init(Bank *bank, int numBranches, int numAccounts,
            AccountAmount initialAmount)
{
  bank->numberBranches = numBranches;
  bank->branches = malloc(numBranches * sizeof(Branch));
  if (bank->branches == NULL) {
    return -1;
  }

  //initializing meta_lock
  pthread_mutex_init(&bank->meta_lock, NULL);

  //initializing branch balance mutex
  pthread_mutex_init(&bank->bank_balance_mutex, NULL);
  pthread_mutex_init(&bank->branch_update_mutex, NULL); 

  int accountsPerBranch = numAccounts /  numBranches;

  for (int i = 0; i < numBranches; i++) {
    Branch *branch = &bank->branches[i];

    //initializing separate lock for each branch
    pthread_mutex_init(&branch->branch_mutex, NULL);

    branch->branchID = i;
    branch->balance = 0;
    branch->numberAccounts = accountsPerBranch;
    branch->accounts = (Account *) malloc(accountsPerBranch * sizeof(Account));
    if (branch->accounts == NULL) {
      return -1;
    }

    for (int a = 0; a < accountsPerBranch; a++) {
      Account_Init(bank, &branch->accounts[a], a, i, initialAmount);
      branch->balance += branch->accounts[a].balance;
    }
  }

  return 0;
}

/*
 * update the balance of a branch.
 */
int
Branch_UpdateBalance(Bank *bank, BranchID branchID, AccountAmount change)
{
  pthread_mutex_lock(&bank->branch_update_mutex); 

  assert(bank->branches);  Y;
  if (branchID >= bank->numberBranches) {
    return -1;
  }
  
  AccountAmount oldBalance = bank->branches[branchID].balance; Y;
  bank->branches[branchID].balance = oldBalance + change; Y;
  
  pthread_mutex_unlock(&bank->branch_update_mutex); 

  return 0;
}

/*
 * get the balance of the branch
 */
int
Branch_Balance(Bank *bank, BranchID branchID, AccountAmount *balance)
{
  pthread_mutex_lock(&bank->bank_balance_mutex); 

  assert(bank->branches);

  if (branchID >= bank->numberBranches) {
    return -1;
  }
  
  *balance = bank->branches[branchID].balance;  Y;
  
  /* It should be the case that the balance of a branch matches the sum 
   * of all the accounts in the branch.  The following routine validates 
   * this assumption but is far too expense to run in normal operation. 
   */
  /* assert(Branch_Validate(bank, branchID) == 0);  */
  pthread_mutex_unlock(&bank->bank_balance_mutex);

  return 0;
}

/*
 * validate the branch by checking its branchID and making sure that
 * its balance equals the sum of balances of all accounts inside
 * the branch.
 */
int
Branch_Validate(Bank *bank, BranchID branchID)
{
  assert(bank->branches);

  if (branchID >= bank->numberBranches) {
    return -1;
  }

  Branch *branch = &bank->branches[branchID];
  AccountAmount total = 0;

  for (int a = 0; a < branch->numberAccounts; a++) {
    total += branch->accounts[a].balance;
  }

  if (total != branch->balance) {
    fprintf(stderr, "Branch balance mismatch. "
            "Computer value is %"PRId64", but stored value is %"PRId64"\n",
            total, branch->balance);
    return -1;
  }

  return 0;
}

/*
 * Compare all data inside two branches to see if they are exactly the same.
 */
int
Branch_Compare(Branch *branch1, Branch *branch2)
{
  int err = 0;

  BranchID branch1ID = branch1->branchID;
  BranchID branch2ID = branch2->branchID;

  if (branch1->numberAccounts !=  branch2->numberAccounts) {
    fprintf(stderr, "Branches %"PRIu64" and %"PRIu64" mismatch in numberAccounts "
            "(%d and %d, respectively).\n",
            branch1ID, branch2ID,
            branch1->numberAccounts,
            branch2->numberAccounts);
    err = -1;
  }

  if (branch1->balance != branch2->balance) {
    fprintf(stderr, "Branches %"PRIu64" and %"PRIu64" mismatch in balance "
            "(%"PRId64" and %"PRId64", respectively).\n",
            branch1ID, branch2ID,
            branch1->balance, branch2->balance);
    err = -1;
  }

  for (int i = 0; i < branch1->numberAccounts; i++) {

    assert(branch1->accounts[i].accountNumber ==
           branch2->accounts[i].accountNumber);

    if (branch1->accounts[i].balance != branch2->accounts[i].balance) {
      fprintf(stderr,
              "Branch %"PRIu64" and %"PRIu64" mismatch in account 0x%"PRIx64" balance "
              "(%"PRId64" and %"PRId64", respectively).\n",
              branch1ID, branch2ID,
              branch1->accounts[i].accountNumber,
              branch1->accounts[i].balance,
              branch2->accounts[i].balance);
      err = -1;
    }
  }
  return err;
}
