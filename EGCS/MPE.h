/******************************************************************** 
�޸�ʱ��:        2016/04/06 17:53
�ļ�����:        MPE.h
�ļ�����:        huming 
=====================================================================
����˵��:        ��С�ܺ��㷨 
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:         
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