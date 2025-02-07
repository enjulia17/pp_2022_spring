// Copyright 2022 Kovalev Ruslan
#include <omp.h>
#include <vector>
#include <string>
#include <random>
#include <utility>
#include <iostream>
#include "../../../modules/task_2/kovalev_r_radix_sort_omp/radix_sort_omp.h"


int getMax(std::vector<int> *arr, int sz) {
  int max = arr->at(0);
  for (int i = 1; i < sz; i++)
    if (arr->at(i) > max) max = arr->at(i);
  return max;
}

void vec_gen(std::vector<int>* vec, int len) {
  std::random_device dev;
  std::mt19937 gen(dev());
  for (int i = 0; i < len; i++) {
    vec->at(i) = gen() % 10000;
  }
}

std::vector<int> Merge(std::vector<int> *arr_1, std::vector<int> *arr_2,
                       int sz_1, int sz_2) {
  int i = 0;
  int j = 0;
  int k = 0;
  std::vector<int> res(sz_1 + sz_2);
  while (i < sz_1 && j < sz_2) {
    if (arr_1->at(i) < arr_2->at(j))
      res[k++] = arr_1->at(i++);
    else
      res[k++] = arr_2->at(j++);
  }
  while (i < sz_1) res[k++] = arr_1->at(i++);
  while (j < sz_2) res[k++] = arr_2->at(j++);
  return res;
}

void countingSort(std::vector<int> *array, int size, int place) {
  const int max = 10;
  std::vector<int> output(size, 0);
  std::vector<int> count(max, 0);
  for (int i = 0; i < size; i++) count[(array->at(i) / place) % 10]++;
  for (int j = 1; j < max; j++) count[j] += count[j - 1];

  for (int i = size - 1; i >= 0; i--) {
    output[count[(array->at(i) / place) % 10] - 1] = array->at(i);
    count[(array->at(i) / place) % 10]--;
  }
  for (int i = 0; i < size; i++) array->at(i) = output[i];
}

void Sort(std::vector<int> *arr, int sz) {
  int max = getMax(arr, sz);
  for (int place = 1; max / place > 0; place *= 10)
    countingSort(arr, sz, place);
}

void Odd_Even_Split_Parallel(const std::vector<int> &arr, std::vector<int> *odd,
                             std::vector<int> *even, int size) {
  int i;
#pragma omp parallel shared(arr, odd, even, size) private(i)
  {
#pragma omp for
    for (i = 0; i < size / 2; i++) {
      odd->at(i) = arr[2 * i + 1];
      even->at(i) = arr[2 * i];
    }
  }
}

void Odd_Even_Split(const std::vector<int> &arr, std::vector<int> *odd, std::vector<int> *even,
                    int size) {
  for (int i = 0; i < size / 2; i++) {
    odd->at(i) = arr[2 * i + 1];
    even->at(i) = arr[2 * i];
  }
}

std::vector<int> Odd_Even_Merge(const std::vector<int> &arr, int len) {
  int odd_len = len / 2;
  int even_len = len - odd_len;
  std::vector<int> Odd(odd_len);
  std::vector<int> Even(even_len);
  Odd_Even_Split(arr, &Odd, &Even, len);
  Sort(&Odd, odd_len);
  Sort(&Even, even_len);
  std::vector<int> arr_1;
  arr_1 = Merge(&Odd, &Even, odd_len, even_len);
  for (int i = 1; i < len - 1; i += 2) {
    if (arr_1[i] > arr_1[i + 1]) {
      std::swap(arr_1[i], arr_1[i + 1]);
    }
  }
  return arr_1;
}

std::vector<int> getParallelRadixSort(const std::vector<int> &commonVector,
                                 int lengt) {
  int numberOfThread = omp_get_num_procs();
  int dataPortion = commonVector.size() / numberOfThread;
  std::vector<std::vector<int>> vecOfVec(numberOfThread);
#pragma omp parallel num_threads(numberOfThread)
  {
    int currentThread = omp_get_thread_num();
    std::vector<int> local;
    if (currentThread != numberOfThread - 1) {
      local = {commonVector.begin() + currentThread * dataPortion,
               commonVector.begin() + (currentThread + 1) * dataPortion};
    } else {
      local = {commonVector.begin() + currentThread * dataPortion,
               commonVector.end()};
    }
    int len = local.size();
    Sort(&local, len);
    vecOfVec[currentThread] = local;
  }
  std::vector<int> resultVector = vecOfVec[0];
  for (int i = 1; i < numberOfThread; ++i) {
    int size_1 = resultVector.size();
    int size_2 = vecOfVec[i].size();
    resultVector = Merge(&resultVector, &vecOfVec[i], size_1, size_2);
  }
  return resultVector;
}

std::vector<int> Odd_Even_Merge_Parallel(const std::vector<int> &arr, int len) {
  int odd_len = len / 2;
  int even_len = len - odd_len;
  std::vector<int> Odd(odd_len);
  std::vector<int> Even(even_len);
  Odd_Even_Split_Parallel(arr, &Odd, &Even, len);
  Odd = getParallelRadixSort(Odd, odd_len);
  Even = getParallelRadixSort(Even, even_len);
  std::vector<int> result = Merge(&Odd, &Even, odd_len, even_len);
  return result;
}
