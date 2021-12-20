extern int scanf(char* s, ...);
extern int printf(char* s, ...);

int main() {
  int n = 100;
  int f[10000];
  int i;
  int j;
  i = 0;
  j = 0;
  scanf("%d", &n);
  while (i < n) {
    scanf("%d", &f[i]);
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
    i = i + 1;
  }
}