#include<iostream>
using namespace std;
void change(int &a,int &b);
int main()
{
	int x,y;
	cin>>x>>y;
	change(x,y);
	cout<<x<<' '<<y;
	return 0;
}
void change(int &a,int &b)
{
	int temp;
	if(a>b)
	{
		temp=a;
		a=b;
		b=temp;
	}
}
