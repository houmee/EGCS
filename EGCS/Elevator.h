/******************************************************************** 
�޸�ʱ��:        2016/03/20 16:37
�ļ�����:        Elevator.h
�ļ�����:        huming 
=====================================================================
����˵��:        ����ģ�ͽӿ� 
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:         
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
  int8              m_iElvtID;        //�������
  uint8             m_iCurFlr;        //��������¥��
  uint8             m_iNextStopFlr;   //��һͣ��¥��
  double            m_dCurRunDis;     //������һͣ��¥���ƶ�����
  uint8             m_iCurPsgNum;     //��ǰ�˿���Ŀ
  RunDir            m_eRundir;
  ElvtRunState      m_eCurState;      //��ǰ״̬
  double            m_dStartTime;
  double            m_dLastStateTime;
  double            m_dNextStateTime; //������һ״̬ʱ��
  bool              m_isSchedule;     //�Ƿ񱻵���
  bool              m_canHandle;      //�����Ƿ�ɲ���
  sRunItem          m_lastRunItem;   //��һ�����б���
  sRunItemVec       m_sRunTable;      //��������ָʾ�б�
  CTools            m_ElvtFile;

  CElevator();         // ���캯��
  ~CElevator(){};        // ��������
  
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