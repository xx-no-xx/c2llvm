extern int scanf(char* s, ...);
extern int printf(char* s, ...);

int main() {
  int n = 100;
  int f[10000];

  printf("排序\n");
  printf("please enter n:");
  scanf("%d", &n);

  printf("please enter f[i]:");
  int i = 0;
  int j = 0;
  for (i = 0; i < n; i = i + 1) {
    scanf("%d", &f[i]);
  }
  for (i = 0; i < n; i = i + 1) {
    int id = i;
    j = i + 1;
    while (j < n) {
      if (f[j] < f[id]) {
        id = j;
      }
      j = j + 1;
    }
    int tmp = f[id];
    f[id] = f[i];
    f[i] = tmp;
  }
  printf("result:");
  for (i = 0; i < n; i = i + 1) {
    printf("%d ", f[i]);
  }
}