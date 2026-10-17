
#include <stdio.h>

#define P 5
#define R 3

int alloc[P][R], max[P][R], need[P][R];
int available[R], work[R], finish[P], seq[P];
int safeCount = 0;

int order[P], used[P];
int safeOrders = 0, unsafeOrders = 0;
int deadEndCount = 0;

/* Check whether process can finish */
int canRun(int p, int w[])
{
    for (int j = 0; j < R; j++)
        if (need[p][j] > w[j])
            return 0;
    return 1;
}

/* Print a sequence */
void printSequence(int s[], int len)
{
    for (int i = 0; i < len; i++)
        printf("P%d%s", s[i], i == len - 1 ? "" : " -> ");
}

/* Find all safe sequences */
void safeSequences(int depth)
{
    if (depth == P) {
        printf("Safe sequence %d: ", ++safeCount);
        printSequence(seq, P);
        printf("\n");
        return;
    }

    for (int i = 0; i < P; i++) {
        if (!finish[i] && canRun(i, work)) {
            for (int j = 0; j < R; j++)
                work[j] += alloc[i][j];

            finish[i] = 1;
            seq[depth] = i;

            safeSequences(depth + 1);

            finish[i] = 0;
            for (int j = 0; j < R; j++)
                work[j] -= alloc[i][j];
        }
    }
}

/* Enumerate every complete process order */
void allOrders(int depth)
{
    if (depth == P) {
        int temp[R];
        int safe = 1;

        for (int j = 0; j < R; j++)
            temp[j] = available[j];

        for (int k = 0; k < P; k++) {
            int p = order[k];

            if (!canRun(p, temp)) {
                safe = 0;
                break;
            }

            for (int j = 0; j < R; j++)
                temp[j] += alloc[p][j];
        }

        if (safe)
            safeOrders++;
        else
            unsafeOrders++;

        printf("%s order: ", safe ? "Safe" : "Unsafe");
        printSequence(order, P);
        printf("\n");
        return;
    }

    for (int i = 0; i < P; i++) {
        if (!used[i]) {
            used[i] = 1;
            order[depth] = i;
            allOrders(depth + 1);
            used[i] = 0;
        }
    }
}

/* Show executable partial sequences that get stuck */
void deadEnds(int depth)
{
    int moved = 0;

    for (int i = 0; i < P; i++) {
        if (!finish[i] && canRun(i, work)) {
            moved = 1;

            finish[i] = 1;
            seq[depth] = i;

            for (int j = 0; j < R; j++)
                work[j] += alloc[i][j];

            deadEnds(depth + 1);

            for (int j = 0; j < R; j++)
                work[j] -= alloc[i][j];

            finish[i] = 0;
        }
    }

    if (!moved && depth < P) {
        deadEndCount++;

        printf("Blocked after: ");
        if (depth == 0)
            printf("no process completed");
        else
            printSequence(seq, depth);

        printf("\nProcesses unable to finish: ");
        for (int i = 0; i < P; i++)
            if (!finish[i])
                printf("P%d ", i);

        printf("\nWork: ");
        for (int j = 0; j < R; j++)
            printf("%d ", work[j]);

        printf("\n\n");
    }
}

int main(void)
{
    printf("Enter Allocation matrix (5 x 3):\n");
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            if (scanf("%d", &alloc[i][j]) != 1 ||
                alloc[i][j] < 0)
                return 1;

    printf("Enter Maximum Claim matrix (5 x 3):\n");
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++) {
            if (scanf("%d", &max[i][j]) != 1 ||
                max[i][j] < alloc[i][j]) {
                printf("Invalid maximum claim.\n");
                return 1;
            }
            need[i][j] = max[i][j] - alloc[i][j];
        }

    printf("Enter Available resources (3 values):\n");
    for (int j = 0; j < R; j++)
        if (scanf("%d", &available[j]) != 1 ||
            available[j] < 0)
            return 1;

    printf("\nNeed Matrix:\n");
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }

    for (int j = 0; j < R; j++)
        work[j] = available[j];

    printf("\n=== ALL SAFE SEQUENCES ===\n");
    safeSequences(0);

    if (safeCount > 0)
        printf("Total safe sequences: %d\n", safeCount);
    else
        printf("No safe sequence exists.\n");

    printf("\n=== ALL 120 COMPLETE PROCESS ORDERS ===\n");
    allOrders(0);

    printf("\nTotal possible orders: 120\n");
    printf("Safe complete orders: %d\n", safeOrders);
    printf("Unsafe complete orders: %d\n", unsafeOrders);

    if (safeCount == 0) {
        printf("\n=== UNSAFE PARTIAL SEQUENCES / DEAD ENDS ===\n");

        for (int i = 0; i < P; i++)
            finish[i] = 0;

        for (int j = 0; j < R; j++)
            work[j] = available[j];

        deadEnds(0);

        printf("Total dead-end partial sequences: %d\n",
               deadEndCount);
        printf("\nSystem is UNSAFE.\n");
    } else {
        printf("\nSystem is SAFE.\n");
    }

    return 0;
}
