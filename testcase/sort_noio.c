#include <stdio.h>

// 一个排序的例子
// 去掉了输入和输出

int main() {
  int n;
  int f[10000];
  int i;
  int j;
  i = 0;
  j = 0;
  n = 10;
  while (i < n) {
    f[i] = n - i;
    i = i + 1;
  }
  i = 0;
  int id;
  int tmp;
  while (i < n) {
    id = i;
    j = i + 1;
    while (j < n) {
      if (f[j] < f[id]) {
        id = j;
      }
      j = j + 1;
    }
    tmp = f[id];
    f[id] = f[i];
    f[i] = tmp;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    printf("%d ", f[i]);
    i++;
  }
  return 0;
}