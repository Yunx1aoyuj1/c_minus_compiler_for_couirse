//------ test functions & return statements -------
int y;
int f1(void)       
  { return 1; }

void f2(int y) 
  { y = 2; }

void f3(void) 
  { y = 3; return; }

void main(void) {
  int x;

  // test function calls
  f1();
  x = 2 + f1();
  f2(x);
  f3();

  output(x);
}

