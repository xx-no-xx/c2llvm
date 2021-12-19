int main() {
  char f[100];
  f[0] = '\n';
  f[1] = 'a';
  f[2] = '\n';
  f[3] = '\0';

  int len = 0;

  char eof = '\0';

  while (eof < f[len]) {
    len = len + 1;
  }

  int i = 0;
  int j = len - 1;

  int flag = 1;
  while (i < j) {
    if (f[i] < f[j]) {
      flag = 0;
    }
    if (f[j] < f[i]) {
      flag = 0;
    }
    i = i + 1;
    j = j - 1;
  }
  if (flag) {
    int xxxx = flag;
  }
}