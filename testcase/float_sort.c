extern int scanf(char* s, ...);
extern int printf(char* s, ...);

int main() {
  int n = 100;
  float f[10000];

  printf("排序（浮点数版本）\n");
  printf("please enter n:");
  scanf("%d", &n);

  printf("please enter f[i]:");
  int i = 0;
  int j = 0;
  for (i = 0; i < n; i = i + 1) {
    scanf("%lf", &f[i]);
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
    float tmp = f[id];
    f[id] = f[i];
    f[i] = tmp;
  }
  printf("result:");
  for (i = 0; i < n; i = i + 1) {
    printf("%.2lf ", f[i]);
  }
}