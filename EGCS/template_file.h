/******************************************************************** 
修改时间:        2016/04/10 17:13
文件名称:        template_file.h
文件作者:        huming 
=====================================================================
功能说明:        模板文件 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

#ifndef	TEMPLATE_FILE_H
#define	TEMPLATE_FILE_H

#include "iostream"
#include <functional>
#include <algorithm>
#include <vector> 
#include <functional>
#include <algorithm>
/********************************************************************
*  @name     : insertElement    
*  @brief    : 
*  @param    : vec 
*  @param    : ind 
*  @return   : void
********************************************************************/ 
template<class T1,class T2>
void insertElement(T1& vec,T2 ind)
{
  vec.push_back(ind);
}

/********************************************************************
*  @name     : deleteElement    
*  @brief    : 
*  @param    : vec 
*  @param    : ind 
*  @param    : it 
*  @return   : bool
********************************************************************/ 
template<class T1, class T2, class T3>
bool deleteElement(T1& vec, T2 ind, T3 iter)
{
  //T3 j = find( vec->begin(), vec->end(), ind );
  T3 j = queryElement(vec, ind ,iter);
  if( j != vec.end() )
  {
    vec.erase(j);
    return true;
  }
  else
    return false;
}

/********************************************************************
*  @name     : queryElement    
*  @brief    : 
*  @param    : vec 
*  @param    : ind 
*  @return   : T3(iter)
********************************************************************/ 
template<class T1, class T2, class T3>
T3 queryElement( T1& vec, T2 ind, T3 it)
{
  return (find( vec.begin(), vec.end(), ind )); 
}

/********************************************************************
*  @name     : sortElement    
*  @brief    : 
*  @param    : vec 
*  @param    : s 
*  @return   : void
********************************************************************/ 
template<class T1, class T2>
void sortElement(T1& vec, T2 s)
{
  sort(vec.begin(), vec.end(), less<T2>());
}


/********************************************************************
*  @name     : printVector    
*  @brief    : 
*  @param    : vector 
*  @return   : void
********************************************************************/ 
template<class T1>
void printVector(vector<T1> &vector)  
{  
  cout << endl;  
  cout << "vector size:" << vector.size() << endl;  
  cout << "vector capacity:" << vector.capacity() << endl;  
}  
/*********************************************************************
*********************************************************************/
#endif /* TEMPLATE_FILE_H */