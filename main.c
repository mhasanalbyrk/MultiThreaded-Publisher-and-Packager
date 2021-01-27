#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int typesOfPublisherThreads ;
int publisherThreads ;
int numberOfPackagerThreads;
int numberOfBooksPerPublisher;
int packageSize ;
int bufferSize;

int numberOfBooksToCreated;


pthread_mutex_t packagerMutex;



int **bufferArray;


//#define Struct book Book typedef struct book **buffer;
struct publishers {
    int type;
    struct publisher *next;
    struct book *buffer;
    int bookCount;
    int size;
    pthread_mutex_t typeMutex;
};
typedef struct publishers Publishers;

struct book {
    int bookNo;
    struct book *next;
};
typedef struct book books;

struct publisherInput{
    int type;
    int publisherNo;
};

Publishers *pHead = NULL;

void addNode(Publishers **head, int type, books **buffHead) {
    Publishers *newNode = (Publishers *) malloc(sizeof(Publishers));
    newNode->type = type;
    newNode->next = NULL;
    newNode->buffer = *buffHead;
    newNode->size = bufferSize;
   // pthread_mutex_init(&packagerMutex, NULL);
    pthread_mutex_init(&(newNode->typeMutex), NULL);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    Publishers *current = *head;
    while (current->next != NULL)
        current = (Publishers *) current->next;
    current->next = (struct publisher *) newNode;

}


void createNode(books **head, int num) {
    books *current;
    current = *head;
    if ((*head) == NULL) {
        books *newNode = (books *) malloc(sizeof(books));
        newNode->bookNo = -1;
        newNode->next = NULL;
        (*head) = newNode;
        return;
    }
    while (current->next != NULL)
        current = (books *) current->next;

    for (int i = 0; i < num; i++) {
        books *newNode = (books *) malloc(sizeof(books));
        newNode->bookNo = -1;
        newNode->next = NULL;
        current->next = newNode;
        current = current->next;
    }
}

void *publisher(void *input1) {
    struct publisherInput * input = (struct publisherInput*) input1;
    Publishers *current = &(*pHead);
    while (current->type != input->type)
        current = (Publishers *) current->next;

    for (int i = 0; i < numberOfBooksPerPublisher; i++) {
        //pthread_mutex_lock(&packagerMutex);
        pthread_mutex_lock(&(current->typeMutex));

        books *bcurrent = current->buffer;
        while (bcurrent->bookNo != -1 && bcurrent->next != NULL) {
            bcurrent = bcurrent->next;
        }

        if (bcurrent->next == NULL && bcurrent->bookNo != -1) {
            createNode(&(current->buffer), current->size);
            printf("Publisher %d of  type %d\t\t", input->publisherNo, input->type + 1);
            printf("Buffer is full size is => %d. Resizing the buffer\n", current->size);
            current->size *= 2;
            bcurrent->next->bookNo = current->bookCount + 1;
        } else {
            bcurrent->bookNo = current->bookCount + 1;
        }



        printf("Publisher %d of  type %d\t\t", input->publisherNo, input->type + 1);
        printf("Book%d_%d is published and put into the buffer.\n", input->type + 1, bcurrent->bookNo);
        current->bookCount++;
        numberOfBooksToCreated--;

       // pthread_mutex_unlock(&packagerMutex);
        pthread_mutex_unlock(&(current->typeMutex));

    }

    printf("Publisher %d of  type %d\t\t", input->publisherNo, input->type + 1);
    printf("***************************** Finished publishing %d books. Exiting the system *****************************\n", numberOfBooksPerPublisher);
}

