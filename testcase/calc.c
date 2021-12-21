extern int scanf(char* s, ...);
extern int printf(char* s, ...);

int main() {
  char exp[10000];
  char op_stack[100];
  char exp_post[10000];
  int pos[10000];
  int value_stack[10000];
  scanf("%s", exp);
  int len = 0;
  while (exp[len]) {
    len = len + 1;
  }
  int i;
  int flag;
  int top = 0;
  int value_top = 0;
  int len_p = 0;
  i = 0;

  while (i < len) {
    if (exp[i] <= '9' && exp[i] >= '0') {
      value_stack[value_top] = 0;
      while (i < len && exp[i] <= '9' && exp[i] >= '0') {
        char c = exp[i];
        int value = 0;
        if (exp[i] == '0') {
          value = 0;
        } else {
          if (exp[i] == '1') {
            value = 1;
          } else {
            if (exp[i] == '2') {
              value = 2;
            } else {
              if (exp[i] == '3') {
                value = 3;
              } else {
                if (exp[i] == '4') {
                  value = 4;
                } else {
                  if (exp[i] == '5') {
                    value = 5;
                  } else {
                    if (exp[i] == '6') {
                      value = 6;
                    } else {
                      if (exp[i] == '7') {
                        value = 7;
                      } else {
                        if (exp[i] == '8') {
                          value = 8;
                        } else {
                          if (exp[i] == '9') {
                            value = 9;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        value_stack[value_top] = value_stack[value_top] * 10;
        value_stack[value_top] = value_stack[value_top] + value;
        i = i + 1;
      }
      value_top = value_top + 1;
    } else {
      if (exp[i] == '+' || exp[i] == '-') {
        flag = 1;
        if (top == 0) {
          flag = 0;
        }
        while (flag) {
          if (op_stack[top - 1] == '+' || op_stack[top - 1] == '-' ||
              op_stack[top - 1] == '*' || op_stack[top - 1] == '/') {
            exp_post[len_p] = op_stack[top - 1];
            pos[len_p] = value_top;
            value_top = value_top + 1;
            len_p = len_p + 1;
            top = top - 1;
            if (top == 0) {
              flag = 0;
            }
          } else {
            flag = 0;
          }
        }
        op_stack[top] = exp[i];
        top = top + 1;
      }
      if (exp[i] == '*' || exp[i] == '/') {
        flag = 1;
        if (top == 0) {
          flag = 0;
        }
        while (flag) {
          if (op_stack[top - 1] == '*' || op_stack[top - 1] == '/') {
            exp_post[len_p] = op_stack[top - 1];
            pos[len_p] = value_top;
            value_top = value_top + 1;
            len_p = len_p + 1;
            top = top - 1;
            if (top == 0) {
              flag = 0;
            }
          } else {
            flag = 0;
          }
        }
        op_stack[top] = exp[i];
        top = top + 1;
      }
      if (exp[i] == '(') {
        op_stack[top] = exp[i];
        top = top + 1;
      }
      if (exp[i] == ')') {
        flag = 1;
        while (flag) {
          if (op_stack[top - 1] != '(') {
            exp_post[len_p] = op_stack[top - 1];
            pos[len_p] = value_top;
            value_top = value_top + 1;
            len_p = len_p + 1;
            top = top - 1;
            if (top == 0) {
              flag = 0;
            }
          } else {
            flag = 0;
          }
        }
        top = top - 1;
      }
      i = i + 1;
    }
  }

  while (top) {
    exp_post[len_p] = op_stack[top - 1];
    pos[len_p] = value_top;
    value_top = value_top + 1;
    top = top - 1;
    len_p = len_p + 1;
  }

  int j = 0;

  int calc_stack[100000];
  int ctop = 0;
  for (i = 0; i < value_top; i = i + 1) {
    if (pos[j] != i) {
      calc_stack[ctop] = value_stack[i];
      ctop = ctop + 1;
    } else {
      int result;
      if (exp_post[j] == '+') {
        result = calc_stack[ctop - 2] + calc_stack[ctop - 1];
      }
      if (exp_post[j] == '-') {
        result = calc_stack[ctop - 2] - calc_stack[ctop - 1];
      }
      if (exp_post[j] == '*') {
        result = calc_stack[ctop - 2] * calc_stack[ctop - 1];
      }
      if (exp_post[j] == '/') {
        result = calc_stack[ctop - 2] / calc_stack[ctop - 1];
      }
      ctop = ctop - 2;
      calc_stack[ctop] = result;
      ctop = ctop + 1;
      j = j + 1;
    }
  }

  exp_post[len_p] = '\0';

  printf("result:%d\n", calc_stack[0]);
}
