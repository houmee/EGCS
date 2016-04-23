/******************************************************************** 
�޸�ʱ��:        2016/03/25 16:29
�ļ�����:        Passenger.h
�ļ�����:        huming 
=====================================================================
����˵��:         
--------------------------------------------------------------------- 
�汾���:        
--------------------------------------------------------------------- 
����˵��:         
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
  uint8		  m_iPsgID;          //�˿����
  uint8		  m_iReqCurFlr;      //��������¥���
  uint8		  m_iDestFlr;        //Ŀ��¥��
  float     m_fReqTime;        //����ʱ��
  float     m_fWaitTime;       //�ȴ�ʱ��
  float	    m_fTravelTime;     //����ʱ��
  PsgState  m_ePsgState;       //�˿�״̬

public:
  CPassenger(){};           // ���캯��
  ~CPassenger(){};          // ��������

  bool operator <(const CPassenger& rhs) const;   //����<�����
  bool operator >(const CPassenger& rhs) const;   
  bool operator == (const CPassenger& rhs) const;
  void initPassenger(uint8 id);
};


/*********************************************************************
*********************************************************************/
#endif /* PASSENGER_H */