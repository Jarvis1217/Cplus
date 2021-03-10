#include<iostream>
#include<string.h>
using namespace std;
class student
{
	protected:
		string name;
		int id;
		
		int Cgrade;            //程序设计 
		int Xgrade;            //信号处理 
		int Sgrade;            //数据结构 
		
		double average;       //每个学生平均分 
		double sumaver;      //每门课程平均分 
	public:
		//录入学生信息 
	    void init(string Name,int ID,int x,int y,int z)
		{
			name=Name;
			id=ID;
			Cgrade=x;
			Xgrade=y;
			Sgrade=z;
		}
		//计算每个学生平均分 
		double aver(int x,int y,int z)
		{
			return average=((x+y+z)/3);
		}
		//输出学生信息 
		void print()
		{
			cout<<"->";
			cout<<"姓名："<<name<<' ';
			cout<<"学号："<<id<<' ';
			cout<<endl;
			cout<<"程序设计："<<Cgrade<<' ';
			cout<<"信号处理："<<Xgrade<<' ';
			cout<<"数据结构："<<Sgrade<<' '<<endl;
			cout<<"平均成绩："<<average<<endl;
			cout<<endl;
		}
		//输出不及格人数 
		void failprint(int count1,int count2,int count3)
		{
			cout<<"----------------------"<<endl;
		    cout<<"程序设计不及格人数："<<count1<<endl;
			cout<<"信号处理不及格人数："<<count2<<endl;
			cout<<"数据结构不及格人数："<<count3<<endl;
		}
} ;

int main()
{
	int n,i,j;
	double Csum=0,Xsum=0,Ssum=0;
	int count1=0,count2=0,count3=0;
	string Name;
	int ID,x,y,z;
	cout<<"请输入学生总数：";
	cin>>n;
	cout<<endl;
	student stu[n];
	for(i=0;i<n;i++)
	{
		cout<<"请输入学生"<<i+1<<"姓名：";
		cin>>Name;
		cout<<"请输入学生"<<i+1<<"学号：";
		cin>>ID;
		cout<<"请输入学生"<<i+1<<"成绩：";
		cin>>x>>y>>z;
		Csum=Csum+x;        //程序设计总分 
		Xsum=Xsum+y;        //信号处理总分 
		Ssum=Ssum+z;        //数据结构总分 
		if(x<60) count1++;                  //统计程序设计不及格人数 
		if(y<60) count2++;                  //统计信号处理不及格人数 
		if(z<60) count3++;                  //统计数据结构不及格人数 
		stu[i].init(Name,ID,x,y,z);        //录入学生信息 
		stu[i].aver(x,y,z);               //计算学生平均分 
		cout<<endl;
	}
	cout<<"----------------------"<<endl;
	for(j=0;j<n;j++)
	{
		stu[j].print();
	}
	stu[0].failprint(count1,count2,count3);
	cout<<"----------------------"<<endl;
	cout<<"程序设计平均分为:"<<Csum/n<<endl;
	cout<<"信号处理平均分为:"<<Xsum/n<<endl;
	cout<<"数据结构平均分为:"<<Ssum/n<<endl;
}
