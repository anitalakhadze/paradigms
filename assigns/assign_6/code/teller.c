#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"

#include <stdint.h>
#include "branch.h"
/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  sem_wait(&(account->acc_lock));
  Account_Adjust(bank,account, amount, 1);
  sem_post(&(account->acc_lock)); 
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);
  uint64_t branchID =  AccountNum_GetBranchID(accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  sem_wait(&(account->acc_lock));
  sem_wait(&(bank->branches[branchID].branch_lock)); 
  if (amount > Account_Balance(account)) {
    sem_post(&(account->acc_lock));
    sem_post(&(bank->branches[branchID].branch_lock)); 
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank,account, -amount, 1);
  sem_post(&(account->acc_lock));
  sem_post(&(bank->branches[branchID].branch_lock));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

   /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);

  uint64_t branchIDsrc =  AccountNum_GetBranchID(srcAccountNum);
  uint64_t branchIDdst =  AccountNum_GetBranchID(dstAccountNum);

  if(!updateBranch){
    if(srcAccount->accountNumber < dstAccount->accountNumber){
      sem_wait(&(srcAccount->acc_lock));
      sem_wait(&(dstAccount->acc_lock)); 
    } else {
      sem_wait(&(dstAccount->acc_lock)); 
      sem_wait(&(srcAccount->acc_lock));
    }
    if (amount > Account_Balance(srcAccount)) {
      sem_post(&(srcAccount->acc_lock)); 
      sem_post(&(dstAccount->acc_lock)); 
      return ERROR_INSUFFICIENT_FUNDS;
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    sem_post(&(srcAccount->acc_lock)); 
    sem_post(&(dstAccount->acc_lock)); 
    return ERROR_SUCCESS;
  } else {
    if(branchIDsrc < branchIDdst){
      sem_wait(&(srcAccount->acc_lock));
      sem_wait(&(dstAccount->acc_lock));
      sem_wait(&(bank->branches[branchIDsrc].branch_lock));
      sem_wait(&(bank->branches[branchIDdst].branch_lock));
    } else {
      sem_wait(&(dstAccount->acc_lock));
      sem_wait(&(srcAccount->acc_lock));
      sem_wait(&(bank->branches[branchIDdst].branch_lock));
      sem_wait(&(bank->branches[branchIDsrc].branch_lock));
    }
    if (amount > Account_Balance(srcAccount)) {
      sem_post(&(srcAccount->acc_lock)); 
      sem_post(&(dstAccount->acc_lock)); 
      sem_post(&(bank->branches[branchIDdst].branch_lock));
      sem_post(&(bank->branches[branchIDsrc].branch_lock));
      return ERROR_INSUFFICIENT_FUNDS;
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    sem_post(&(srcAccount->acc_lock)); 
    sem_post(&(dstAccount->acc_lock)); 
    sem_post(&(bank->branches[branchIDsrc].branch_lock));
    sem_post(&(bank->branches[branchIDdst].branch_lock));
    return ERROR_SUCCESS;
  }
}
