/******************************************************************** 
�޸�ʱ��:        2016/04/06 17:54
�ļ�����:        MWT.h
�ļ�����:        huming 
=====================================================================
����˵��:        ��С�ȴ�ʱ���㷨 
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:         
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

  bool Core_Main();
  void schedule();
  cElevatorIterator fitness(sOutRequestIterator& reqIter);
  void dispatch(sOutRequestIterator& reqIter, cElevatorIterator& elvtIter);

  void processOuterReqFlow();
  void onClickOutBtn(sPassengerIterator& psg);
};

/*********************************************************************
*********************************************************************/
#endif /* MWT_H */