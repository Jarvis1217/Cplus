#include<iostream>
#include<iomanip>
double Girth(double width, double height); 
double Area(double width, double height);
using namespace std;
int main (void)
{
	cout<<"请输入矩形长与宽:";
	double a,b;
	cin>>a>>b;
	 Girth(a,b);
	 Area(a,b);
	 return 0;
}
double Girth(double width, double height)//计算矩形周长 
{
	double z;
	z=2*(width+height);
	cout<<"周长="<<fixed<<setprecision(2)<<z<<endl;
}
double Area(double width, double height)//计算矩形面积
{
	double s;
	s=width*height;
	cout<<"面积="<<fixed<<setprecision(2)<<s;
} 
