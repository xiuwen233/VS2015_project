#include <iostream>
#include <cstring>        // for strcpy(), strcat()
#include <io.h>

using namespace std;

void listFiles(const char * dir);

int main()
{
	char dir[200];
	cout << "Enter a directory: ";
	cin.getline(dir, 200);

	listFiles(dir);
	getchar();
	return 0;
}

void listFiles(const char * dir)
{
	char dirNew[200];
	strcpy_s(dirNew, dir);
	strcat_s(dirNew, "\\*.*");    // ��Ŀ¼�������"\\*.*"���е�һ������

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dirNew, &findData);
	if (handle == -1)        // ����Ƿ�ɹ�
		return;

	do
	{
		if (findData.attrib == _A_SUBDIR)
		{
			if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				continue;

			cout <<findData.name << "\t<dir>\n";

			// ��Ŀ¼�������"\\"����������Ŀ¼��������һ������
			strcpy_s(dirNew, dir);
			strcat_s(dirNew, "\\");
			strcat_s(dirNew, findData.name);

			listFiles(dirNew);
		}
		else
		{
			string filename = findData.name; 

			string str = filename.substr(filename.length() - filename.rfind('.'), filename.rfind('.'));
			
			char ste[30] ;
			strcpy_s(ste, str.c_str());
			cout << ste << endl;
			if (strcmp(ste,"mp4") == 0)
			{
				cout << dirNew << "/" << findData.name << "\t" << findData.size << " bytes.\n";
			}
		}
			//cout << dirNew <<"/"<< findData.name << "\t" << findData.size << " bytes.\n" ;
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);    // �ر��������
}