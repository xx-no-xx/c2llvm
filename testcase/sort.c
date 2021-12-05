#include <stdio.h>

// 一个排序的例子

int main() {
  int n;
  int f[10000];
  int i;
  int j;
  i = 0;
  j = 0;
  /* 输入 */
  scanf("%d", &n);
  while (i < n) {
    scanf("%d", &f[i]);
    i = i + 1;
  }
  /* 排序 */
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
  /* 输出 */
  i = 0;
  while (i < n) {
    printf("%d ", f[i]);
    i++;
  }
  return 0;
}