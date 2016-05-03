/******************************************************************** 
修改时间:        2016/03/20 16:24
文件名称:        AlgInterface.h
文件作者:        huming 
=====================================================================
功能说明:        算法接口定义 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:        由于把私有成员变成共用成员，破坏了程序封装性！ 
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#ifndef	ALG_INTERFACE_H
#define	ALG_INTERFACE_H

#include "com_def.h"
#include "com_type.h"
#include "Elevator.h"
#include "Tools.h"
#include <vector> 
#include <functional>
#include <algorithm>
#include <ctime>
#include <math.h>

using namespace std;

/*********************************************************************
 * TYPEDEF
 */
typedef vector<sOutRequest>             sOutRequestVec;
typedef vector<sOutRequest>::iterator   sOutRequestIterator;
typedef sOutRequest*                    sOutRequestPtr;

typedef vector<sPassengerInfo>             sPassengerInfoVec;
typedef vector<sPassengerInfo>::iterator   sPassengerIterator;
typedef sPassengerInfo*                    sPassengerInfoPtr;

typedef vector<sEvent>                  sEventVec;
typedef vector<sEvent>::iterator        sEventIterator;
typedef sEvent*                         sEventPtr;

typedef vector<CElevator>             CElevatorVec;
typedef vector<CElevator>::iterator   CElevatorIterator;
typedef CElevator*                    CElevatorPtr;

/*********************************************************************
 * CLASS
 */
class CAlgInterface
{
private: 
  double generateRandTime();   //产生客流时间

public:
  sOutRequestVec    m_outReqVec;          //外部请求列表
  sPassengerInfoVec m_passengerVec;       //乘客流列表
  sEventVec         m_eventVec;           //时间列表
  CElevatorVec      m_elevatorVec;        //电梯群列表
  CTools            m_AlgFile;            //文件操作对象
  FlowType          m_eFlowType;          //交通流类型(上高峰、小高峰、随机)

  CAlgInterface();           // 子类使用父类的无参构造函数
  ~CAlgInterface();          // 析构函数

  //父类实现的函数--子类通用
  void generatePsgFlow();     //产生客流
  void generateElevatorVec(); //产生电梯列表
  bool isAlgFinished();
  void testPsgFlow();

  //虚函数部分子类必须实现
  virtual bool Core_Main(){ return false;};
  virtual void schedule(){};        //调度
  virtual void fitness(){};         //适应度值

  //访问父类private数据接口函数
  //float             getSystime(){ return m_fSystemTime; };
  //sOutRequestVec    getOutReqVec(){ return m_outReqVec;};
  //sPassengerInfoVec getPsgArr(){ return m_passengerVec;};
  //sEventVec         getEventArr(){ return m_eventVec;};
  //CElevatorVec      getElevatorArr(){ return m_elevatorVec;};
  //CTools            getAlgFile(){ return m_AlgFile;};
  //FlowType          getFlowType(){ return m_eFlowType;};
};

/*********************************************************************
*********************************************************************/
#endif /* ALG_INTERFACE_H */