// 判断回文字符串
extern int scanf(char *s, ...);
extern int printf(char *s, ...);

int main() {
  char f[100];
  // 输入
  scanf("%s", f);
  int len;
  len = 0;
  // strlen
  char eof;
  eof = '\0';
  while(f[len] != eof) {
    len = len +  1;
  }
  // 判断
  int i = 0;
  int j = len - 1;
  while(i < j) {
    if (f[i] != f[j]) {
      printf("no");
      return 0;
    }
    i = i + 1;
    j = j - 1;
  }
  printf("yes");
  return 0;
}