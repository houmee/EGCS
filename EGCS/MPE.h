/******************************************************************** 
修改时间:        2016/04/06 17:53
文件名称:        MPE.h
文件作者:        huming 
=====================================================================
功能说明:        最小能耗算法 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/
#ifndef	MPE_H
#define	MPE_H

#include "AlgInterface.h"

using namespace std;
/*********************************************************************
* CLASS
*/
class CMPE : public CAlgInterface
{
public:
  CMPE();
  ~CMPE(){};

  bool Core_Main();
  void schedule();
  cElevatorIterator fitness(sOutRequestIterator& reqIter);
  void dispatch(sOutRequestIterator& reqIter, cElevatorIterator& elvtIter);

  void processOuterReqFlow();
  void onClickOutBtn(sPassengerIterator& psg);
};

/*********************************************************************
*********************************************************************/
#endif /* MPE_H */