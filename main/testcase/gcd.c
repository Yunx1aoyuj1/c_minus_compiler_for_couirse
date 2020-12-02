/* A program to perform Euclid's
Algorithm to compute gcd. */
int gcd (int u, int v)
{  
	if (v == 0) 
		return u ;
	else 
		return gcd(v,u-u/v*v);
}
//6546548
void main(void)
{  
	int x;
       	int y;
	x=input(); 
	y=input();
	output(gcd(x,y));
}

