#include <iostream>    
#include <string>    
#include <io.h>    
using namespace std;

void main()
{
	_finddata_t file;
	int longf;
	string tempName;
	//_findfirst���ص���long��; long __cdecl _findfirst(const char *, struct _finddata_t *)    
	if ((longf = _findfirst("E://cooker/nane/img", &file)) == -1l)
	{
		cout << "�ļ�û���ҵ�!/n";
		return;
	}
	do
	{
		cout << "/n�ļ��б�:/n"<<endl;
		tempName = file.name;
		if (tempName[0] == '.')
			continue;
		cout << file.name;

		if (file.attrib == _A_NORMAL)
		{
			cout << "  ��ͨ�ļ�  ";
		}
		else if (file.attrib == _A_RDONLY)
		{
			cout << "  ֻ���ļ�  ";
		}
		else if (file.attrib == _A_HIDDEN)
		{
			cout << "  �����ļ�  ";
		}
		else if (file.attrib == _A_SYSTEM)
		{
			cout << "  ϵͳ�ļ�  ";
		}
		else if (file.attrib == _A_SUBDIR)
		{
			cout << "  ��Ŀ¼  ";
		}
		else
		{
			cout << "  �浵�ļ�  ";
		}
		cout << endl;
	} while (_findnext(longf, &file) == -1);//int __cdecl _findnext(long, struct _finddata_t *);����ҵ��¸��ļ������ֳɹ��Ļ��ͷ���0,���򷵻�-1   
	_findclose(longf);
	cout << "game over" << endl;
}