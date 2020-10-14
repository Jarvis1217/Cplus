#include<iostream>
using namespace std;
class Date
{
	protected:
		int day;
		int month;
		int year;
	public:
		Date(int d=0,int m=0,int y=0)
		{
			day=d;
			month=m;
			year=y;
		}
		void input(int d,int m,int y)
		{
			day=d;
			month=m;
			year=y;
		}
		void output()
		{
			cout<<day<<"/"<<month<<"/"<<year;
		}
};
int main(void)
{
	Date x;
	int d,m,y;
	cout<<"please input:";
	cin>>d>>m>>y;
	x.input(d,m,y);
	x.output();
	return 0;
}
