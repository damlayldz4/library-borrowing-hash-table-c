#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//DAMLA YILDIZ
//2640746
typedef struct BorrowRecord {
    char borrowID[20];
    char bookTitle[200];
    char author[300];
    char borrowDate[20];
    struct BorrowRecord* next;
} BorrowRecord;

typedef struct {
    int studentID;
    char studentName[50];
    BorrowRecord* borrowList;
    int borrowCount;
} Student;

typedef struct {
    int isOccupied;
    Student student;
} HashEntry;

//prototypes
HashEntry* createHashTable(int size);
HashEntry* readBorrowings(FILE* inFile, HashEntry* table, int* sizePtr, int method);
HashEntry* addStudent(HashEntry* table, Student addedStudent, int* sizePtr, int method);
HashEntry* rehash(HashEntry* table, int* sizePtr, int method);
void printTable(HashEntry* table, int size);
void searchTable(HashEntry* table, int size, int studentID, int method);
void returnBook(HashEntry* table, int size, int studentID, char borrowID[], int method);

// Helper function prototypes
int calculateKey(int studentID);
int isPrime(int n);
int nextPrime(int n);
int hash1(int key, int tableSize);
int hash2(int key);

int main() {
    FILE* inFile = fopen("borrowed_books.txt", "r");
    if (!inFile) {
        printf("Error!! We can't access to borrowed_books file\n");
        return 1;
    }
    int technique = 0;
    int tableSize = 11;
    printf("Select collision resolution technique:\n");
    printf("1. Linear Probing\n");
    printf("2. Quadratic Probing\n");
    printf("3. Double Hashing\n");
    printf("Enter your choice: ");
    scanf("%d", &technique);
    while (technique < 1 || technique > 3) {
        printf("Invalid choice. Enter 1, 2, or 3: ");
        scanf("%d", &technique);
    }
    HashEntry* table = createHashTable(tableSize);
    table = readBorrowings(inFile, table, &tableSize, technique);
    fclose(inFile);
    int choice = -1;
    while (choice != 0) {
        printf("\n------ MENU ------\n");
        printf("1. Print Hash Table\n");
        printf("2. Search Student\n");
        printf("3. Return Book\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        if (choice == 1) {
            printTable(table, tableSize);
        } else if (choice == 2) {
            int sid;
            printf("Enter Student ID to search: ");
            scanf("%d", &sid);
            searchTable(table, tableSize, sid, technique);
        } else if (choice == 3) {
            int sid;
            char borrowID[20];
            printf("Enter Student ID: ");
            scanf("%d", &sid);
            printf("Enter Borrow ID to return: ");
            scanf("%19s", borrowID);
            returnBook(table, tableSize, sid, borrowID, technique);
        } else if (choice == 0) {
            printf("Exiting program...\n");
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }
    free(table);
    return 0;
}

// Helper function to calculate key from student ID
int calculateKey(int studentID) {
    int key = 1;
    int temp = studentID;

    while (temp > 0) {
        int digit = temp % 10;
        if (digit == 0) {
            digit = 1;
        }
        key *= digit;
        temp /= 10;
    }
    return key;
}

// Helper function to check if a number is prime
int isPrime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;

    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

// Helper function to find next prime number
int nextPrime(int n) {
    if (n <= 1) return 2;

    int prime = n;
    int found = 0;

    while (!found) {
        prime++;
        if (isPrime(prime)) {
            found = 1;
        }
    }
    return prime;
}

// First hash function
int hash1(int key, int tableSize) {
    return key % tableSize;
}

// Second hash function for double hashing
int hash2(int key) {
    return 7 - (key % 7);
}

HashEntry* createHashTable(int size) {
    // I created memory for the hash table
    HashEntry* table = (HashEntry*)malloc(size * sizeof(HashEntry));
    if (table == NULL) {
        printf("Memory allocation failed!!\n");
        exit(1);
    }

    //We initialize every position as empty for every part in that index
    for (int i = 0; i < size; i++) {
        table[i].isOccupied = 0;
        table[i].student.studentID = -1;
        strcpy(table[i].student.studentName, "unassigned");
        table[i].student.borrowList = NULL;
        table[i].student.borrowCount = 0;
    }

    return table;
}

HashEntry* readBorrowings(FILE* inFile, HashEntry* table, int* sizePtr, int method) {
    //This function reads each line from the file chosen
    //After every parsing operation, it adds the student and it's borrowing details
    char line[1000];

    // Skip header line because it contains only the column names
    fgets(line, sizeof(line), inFile);

    while (fgets(line, sizeof(line), inFile)) {
        // Delete newline character at the end (if exists)
        line[strcspn(line, "\n")] = '\0';

        char* token;
        Student tempS;

        // create a new borrow record node
        BorrowRecord* newRecord = (BorrowRecord*)malloc(sizeof(BorrowRecord));

        // Parse the line and read ID,name,data
        token = strtok(line, ";");
        tempS.studentID = atoi(token);

        token = strtok(NULL, ";");
        strcpy(tempS.studentName, token);
        token = strtok(NULL, ";");
        strcpy(newRecord->borrowID, token);
        token = strtok(NULL, ";");
        strcpy(newRecord->bookTitle, token);
        token = strtok(NULL, ";");
        strcpy(newRecord->author, token);
        token = strtok(NULL, ";");
        strcpy(newRecord->borrowDate, token);

        newRecord->next = NULL;

        // Try to find if the student already exists
        int found = 0;
        int key = calculateKey(tempS.studentID);
        int index = hash1(key, *sizePtr);
        int i = 0;
        int stop = 0;

        while (!stop && i < *sizePtr) {
            int currentIndex;

            if (method == 1) { // Linear probing
                currentIndex = (index + i) % (*sizePtr);
            } else if (method == 2) { // Quadratic probing
                currentIndex = (index + i * i) % (*sizePtr);
            } else { // Double hashing
                currentIndex = (index + i * hash2(key)) % (*sizePtr);
            }

            if (table[currentIndex].isOccupied && table[currentIndex].student.studentID == tempS.studentID) {
                // Student found, add borrow record
                newRecord->next = table[currentIndex].student.borrowList;
                table[currentIndex].student.borrowList = newRecord;
                table[currentIndex].student.borrowCount++;
                found = 1;
                stop = 1;
            } else if (!table[currentIndex].isOccupied) {
                // Empty slot: student is not in the table
                stop = 1;
            } else {
                // adding 1 to continue probing
                i++;
            }
        }

        if (!found) {
            // Student not found, add as a new student
            tempS.borrowList = newRecord;
            tempS.borrowCount = 1;
            table = addStudent(table, tempS, sizePtr, method);
        }
    }

    return table;
}

HashEntry* addStudent(HashEntry* table, Student addedStudent, int* sizePtr, int method) {
    // first we need to control the load factor
    int occupiedcount = 0;
    for (int j = 0; j < *sizePtr; j++) {
        if (table[j].isOccupied) {
            occupiedcount++;
        }
    }

    double loadFactor = (double)(occupiedcount + 1) / (*sizePtr); // +1 because we will add a student
    if (loadFactor > 0.5) {
        printf("Load factor exceeded 0.5 rehashing table from size %d to ", *sizePtr);
        table = rehash(table, sizePtr, method);
        printf("%d.\n\n", *sizePtr);
    }

    // now, we need to add the student
    int key = calculateKey(addedStudent.studentID);
    int index = hash1(key, *sizePtr);
    int i = 0;

    // We need to find an empty slot
    while (table[index].isOccupied) {
        if (method == 1) { // Linear probing
            index = (index + 1) % (*sizePtr);
        } else if (method == 2) { // Quadratic probing
            i++;
            index = (hash1(key, *sizePtr) + i * i) % (*sizePtr);
        } else { // Double hashing
            i++;
            index = (hash1(key, *sizePtr) + i * hash2(key)) % (*sizePtr);
        }
    }

    // adding the student to the table
    table[index].isOccupied = 1;
    table[index].student.studentID = addedStudent.studentID;
    strcpy(table[index].student.studentName, addedStudent.studentName);
    table[index].student.borrowList = addedStudent.borrowList;
    table[index].student.borrowCount = addedStudent.borrowCount;

    printf("%d (%s) has been added to the table, the hash table now can be seen below:\n",
           addedStudent.studentID, addedStudent.studentName);
    printTable(table, *sizePtr);

    return table;
}

HashEntry* rehash(HashEntry* table, int* sizePtr, int method) {
    // we need to double the old size and choose the most near prime number as a new size
    int old_size = *sizePtr;
    int new_size = nextPrime(old_size * 2);
    HashEntry* newTable = createHashTable(new_size);

    *sizePtr = new_size;

    // Rehash all students from old table to new table
    for (int i = 0; i < old_size; i++) {
        if (table[i].isOccupied) {
            Student student = table[i].student;

            int key = calculateKey(student.studentID);
            int index = hash1(key, new_size);
            int j = 0;

            // Find empty slot in new table
            while (newTable[index].isOccupied) {
                if (method == 1) { // Linear probing
                    index = (index + 1) % new_size;
                } else if (method == 2) { // Quadratic probing
                    j++;
                    index = (hash1(key, new_size) + j * j) % new_size;
                } else { // Double hashing
                    j++;
                    index = (hash1(key, new_size) + j * hash2(key)) % new_size;
                }
            }

            // Add student to new table
            newTable[index].isOccupied = 1;
            newTable[index].student = student;
        }
    }

    free(table);
    return newTable;
}

void printTable(HashEntry* table, int size) {
    printf("Index         StudentID       Name                     BorrowCount\n");
    for (int i = 0; i < size; i++) {
        if (table[i].isOccupied) {
            printf("%-14d %-15d %-24s %d\n",
                   i,
                   table[i].student.studentID,
                   table[i].student.studentName,
                   table[i].student.borrowCount);
        } else {
            printf("%-14d \n", i);
        }
    }
    printf("\n");
}

void searchTable(HashEntry* table, int size, int studentID, int method) {
    int key = calculateKey(studentID);
    int index = hash1(key, size);
    int i = 0;
    int found = 0;
    int stop = 0;

    while (!found && !stop && i < size) {
        int currentIndex;
        if (method == 1) { // Linear probing
            currentIndex = (index + i) % size;
        } else if (method == 2) { // Quadratic probing
            currentIndex = (index + i * i) % size;
        } else { // Double hashing
            currentIndex = (index + i * hash2(key)) % size;
        }

        if (table[currentIndex].isOccupied &&
            table[currentIndex].student.studentID == studentID) {

            // Student found
            printf("\nStudent ID: %d\n", studentID);
            printf("Student Name: %s\n", table[currentIndex].student.studentName);
            printf("Number of Borrowed Books: %d\n", table[currentIndex].student.borrowCount);

            BorrowRecord* current = table[currentIndex].student.borrowList;
            while (current != NULL) {
                printf("\nBorrow ID: %s\n", current->borrowID);
                printf("Title: %s\n", current->bookTitle);
                printf("Author(s): %s\n", current->author);
                printf("Borrow Date: %s\n", current->borrowDate);
                current = current->next;
            }
            printf("\n");
            found = 1;
        } else if (!table[currentIndex].isOccupied) {
            // Reached an empty slot: student not in table
            stop = 1;
        } else {
            // Continue probing
            i++;
        }
    }

    if (!found) {
        printf("Student %d not found.\n\n", studentID);
    }
}

void returnBook(HashEntry* table, int size, int studentID, char borrowID[], int method) {
    int key = calculateKey(studentID);
    int index = hash1(key, size);
    int i = 0;
    int found_borrow = 0;
    int stop = 0;
    int notfound = 0;

    while (!stop && !found_borrow && i < size) {
        int currentIndex;
        if (method == 1) { // Linear probing
            currentIndex = (index + i) % size;
        } else if (method == 2) { // Quadratic probing
            currentIndex = (index + i * i) % size;
        } else { // Double hashing
            currentIndex = (index + i * hash2(key)) % size;
        }

        if (table[currentIndex].isOccupied &&
            table[currentIndex].student.studentID == studentID) {

            // Student found, search for borrow record
            notfound = 1;
            BorrowRecord* current = table[currentIndex].student.borrowList;
            BorrowRecord* prev = NULL;

            while (current != NULL && !found_borrow) {
                if (strcmp(current->borrowID, borrowID) == 0) {
                    // Borrow record found, remove it
                    if (prev == NULL) {
                        // First node in the list
                        table[currentIndex].student.borrowList = current->next;
                    } else {
                        prev->next = current->next;
                    }

                    free(current);
                    table[currentIndex].student.borrowCount--;
                    printf("Borrow ID %s removed for Student %d.\n\n", borrowID, studentID);
                    found_borrow = 1;
                } else {
                    prev = current;
                    current = current->next;
                }
            }

            if (!found_borrow) {
                printf("Borrow ID %s not found for Student %d.\n\n", borrowID, studentID);
            }

            // We already checked this student list. So, no need to probe further
            stop = 1;
        } else if (!table[currentIndex].isOccupied) {
            // Empty slot: student not in this probe chain
            stop = 1;
        } else {
            // Continue probing
            i++;
        }
    }

    if (!notfound) {
        printf("Student %d not found.\n\n", studentID);
    }
}
//
// Created by BiLKANCOMPUTERS on 30.11.2025.
//
