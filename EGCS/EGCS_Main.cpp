/******************************************************************** 
修改时间:        2016/03/24 16:18
文件名称:        EGCS_Main.cpp
文件作者:        huming 
=====================================================================
功能说明:        电梯群控系统主函数 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "EGCS.h"

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @函数名：   main
 *
 * @描述：      Start of application.
 *
 * @参数：      none
 *
 * @返回：      none
 *********************************************************************
 */
int main( void )
{
    //CElevator ce;
    CEGCS egcs;
    egcs.StartWork();
    return 0;
}


/*********************************************************************
*********************************************************************/

//#include <vector>
//#include <algorithm>
//#include <functional>
//
//
//using namespace std;
//struct TItem
//{
//  int m_i32Type;
//  int m_i32ID;
//
//
//  bool operator <(const TItem& rhs) const // 升序排序时必须写的函数
//  {
//    return m_i32Type < rhs.m_i32Type;
//  }
//  bool operator >(const TItem& rhs) const // 降序排序时必须写的函数
//  {
//    return m_i32Type > rhs.m_i32Type;
//  }
//};
//
//
//int main()
//{
//  vector<TItem> stItemVec;
//
//
//  TItem stItem1;
//  stItem1.m_i32Type = 1;
//  stItem1.m_i32ID = 1;
//
//
//  TItem stItem2;
//  stItem2.m_i32Type = 2;
//  stItem2.m_i32ID = 2;
//
//
//  TItem stItem3;
//  stItem3.m_i32Type = 3;
//  stItem3.m_i32ID = 3;
//
//
//  TItem stItem4;
//  stItem4.m_i32Type = 2;
//  stItem4.m_i32ID = 4;
//
//
//  stItemVec.push_back(stItem1);
//  stItemVec.push_back(stItem2);
//  stItemVec.push_back(stItem3);
//  stItemVec.push_back(stItem4);
//
//
//  // 升序排序
//  sort(stItemVec.begin(), stItemVec.end(), less<TItem>()); 
//  // 或者sort(ctn.begin(), ctn.end());   默认情况为升序
//
//
//  for (size_t i = 0; i < stItemVec.size(); i++)
//    printf("type: %d, id: %d\n", stItemVec[i].m_i32Type, stItemVec[i].m_i32ID);
//
//
//  printf("--\n");
//
//
//  // 降序排序
//  sort(stItemVec.begin(), stItemVec.end(), greater<TItem>());
//
//
//  for (size_t i = 0; i < stItemVec.size(); i++)
//    printf("type: %d, id: %d\n", stItemVec[i].m_i32Type, stItemVec[i].m_i32ID);
//
//
//  return 0;
//}
