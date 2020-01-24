<h1 class="contract">add</h1>

Adding a list of accounts for RAM recovery.

### Intent
INTENT. This is the only way to add accounts to this contract. This is done by contract operator(s).

### Term
TERM. This Contract expires at the conclusion of code execution.


<h1 class="contract">remove</h1>

Removes a list of accounts, either from the main account list, or from the skipped account list.

### Intent
INTENT. This is one way to remove accounts from this contract. This is done by contract operator(s).

### Term
TERM. This Contract expires at the conclusion of code execution.


<h1 class="contract">sellram</h1>

Selling RAM from n accounts.

### Intent
INTENT. Anyone who can issue transactions, can participate to the recovery process.

### Term
TERM. This Contract expires at the conclusion of code execution.


<h1 class="contract">retry</h1>

Places n accounts from the skipped list to the main account list for retrying.
The main account list must be empty.

### Intent
INTENT. Anyone who can issue transactions, can participate to the recovery process.

### Term
TERM. This Contract expires at the conclusion of code execution.


<h1 class="contract">setconfig</h1>

Sets the following contract configuration parameters:
    - What is the amount of RAM to leave to each account (bytes)
    - Should tokens be transferred to the recovery contract

### Intent
INTENT. This is the only way to change the contract configuration. This is done by contract operator(s).

### Term
TERM. This Contract expires at the conclusion of code execution.

