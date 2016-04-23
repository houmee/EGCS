/******************************************************************** 
修改时间:        2016/04/06 17:54
文件名称:        MWT.h
文件作者:        huming 
=====================================================================
功能说明:        最小等待时间算法 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

#ifndef	MWT_H
#define	MWT_H

#include "AlgInterface.h"

using namespace std;
/*********************************************************************
 * CLASS
 */
class CMWT : public CAlgInterface
{
public:
  CMWT();
  ~CMWT(){};

  void Core_Main();
  void schedule();
  CElevatorIterator fitness(sOutRequestIterator& reqIter);
  void dispatch(sOutRequestIterator& reqIter, CElevatorIterator& elvtIter);

  void processOuterReqFlow();
  void onClickOutBtn(sPassengerIterator& psg);
};

/*********************************************************************
*********************************************************************/
#endif /* MWT_H */