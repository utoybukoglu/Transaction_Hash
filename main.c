#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // I added this library for use the tolower() function

struct Customer
{
    char name[81];
    int transactions;
    int items_purchased;
    float amount_paid;
};

typedef struct Customer customer;

void menu();
void printCustomers(customer *, int);
int countCustomers(FILE *);
void readTransactions(FILE *, customer *, int);
customer *createHashTable(); // creates and initializes the HashTable
customer *addCustomer(customer *, customer, int, int *, int);
customer *rehash(customer *, int *, int);
void printTable(customer *, int);
void searchTable(customer *, int, char[], int);
int checkPrime(int); // checks the given number is prime or not
int calculateKey(char *); // calculates the key (ASCII(lastletter - firstletter))
int hash(int, int); // hash function
int hash2(int); // hash2 function
void linearProbing(customer *, const int *, int *, char[]); // Linear Probing
void quadraticProbing(customer *, const int *, int *, char[]); // Quadratic Probing
void doubleHashing(customer *, const int *, int *, int, char[]); // Double Hashing
void displayCustomerInfo(customer);

int main()
{
    FILE *inFile;
    int no_of_customers = 0;
    customer *customers;
    customer *hashTable;

    inFile = fopen("transactions.txt", "r");
    if (inFile == NULL){
        printf("File could not be opened.\n");
        exit(1);
    }
    no_of_customers = countCustomers(inFile);
    printf("There are %d unique customers\n", no_of_customers);

    customers = (customer*) malloc(no_of_customers * sizeof (customer));
    fclose(inFile);
    inFile = fopen("transactions.txt", "r");
    if (inFile == NULL){
        printf("File could not be opened.\n");
        exit(1);
    }

    readTransactions(inFile, customers, no_of_customers);
    if (customers == NULL) {
        printf("\nCustomers have NOT been read successfully!\n");
        exit(1);
    }
    printCustomers(customers, no_of_customers);

    hashTable = createHashTable();
    int hashTableSize = 11;
    char criteria;

    printf("Enter your hashing criteria: ");
    fflush(stdin);
    scanf("%c", &criteria);

    while (criteria != '1' && criteria != '2' && criteria != '3'){
        printf("Error! Please enter a valid hashing criteria: ");
        fflush(stdin);
        scanf("%c", &criteria);
    }
    fflush(stdin);

    int i;
    for (i = 0; i < no_of_customers; i++){
        hashTable = addCustomer(hashTable, customers[i], i, &hashTableSize, criteria - '0');
        printf("%s has been added to the table, the hash table now can be seen below:", customers[i].name);
        printTable(hashTable, hashTableSize);
    }

    char command = 0;
    int exit = 0;
    char searchName[81];
    while (exit != 1) {
        menu();
        printf("\nCommand: ");
        fflush(stdin);
        scanf("%c", &command);

        if (command == '1') {
            printf("Enter the customer's name: ");
            fflush(stdin);
            scanf("%s", searchName);
            searchTable(hashTable, hashTableSize, searchName, criteria - '0');
        }
        else if (command == '2'){
            printTable(hashTable, hashTableSize);
        }
        else if (command == '3'){
            exit = 1;
            printf("Goodbye!\n");
        }
        else{
            printf("Please enter a valid command!\n");
        }
    }
    free(customers);
    free(hashTable);
    return 0;
}

void menu()
{
    printf("Please choose one of the following options:\n"
           "(1) Search a customer\n"
           "(2) Display hashtable\n"
           "(3) Exit\n");
}

void printCustomers(customer *customers, int no_of_customers)
{

    printf("List of customers:\n");
    int i;
    for (i = 0; i < no_of_customers; ++i)
    {
        printf("Name: %s, number of transactions = %d, number of items = %d, total amount paid = %.2f,"
               " average amount paid per transaction = %.2f\n",
               customers[i].name, customers[i].transactions, customers[i].items_purchased, customers[i].amount_paid,
               customers[i].amount_paid / customers[i].transactions);
    }
}

int countCustomers(FILE *inFile)
{
    int no_of_customers = 0;
    char **names;
    char **temp;
    char userName[100];
    int transactionId;
    char transactionTime[100];
    char itemDescription[100];
    int numberOfItemsPurchased;
    float costPerItem;
    char country[100];
    int exists = 0;
    char c;
    while ((c = fgetc(inFile)) != EOF)
    {
        if (c == '\n')
        {
            break;
        }
    }
    fscanf(inFile, "%[^;];%d;%[^;];%[^;];%d;%f;%[^\n]\n",
           userName, &transactionId, transactionTime, itemDescription,
           &numberOfItemsPurchased, &costPerItem, country);
    no_of_customers++;
    names = (char **)malloc(sizeof(char *) * (no_of_customers));
    names[no_of_customers - 1] = (char *)malloc(100 * sizeof(char));
    strcpy(names[no_of_customers - 1], userName);

    while (fscanf(inFile, "%[^;];%d;%[^;];%[^;];%d;%f;%[^\n]\n",
                  userName, &transactionId, transactionTime, itemDescription,
                  &numberOfItemsPurchased, &costPerItem, country) != EOF)
    {
        exists = 0;
        for (int i = 0; i < no_of_customers; ++i)
        {
            if (strncmp(names[i], userName, strlen(userName)) == 0)
            {
                exists = 1;
                break;
            }
        }
        if (exists)
            continue;
        temp = (char **)malloc(sizeof(char *) * (no_of_customers));
        for (int i = 0; i < no_of_customers; ++i)
        {
            temp[i] = (char *)malloc(100 * sizeof(char));
            strcpy(temp[i], names[i]);
        }
        free(names);

        names = (char **)malloc(sizeof(char *) * (no_of_customers + 1));
        for (int i = 0; i < no_of_customers; ++i)
        {
            names[i] = (char *)malloc(100 * sizeof(char));
            strcpy(names[i], temp[i]);
        }
        free(temp);
        names[no_of_customers] = (char *)malloc(100 * sizeof(char));
        strcpy(names[no_of_customers], userName);
        no_of_customers++;
    }
    return no_of_customers;
}

