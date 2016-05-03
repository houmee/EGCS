/******************************************************************** 
�޸�ʱ��:        2016/03/20 16:24
�ļ�����:        AlgInterface.h
�ļ�����:        huming 
=====================================================================
����˵��:        �㷨�ӿڶ��� 
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:        ���ڰ�˽�г�Ա��ɹ��ó�Ա���ƻ��˳����װ�ԣ� 
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
  double generateRandTime();   //��������ʱ��

public:
  sOutRequestVec    m_outReqVec;          //�ⲿ�����б�
  sPassengerInfoVec m_passengerVec;       //�˿����б�
  sEventVec         m_eventVec;           //ʱ���б�
  CElevatorVec      m_elevatorVec;        //����Ⱥ�б�
  CTools            m_AlgFile;            //�ļ���������
  FlowType          m_eFlowType;          //��ͨ������(�ϸ߷塢С�߷塢���)

  CAlgInterface();           // ����ʹ�ø�����޲ι��캯��
  ~CAlgInterface();          // ��������

  //����ʵ�ֵĺ���--����ͨ��
  void generatePsgFlow();     //��������
  void generateElevatorVec(); //���������б�
  bool isAlgFinished();
  void testPsgFlow();

  //�麯�������������ʵ��
  virtual bool Core_Main(){ return false;};
  virtual void schedule(){};        //����
  virtual void fitness(){};         //��Ӧ��ֵ

  //���ʸ���private���ݽӿں���
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