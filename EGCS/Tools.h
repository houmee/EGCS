/******************************************************************** 
�޸�ʱ��:        2016/04/06 20:56
�ļ�����:        Tools.h
�ļ�����:        huming 
=====================================================================
����˵��:        �ļ����� 
--------------------------------------------------------------------- 
�汾���:        
--------------------------------------------------------------------- 
����˵��:         
*********************************************************************/

#ifndef  __TOOLS_H__
#define	 __TOOLS_H__

#include <iostream>

using namespace std;

class CTools
{
public:
  FILE* m_OutputFilePtr;			// ����ļ�ָ��
  FILE* m_PsgFilePtr;

public:
  CTools();                  // ���캯�� 
  ~CTools(){};                 // �������� 	 

  void InitTools();
  void CloseTools(int item);
  void ReadLine(int index);
  void InitReport();
  void WriteLine(int skipcount);
  void WriteString(char *ch, int repcount);
  void Report();
  void ReadFile(int index);

}; 

#endif