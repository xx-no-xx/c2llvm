int main() {
  int n = 10;
  int f[10000];
  int i;
  int j;
  i = 0;
  j = 0;
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
    int load_load = f[i];
    i = i + 1;
  }
}