void readTransactions(FILE *inFile, customer *customers, int no_of_customers)
{
    char userName[100];
    int transactionId;
    char transactionTime[100];
    char itemDescription[100];
    int numberOfItemsPurchased;
    float costPerItem;
    char country[100];
    int exists = 0;
    int filled = 0;
    char c;

    for (int i = 0; i < no_of_customers; ++i)
    {
        strcpy(customers[i].name, "unassigned");
        customers[i].amount_paid = 0;
        customers[i].items_purchased = 0;
        customers[i].transactions = 0;
    }

    while ((c = fgetc(inFile)) != EOF)
    {
        if (c == '\n')
        {
            break;
        }
    }

    while (fscanf(inFile, "%[^;];%d;%[^;];%[^;];%d;%f;%[^\n]\n",
                  userName, &transactionId, transactionTime, itemDescription,
                  &numberOfItemsPurchased, &costPerItem, country) != EOF)
    {
        exists = 0; // n
        int i;      // n
        for (i = 0; i < no_of_customers; ++i)
        {
            if (strcmp(userName, customers[i].name) == 0)
            {
                exists = 1;
                break;
            }
        }
        if (exists)
        {
            customers[i].transactions++;
            customers[i].items_purchased += numberOfItemsPurchased;
            customers[i].amount_paid += (numberOfItemsPurchased * costPerItem);
        }
        else
        {
            strcpy(customers[filled].name, userName);
            customers[filled].transactions++;
            customers[filled].items_purchased += numberOfItemsPurchased;
            customers[filled].amount_paid += (numberOfItemsPurchased * costPerItem);
            filled++;
        }
    }
}

int checkPrime(int N) // https://www.geeksforgeeks.org/c-program-to-check-whether-a-number-is-prime-or-not/
{
    // initially, flag is set to true or 1
    int flag = 1;

    // loop to iterate through 2 to N/2
    for (int i = 2; i <= N / 2; i++)
    {

        // if N is perfectly divisible by i
        // flag is set to 0 i.e false
        if (N % i == 0)
        {
            flag = 0;
            break;
        }
    }

    if (flag)
        return 1;

    return 0;
}

int calculateKey(char *customerName)
{
    char tempName[50];
    strcpy(tempName, customerName); // I created tempName because I did not want to change the original name in tolower()

    for (int i = 0; tempName[i] != '\0'; i++)
        tempName[i] = tolower(tempName[i]);

    int size = strlen(tempName);
    int key = abs(tempName[size - 1] - tempName[0]);

    return key;
}

int hash(int key, int tableSize)
{
    return key % tableSize;
}

int hash2(int key)
{
    return 7 - (key % 7);
}

void linearProbing(customer *hashTable, const int *hashTableSize, int *index, char givenName[])
{
    if (strcasecmp(hashTable[*index].name, givenName) != 0) // collision (if name != unassigned)
    {
        int counter = 0;
        while (strcasecmp(hashTable[*index].name, givenName) != 0 && counter < *hashTableSize)
        {
            if (*index != *hashTableSize - 1)
                *index = *index + 1;
            else
                *index = 0;
            counter++;
        }
    }
}

void quadraticProbing(customer *hashTable, const int *hashTableSize, int *index, char givenName[])
{
    if (strcasecmp(hashTable[*index].name, givenName) != 0) // collision (if name != unassigned)
    {
        int i = 1;
        int temp = *index;
        while (strcasecmp(hashTable[temp].name, givenName) != 0 && i < *hashTableSize)
        {
            temp = hash(*index + (i * i), *hashTableSize);
            i++;
        }
        *index = temp;
    }
}

void doubleHashing(customer *hashTable, const int *hashTableSize, int *index, int key, char givenName[])
{
    if (strcasecmp(hashTable[*index].name, givenName) != 0) // collision (if name != unassigned)
    {
        int i = 1;
        int temp = *index;
        int v = hash2(key);
        while (strcasecmp(hashTable[temp].name, givenName) != 0 && i < *hashTableSize)
        {
            temp = hash(*index + (v * i), *hashTableSize);
            i++;
        }
        *index = temp;
    }
}

