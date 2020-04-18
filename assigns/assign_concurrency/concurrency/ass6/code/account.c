
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "teller.h"
#include "error.h"
#include "debug.h"
#include "account.h"

// #include "branch.h"

#include "report.h"
#include <pthread.h>

pthread_mutex_t account_adjust_m = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t account_balance_m = PTHREAD_MUTEX_INITIALIZER; 

/*
 * initialize the account based on the passed-in information.
 */
void
Account_Init(Bank *bank, Account *account, int id, int branch,
             AccountAmount initialAmount)
{
  extern int testfailurecode;

  account->accountNumber = Account_MakeAccountNum(branch, id);
  account->balance = initialAmount;
  if (testfailurecode) {
    // To test failures, we initialize every 4th account with a negative value
    if ((id & 0x3) == 0) {
      account->balance = -1;
    }
  }

}

/*
 * get the ID of the branch which the account is in.
 */
BranchID
AccountNum_GetBranchID(AccountNumber accountNum)
{
  Y;
  return (BranchID) (accountNum >> 32);
}

/*
 * get the branch-wide subaccount number of the account.
 */
int
AcountNum_Subaccount(AccountNumber accountNum)
{
  Y;
  return  (accountNum & 0x7ffffff);
}

/*
 * find the account address based on the accountNum.
 */
Account *
Account_LookupByNumber(Bank *bank, AccountNumber accountNum)
{
  BranchID branchID =  AccountNum_GetBranchID(accountNum);
  int branchIndex = AcountNum_Subaccount(accountNum);
  return &(bank->branches[branchID].accounts[branchIndex]);
}

/*
 * adjust the balance of the account.
 */

// Mistake may be made here. There should not be more than one operation here. 
//These mutexes are not necessary here as we control this from teller 

void
Account_Adjust(Bank *bank, Account *account, AccountAmount amount,
               int updateBranch)
{
  //pthread_mutex_lock(&account_adjust_m); 
  account->balance = Account_Balance(account) + amount;
  if (updateBranch) {
    Branch_UpdateBalance(bank, AccountNum_GetBranchID(account->accountNumber),
                         amount);
  }
  //pthread_mutex_unlock(&account_adjust_m); 
  Y;
}

/*
 * return the balance of the account.
 */


//Account amount not to be returned by mistake??? Not exactly right I think but the idea is 
//that I somehow want to return the correct value of the account reflected by the 
//amount that was deposited or withdrawn 
AccountAmount
Account_Balance(Account *account)
{
  pthread_mutex_lock(&account_balance_m); 
  AccountAmount balance = account->balance; Y;
  pthread_mutex_unlock(&account_balance_m);
  return balance;
}

/*
 * make the account number based on the branch number and
 * the branch-wise subaccount number.
 */
AccountNumber
Account_MakeAccountNum(int branch, int subaccount)
{
  AccountNumber num;

  num = subaccount;
  num |= ((uint64_t) branch) << 32;  Y;
  return num;
}


/*
 * Test to see if two accounts are in the same branch.
 */

int
Account_IsSameBranch(AccountNumber accountNum1, AccountNumber accountNum2)
{
  return (AccountNum_GetBranchID(accountNum1) ==
          AccountNum_GetBranchID(accountNum2));
}
