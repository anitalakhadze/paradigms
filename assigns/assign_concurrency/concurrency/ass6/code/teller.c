#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include <pthread.h>

#include <stdint.h>

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
  uint64_t branchID =  AccountNum_GetBranchID(accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  pthread_mutex_lock(&bank->branches[branchID].branch_mutex); 
  Account_Adjust(bank,account, amount, 1);
  pthread_mutex_unlock(&bank->branches[branchID].branch_mutex);

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

  if (amount > Account_Balance(account)) {
    return ERROR_INSUFFICIENT_FUNDS;
  }

  pthread_mutex_lock(&bank->branches[branchID].branch_mutex);
  Account_Adjust(bank,account, -amount, 1);
  pthread_mutex_unlock(&bank->branches[branchID].branch_mutex);

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

  if (amount > Account_Balance(srcAccount)) {
    return ERROR_INSUFFICIENT_FUNDS;
  }

  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);

  uint64_t branchIDsrc =  AccountNum_GetBranchID(srcAccountNum);
  uint64_t branchIDdst =  AccountNum_GetBranchID(dstAccountNum);

  //printf("#%ld thread: update branch is %i. START! \n", pthread_self(), updateBranch);

  if (updateBranch){
    //printf("#%ld thread: is DOING TRANSFER. Branch is not same.\n", pthread_self());
    pthread_mutex_lock(&bank->meta_lock);
      //printf("#%ld thread: ATTEMPTING to take lock SRC, %lu.\n", pthread_self(), branchIDsrc);
      pthread_mutex_lock(&bank->branches[branchIDsrc].branch_mutex);
      //printf("#%ld thread: ATTEMPTING to take lock DST, %lu.\n", pthread_self(), branchIDdst);
      pthread_mutex_lock(&bank->branches[branchIDdst].branch_mutex);
    pthread_mutex_unlock(&bank->meta_lock);

    //printf("#%ld thread: adjusting amounts.\n", pthread_self());
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
   // printf("#%ld thread: RELEASING lock DST, %lu.\n", pthread_self(), branchIDdst);
    pthread_mutex_unlock(&bank->branches[branchIDdst].branch_mutex);
    //printf("#%ld thread: RELEASING lock SRC, %lu.\n", pthread_self(), branchIDdst);
    pthread_mutex_unlock(&bank->branches[branchIDsrc].branch_mutex);
    //printf("#%ld thread: RELEASED ALL LOCKS. Different branches.\n", pthread_self());
  } else {
    //printf("#%ld thread: is DOING TRANSFER. Same branch.\n", pthread_self());
    if (srcAccount == dstAccount){
      //printf("#%ld thread: accounts are same!\n", pthread_self());
    }
   // printf("#%ld thread: ATTEMPTING to take lock SRC, %lu.\n", pthread_self(), branchIDsrc);
    pthread_mutex_lock(&bank->branches[branchIDsrc].branch_mutex);
    //printf("#%ld thread: ADJUSTING amounts.\n", pthread_self());
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    pthread_mutex_unlock(&bank->branches[branchIDsrc].branch_mutex);
    //printf("#%ld thread: RELEASED locks. Same branches.\n", pthread_self());
  }

  //printf("#%ld thread: update branch is %i. END \n", pthread_self(), updateBranch);

  return ERROR_SUCCESS;
}