void displayCustomerInfo(customer c){
    printf("Information for customer %s:\n", c.name);
    printf("\tNumber of transactions = %d\n", c.transactions);
    printf("\tItems purchased = %d\n", c.items_purchased);
    printf("\tTotal Amount Paid = %.2f\n", c.amount_paid);
}

customer *createHashTable()
{
    customer *hashTable;
    hashTable = (customer *)malloc(11 * sizeof(customer));
    for (int i = 0; i < 11; i++)
    {
        strcpy(hashTable[i].name, "unassigned");
        hashTable[i].transactions = 0;
        hashTable[i].items_purchased = 0;
        hashTable[i].amount_paid = 0;
    }

    return hashTable;
}

customer *addCustomer(customer *hashTable, customer addedCustomer, int hashTableCapacity, int *hashTableSize, int criteria)
{
    float loadFactor;
    int key, index;
    char tempName[81];
    strcpy(tempName, "unassigned"); // I created this because I want to check is there someone at specific index

    key = calculateKey(addedCustomer.name);
    index = hash(key, *hashTableSize);

    if (criteria == 1)
        linearProbing(hashTable, hashTableSize, &index, tempName);

    else if (criteria == 2)
        quadraticProbing(hashTable, hashTableSize, &index, tempName);

    else
        doubleHashing(hashTable, hashTableSize, &index, key, tempName);

    hashTable[index] = addedCustomer; // add operation

    loadFactor = (float)(hashTableCapacity + 1) / *hashTableSize;

    if (loadFactor > 0.5)
        hashTable = rehash(hashTable, hashTableSize, criteria);

    return hashTable;
}

customer *rehash(customer *hashTable, int *hashTableSize, int criteria)
{
    char tempName[81];
    strcpy(tempName, "unassigned");
    int oldSize = *hashTableSize;
    *hashTableSize = 2 * (*hashTableSize);

    while (!checkPrime(*hashTableSize)) // Rounds the next prime number
        *hashTableSize = *hashTableSize + 1;

    int key, index;
    customer *newHashTable, *temp;
    temp = hashTable;
    newHashTable = (customer *)malloc(*hashTableSize * sizeof(customer));

    for (int i = 0; i < *hashTableSize; i++) // initialize the new table
    {
        strcpy(newHashTable[i].name, "unassigned");
        newHashTable[i].transactions = 0;
        newHashTable[i].items_purchased = 0;
        newHashTable[i].amount_paid = 0;
    }

    if (criteria == 1)
    {
        for (int i = 0; i < oldSize; i++) // rehashing with linearProbing
        {
            if (hashTable[i].transactions != 0)
            {
                key = calculateKey(hashTable[i].name);
                index = hash(key, *hashTableSize);
                linearProbing(newHashTable, hashTableSize, &index, tempName);
                newHashTable[index] = hashTable[i];
            }
        }
    }

    else if (criteria == 2) // rehashing with quadraticProbing
    {
        for (int i = 0; i < oldSize; i++)
        {
            if (hashTable[i].transactions != 0)
            {
                key = calculateKey(hashTable[i].name);
                index = hash(key, *hashTableSize);
                quadraticProbing(newHashTable, hashTableSize, &index, tempName);
                newHashTable[index] = hashTable[i];
            }
        }
    }

    else
    {
        for (int i = 0; i < oldSize; i++) // rehashing with doubleHashing
        {
            if (hashTable[i].transactions != 0)
            {
                key = calculateKey(hashTable[i].name);
                index = hash(key, *hashTableSize);
                doubleHashing(newHashTable, hashTableSize, &index, key, tempName);
                newHashTable[index] = hashTable[i];
            }
        }
    }

    hashTable = newHashTable;
    free(temp); // free old table

    return hashTable;
}

void printTable(customer *hashTable, int hashTableSize)
{
    printf("\n***************************************************************************\n");
    printf("Index         Name     Transactions      Items    Paid_Amount\n");

    for (int i = 0; i < hashTableSize; i++)
    {
        if(hashTable[i].transactions != 0)
            printf("%5d%13s%17d%11d%15.2f\n", i, hashTable[i].name, hashTable[i].transactions, hashTable[i].items_purchased, hashTable[i].amount_paid);
        else
            printf("%5d\n", i);
    }

    printf("***************************************************************************\n");
}

void searchTable(customer *hashTable, int hashTableSize, char searchName[], int criteria)
{
    int key, index, eqFlag;
    key = calculateKey(searchName);
    index = hash(key, hashTableSize);

    if (criteria == 1)
        linearProbing(hashTable, &hashTableSize, &index, searchName);

    else if (criteria == 2)
        quadraticProbing(hashTable, &hashTableSize, &index, searchName);

    else
        doubleHashing(hashTable, &hashTableSize, &index, key, searchName);

    eqFlag = strcasecmp(hashTable[index].name, searchName); // if found return 0

    if (!eqFlag)
        displayCustomerInfo(hashTable[index]);

    else
        printf("User does not exist in the table!\n");
}
