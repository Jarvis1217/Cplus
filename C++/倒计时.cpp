#include<iostream>
#include<windows.h>
using namespace std;

//隐藏光标
void HideCursor()
{
 CONSOLE_CURSOR_INFO cursor_info = {1, 0}; 
 SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
//显示光标
void DispCursor()
{
 CONSOLE_CURSOR_INFO cursor_info = {1, 1}; 
 SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
} 

int main(void)
{
	int sec;
	cout<<"输入开始时间(秒):";
	cin>>sec;
	cout<<"点击回车键开始计时";
	while(getchar()!='\n')
	{
	}
	
	system("cls");
	//开始倒计时
	 HideCursor();
	 while(sec!=0)
	 {
	 	cout<<"距离时间结束还剩"<<sec<<"s";
	 	sec=sec-1;
	 	Sleep(1000);
	 	system("cls");
	 }
	 cout<<"Time Over";
	 DispCursor();
	 return 0;
}
