/******************************************************************** 
修改时间:        2016/03/20 16:37
文件名称:        Elevator.h
文件作者:        huming 
=====================================================================
功能说明:        电梯模型接口 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#ifndef	ELEVATOR_H
#define	ELEVATOR_H


#include "com_def.h"
#include "com_type.h"
#include "Tools.h"
#include <vector> 
#include <functional>
#include <algorithm>

using namespace std;

typedef vector<sRunItem>             sRunItemVec;
typedef vector<sRunItem>::iterator   sRunItemIterator;

typedef vector<sOutRequest>             sOutRequestVec;
typedef vector<sOutRequest>::iterator   sOutRequestIterator;

typedef vector<sOutRequest>             sOutRequestVec;
typedef vector<sOutRequest>::iterator   sOutRequestIterator;

typedef vector<sPassengerInfo>             sPassengerInfoVec;
typedef vector<sPassengerInfo>::iterator   sPassengerIterator;

typedef vector<sEvent>                  sEventVec;
typedef vector<sEvent>::iterator        sEventIterator;

/*********************************************************************
 * CLASS
 */
class CElevator
{
public:
  int8              m_iElvtID;        //电梯序号
  uint8             m_iCurFlr;        //电梯所在楼层
  uint8             m_iNextStopFlr;   //下一停靠楼层
  double            m_dCurRunDis;     //距离上一停靠楼层移动距离
  uint8             m_iCurPsgNum;     //当前乘客数目
  RunDir            m_eRundir;
  ElvtRunState      m_eCurState;      //当前状态
  double            m_dStartTime;
  double            m_dLastStateTime;
  double            m_dNextStateTime; //电梯下一状态时间
  bool              m_isSchedule;     //是否被调度
  bool              m_canHandle;      //电梯是否可操作
  sRunItem          m_lastRunItem;   //上一个运行表项
  sRunItemVec       m_sRunTable;      //电梯运行指示列表
  CTools            m_ElvtFile;

  CElevator();         // 构造函数
  ~CElevator(){};        // 析构函数
  
  void Elevator_Main(sOutRequestVec& reqVec, sPassengerInfoVec& psgVec);
  void changeNextStop();
  void gotoNextDest();
  void onClickInnerBtn(sPassengerIterator& psg);
  void initELevator(int id, CTools& tools);
  //bool deleteTask(sRunIndex ind);
  //void insertTask(sRunIndex ind);
  //void sortTask(void);
  void showElevator(uint8 op);
  void getTaskPriority(sRunItem& ind);
  void updateRunInfo();
  sTargetVal trytoDispatch(sOutRequestIterator reqIter);
  sTargetVal runfromXtoY(sRunItem x, sRunItem y);
  void processInnerPsgFlow(sPassengerInfoVec& psgVec);
  void psgLeave(sPassengerIterator& psg);
  void psgEnter(sPassengerIterator& psg);
  void updateRunTable();
  void insertRunTableItem( sRunItem runInd );
  void deleteRunTableItem( sRunItem runInd );
};

/*********************************************************************
*********************************************************************/
#endif /* ELEVATOR_H */