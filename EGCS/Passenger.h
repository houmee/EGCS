/******************************************************************** 
修改时间:        2016/03/25 16:29
文件名称:        Passenger.h
文件作者:        huming 
=====================================================================
功能说明:         
--------------------------------------------------------------------- 
版本编号:        
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#ifndef	PASSENGER_H
#define	PASSENGER_H

#include "com_def.h"
#include "com_type.h"
#include <vector> 
#include <functional>
#include <algorithm>

using namespace std;
/*********************************************************************
 * CLASS
 */
class CPassenger
{
private:
  uint8		  m_iPsgID;          //乘客序号
  uint8		  m_iReqCurFlr;      //请求所在楼层号
  uint8		  m_iDestFlr;        //目的楼层
  float     m_fReqTime;        //请求时间
  float     m_fWaitTime;       //等待时间
  float	    m_fTravelTime;     //乘梯时间
  PsgState  m_ePsgState;       //乘客状态

public:
  CPassenger(){};           // 构造函数
  ~CPassenger(){};          // 析构函数

  bool operator <(const CPassenger& rhs) const;   //重载<运算符
  bool operator >(const CPassenger& rhs) const;   
  bool operator == (const CPassenger& rhs) const;
  void initPassenger(uint8 id);
};


/*********************************************************************
*********************************************************************/
#endif /* PASSENGER_H */