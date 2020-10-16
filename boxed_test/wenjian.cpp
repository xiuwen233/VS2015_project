#include <iostream>  
#include <string>  
#include <io.h>  

using namespace std;

//������ǰĿ¼�µ��ļ��к��ļ�,Ĭ���ǰ���ĸ˳�����  
bool TraverseFiles(string path, int &file_num)
{
	_finddata_t file_info;
	string current_path = path + "/*.*"; //���Զ������ĺ�׺Ϊ*.exe��*.txt���������ض���׺���ļ���*.*��ͨ�����ƥ����������,·�����ӷ��������б��/���ɿ�ƽ̨  
										 //���ļ����Ҿ��  
	long handle = _findfirst(current_path.c_str(), &file_info);
	//����ֵΪ-1�����ʧ��  
	cout << handle<<endl;
	if (-1 == handle)
		return false;
	int flag = 0;
	do
	{
		//�ж��Ƿ���Ŀ¼  
		string attribute;
		if (file_info.attrib == _A_SUBDIR) //��Ŀ¼  
			attribute = "dir";
		else
			attribute = "file";
		cout << attribute << endl;
		cout << ++flag << endl;
		
		//����ļ���Ϣ������,�ļ���(����׺)���ļ�����޸�ʱ�䡢�ļ��ֽ���(�ļ�����ʾ0)���ļ��Ƿ�Ŀ¼  
		cout << file_info.name << "  " << file_info.time_create <<"  " << file_info.size << "  " << attribute << endl; //��õ�����޸�ʱ����time_t��ʽ�ĳ����ͣ���Ҫ����������ת������ʱ����ʾ  
		file_num++;
		cout << file_num << endl;
	} while (_findnext(handle, &file_info) == 0);  //����0�������  
											   //�ر��ļ����  
	_findclose(handle);
	return true;
}

//������ȵݹ������ǰĿ¼���ļ��к��ļ������ļ��к��ļ�  
void DfsFolder(string path, int layer)
{
	_finddata_t file_info;
	string current_path = path + "/*.*"; //Ҳ������/*��ƥ������  
	int handle = _findfirst(current_path.c_str(), &file_info);
	//����ֵΪ-1�����ʧ��  
	if (-1 == handle)
	{
		cout << "cannot match the path" << endl;
		return;
	}

	do
	{
		//�ж��Ƿ���Ŀ¼  
		if (file_info.attrib == _A_SUBDIR)
		{
			//�ݹ������Ŀ¼  
			//��ӡ�Ǻŷ�ӳ����Ȳ��  
			for (int i = 0; i<layer; i++)
				cout << "--";
			cout << file_info.name << endl;
			int layer_tmp = layer;
			if (strcmp(file_info.name, "..") != 0 && strcmp(file_info.name, ".") != 0)  //.�ǵ�ǰĿ¼��..���ϲ�Ŀ¼�������ų������������  
				DfsFolder(path + '/' + file_info.name, layer_tmp + 1); //��windows�¿�����\\ת��ָ��������Ƽ�  
		}
		else
		{
			//��ӡ�Ǻŷ�ӳ����Ȳ��  
			for (int i = 0; i<layer; i++)
				cout << "--";
			cout << file_info.name << endl;
		}
	} while (!_findnext(handle, &file_info));  //����0�������  
											   //�ر��ļ����  
	_findclose(handle);
}

//���ַ�����飨���ں�׺����飩
char *substr(char *str1, char *str2)
{
	char * p = str1;
	while (*p)
	{
		if (memcmp(p, str2, strlen(str2)) == 0)
			return p;
		p++;
	}
	return NULL;
}


//int main(int argc, char *argv[])
//{
//	////��������Ŀ¼  
//	//int file_num = 0;
//	//if (!TraverseFiles("E://cooker/nane/img", file_num))  //�˴�·�����ӷ�ֻ����/�����̷���Сд����  
//	//	cout << "traverse files failed" << endl;
//	//cout << "-------------------\n" << "file number: " << file_num << endl;
//
//	////�ݹ�����ļ���  
//	//char cmdstr[1024];
//	//char linrchartmp[];//"**********.jpg"
//	//sprintf(cmdstr,"ren %s\%s *",linrchar[], linrchartmp);
//
//	////DfsFolder("E://personal_profile/tinyxml", 0);
//	//system("pause");
//	//return 0;
//
//	char filename[] = "file_list.txt"; //�ļ���
//	FILE *fp;
//	char strLine[1024];//ÿ������ȡ���ַ���
//	if ((fp = fopen(filename, "r")) == NULL) //�ж��ļ��Ƿ���ڼ��ɶ�
//	{
//		printf("input directory error!");
//		return -1;
//	}
//
//	while (!feof(fp))
//	{
//		fgets(strLine, 1024, fp);  //��ȡһ��
//
//		for (int i = 0; i < 1024; i++)
//		{
//			if (strLine[i] == '\r' || strLine[i] == '\n')
//				strLine[i] = 0;
//		}
//
//		if (substr(strLine, ".jpg@360h") != NULL )
//		{
//			char cmdstr[1024];
//			char linrchartmp[1024];
//			for (int i = 0; i < 1024; i++)
//			{
//				if (cmdstr[i] != '@')
//				{
//					linrchartmp[i] = cmdstr[i];
//				}
//				else {
//					break;
//				}					
//			}
//			sprintf(cmdstr,"ren %s  %s *", strLine, linrchartmp);
//			system(cmdstr);
//		}
//		else {
//			printf("NO\n");
//		}
//	}
//	fclose(fp);
//}