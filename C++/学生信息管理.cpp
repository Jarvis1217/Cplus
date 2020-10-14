#include<iostream>
#include<string>
using namespace std;

class student
{
	private:
		int ID;
		string Name;
		double Grade;
		static int N;
		static double Gradesum;
	public:
		student(int id,string name,double grade)
		{
			ID=id;
			Name=name;
			Grade=grade;
			N++;
			Gradesum+=grade;
		}
		void disp()
		{
			cout<<"->"<<"Name:"<<Name<<'/'<<"ID:"<<ID;
			cout<<endl;
			cout<<"Grade:"<<Grade<<endl;
			cout<<endl;
		}
		void output()
		{
			cout<<"Number of students is:"<<N<<endl;
			cout<<"Average score is:"<<Gradesum/3;
		}
};

double	student::Gradesum=0;	
int	student::N=0;

int main(void)
{
	int	i;
	student	stu[3]={
	student(1,"Tom",90),
	student(2,"Kimi",90),
	student(3,"Petter",90)
	};
	for(i=0;i<3;i++)
	stu[i].disp();
    stu[0].output();
	return 0;
}
