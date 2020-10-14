#include<iostream>
using namespace std;
int main(void)
{
	int *p=new int[20];
	p[0]=p[1]=1;
	int i;
	for(i=2;i<20;i++)
	{
		p[i]=p[i-1]+p[i-2];
	}
	for(i=0;i<20;i++)
	{
	 cout<<p[i]<<" ";
	 if((i+1)%5==0)
		 cout<<endl;	
	}
	delete []p;
	return 0;
}
