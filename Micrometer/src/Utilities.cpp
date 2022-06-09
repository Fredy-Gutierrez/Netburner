/*
 * Utilities.cpp
 *
 *  Created on: 6 oct 2021
 *      Author: Integra Fredy
 */
#include "headers/Utilities.h"

void swap(struct InputsInfoMod* xp, InputsInfoMod* yp)
{
	InputsInfoMod temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Function to perform Selection Sort
void orderByAsc(InputsInfoMod arr[], int n)
{
    int i, j, min_idx;
    // One by one move boundary of unsorted subarray
    for (i = 0; i < n - 1; i++) {
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j].inputNumber < arr[min_idx].inputNumber)
                min_idx = j;
        // Swap the found minimum element
        // with the first element
        swap(&arr[min_idx], &arr[i]);
    }
}

