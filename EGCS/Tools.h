/******************************************************************** 
修改时间:        2016/04/06 20:56
文件名称:        Tools.h
文件作者:        huming 
=====================================================================
功能说明:        文件操作 
--------------------------------------------------------------------- 
版本编号:        
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

#ifndef  __TOOLS_H__
#define	 __TOOLS_H__

#include <iostream>

using namespace std;

class CTools
{
public:
  FILE* m_OutputFilePtr;			// 输出文件指针
  FILE* m_PsgFilePtr;

public:
  CTools();                  // 构造函数 
  ~CTools(){};                 // 析构函数 	 

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