void *packager( int pNo) {
    char packagedBooks[packageSize][50];
    int index = 0;

//    int a = rand() % typesOfPublisherThreads;
//    Publishers *current = &(*pHead);
//    while (current->type != a)
//        current = (Publishers *) current->next;

    while (1) {
       // pthread_mutex_lock(&packagerMutex);

        int a = rand() % typesOfPublisherThreads;
        Publishers *current = &(*pHead);
        while (current->type != a)
            current = (Publishers *) current->next;

        pthread_mutex_lock(&(current->typeMutex));

        books *bcurrent = current->buffer;
        while (bcurrent->bookNo == -1 && bcurrent->next != NULL) {
            bcurrent = bcurrent->next;
        }
        if (bcurrent->bookNo != -1) {
            char buf[50];
            printf("Packager %d\t\t", pNo);
            printf("Put Book%d_%d into the package.\n", a + 1, bcurrent->bookNo);
            snprintf(buf, 50, "Book%d_%d", a + 1, bcurrent->bookNo);
            strcpy(packagedBooks[index], buf);
            index++;
            bcurrent->bookNo = -1;
        }
        if (index == packageSize ) {
            printf("Packager %d\t\t", pNo);
            printf("Finished preparing one package. ");
            printf("The package contains ");
            for (int i = 0; i < index; i++) {
                printf("%s, ", packagedBooks[i]);
            }
            index = 0;
            pthread_mutex_unlock(&(current->typeMutex));


        }
        else if (numberOfBooksToCreated == 0) {
            printf("Packager %d\t\t", pNo);
            printf("There are no publishers left in the system.");
            printf("Only %d of %d books could be packaged.", index + 1, packageSize);

            printf(" \n***************************** Exiting the system. *****************************\n");
            pthread_mutex_unlock(&(current->typeMutex));

            break;
        }
        else {
            pthread_mutex_unlock(&(current->typeMutex));
        }
    }


//    pthread_mutex_unlock(&packagerMutex);


}


int main(int argc, char *argv[]) {

    char *a = argv[2];
    typesOfPublisherThreads = atoi(a);
    a = argv[3];
    publisherThreads = atoi(a);
    a = argv[4];
    numberOfPackagerThreads = atoi(a);
    a = argv[6];
    numberOfBooksPerPublisher = atoi(a);
    a = argv[8];
    packageSize = atoi(a);
    a = argv[9];
    bufferSize = atoi(a);


//    printf("%d \n", typesOfPublisherThreads);
//    printf("%d \n", publisherThreads);
//    printf("%d \n", numberOfPackagerThreads);
//    printf("%d \n", numberOfBooksPerPublisher);
//    printf("%d \n", packageSize);
//    printf("%d \n", bufferSize);
//    printf("%d\n", argc);
    books *bookArray[typesOfPublisherThreads];
    for (int i = 0; i < typesOfPublisherThreads; i++) {//loop for publisher types
        bookArray[i] = NULL;
        createNode(&(bookArray[i]), 1);
        createNode(&(bookArray[i]), bufferSize - 1);
        addNode(&pHead, i, &(bookArray[i]));
    }


    //pthread_mutex_init(&publisherMutex, NULL);
    pthread_mutex_init(&packagerMutex, NULL);


    pthread_t th_packager[numberOfPackagerThreads];
    pthread_t th_publisher[typesOfPublisherThreads][publisherThreads];

    numberOfBooksToCreated = typesOfPublisherThreads * publisherThreads * numberOfBooksPerPublisher;


    for (int i = 0; i < typesOfPublisherThreads; i++) {//loop for publisher types
        for (int j = 0; j < publisherThreads; j++) {//loop for publishers
            // TODO Create publisher
            struct publisherInput *input = malloc(sizeof (struct publisherInput));
            input->publisherNo = j + 1;
            input->type = i;
            pthread_create(th_publisher[i] + j, NULL,  &publisher, (void *) input); //last arg is args
        }
    }

    for (int i = 0; i < numberOfPackagerThreads; i++) {
        // TODO Create packagers
        pthread_create(th_packager + i, NULL, &packager, i + 1); //last arg is args
    }


    for (int i = 0; i < typesOfPublisherThreads; i++) {
        for (int j = 0; j < publisherThreads; j++) {
            // TODO Join
            if (pthread_join(th_publisher[i][j], NULL) != 0) {
                perror("Publisher thread join failed! \n");
            }
        }
    }

    for (int i = 0; i < numberOfPackagerThreads; i++) {
        // TODO Join
        if (pthread_join(th_packager[i], NULL) != 0) {
            perror("Packager thread join failed!\n");
        }
    }

}