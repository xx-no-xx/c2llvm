extern int scanf(char* s, ...);
extern int printf(char* s, ...);

int main() {
  char f[100];
  printf("回文检测：");
  printf("please enter str:");
  scanf("%s", f);
  int len = 0;
  while (f[len]) {
    len = len + 1;
  }
  int i = 0;
  int j = len - 1;
  int flag = 1;
  for (i = 0, j = len - 1; i < j && f[i] == f[j]; i = i + 1, j = j - 1) {
  }
  if (i >= j) {
    printf("yes");
  } else {
    printf("no");
  }
}