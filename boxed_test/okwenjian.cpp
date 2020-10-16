#include <iostream>    
#include <string>    
#include <io.h>    
using namespace std;

void main()
{
	_finddata_t file;
	int longf;
	string tempName;
	//_findfirst返回的是long型; long __cdecl _findfirst(const char *, struct _finddata_t *)    
	if ((longf = _findfirst("E://cooker/nane/img", &file)) == -1l)
	{
		cout << "文件没有找到!/n";
		return;
	}
	do
	{
		cout << "/n文件列表:/n"<<endl;
		tempName = file.name;
		if (tempName[0] == '.')
			continue;
		cout << file.name;

		if (file.attrib == _A_NORMAL)
		{
			cout << "  普通文件  ";
		}
		else if (file.attrib == _A_RDONLY)
		{
			cout << "  只读文件  ";
		}
		else if (file.attrib == _A_HIDDEN)
		{
			cout << "  隐藏文件  ";
		}
		else if (file.attrib == _A_SYSTEM)
		{
			cout << "  系统文件  ";
		}
		else if (file.attrib == _A_SUBDIR)
		{
			cout << "  子目录  ";
		}
		else
		{
			cout << "  存档文件  ";
		}
		cout << endl;
	} while (_findnext(longf, &file) == -1);//int __cdecl _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1   
	_findclose(longf);
	cout << "game over" << endl;
}