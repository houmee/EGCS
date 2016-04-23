/******************************************************************** 
�޸�ʱ��:        2016/03/20 16:41
�ļ�����:        AlgInterface.cpp
�ļ�����:        huming 
=====================================================================
����˵��:        �㷨�ӿ�
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "AlgInterface.h"
#include "template_file.h"

/*********************************************************************
 * GLOBAL VALUE
 */
int gUnusedRand = (srand((unsigned)time(NULL)),0);
double gSystemTime;        //ϵͳʱ��
uint8  gBatch;
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/********************************************************************
*  @name     : CAlgInterface::CAlgInterface    
*  @brief    : init alg
*  @return   : 
********************************************************************/ 
CAlgInterface::CAlgInterface()
{                            
  gSystemTime = 0.0;
  m_outReqVec.reserve(MAX_OUT_REQUEST);     //Ϊvector����Ԥ���ռ�
  m_passengerVec.reserve(MAX_PSG_FLOW_NUM);
  m_eventVec.reserve(MAX_PSG_FLOW_NUM*2);
  m_elevatorVec.reserve(MAX_ELEVATOR_NUM);
  m_AlgFile.InitTools();
  m_eFlowType = RAND_FLOW;
  //m_eFlowType = UP_FLOW;
} 

/********************************************************************
*  @name     : CAlgInterface::~CAlgInterface    
*  @brief    : 
*  @return   : 
********************************************************************/ 
CAlgInterface::~CAlgInterface()
{
  m_AlgFile.CloseTools(1);  //�ر��ļ���
}
 
/********************************************************************
*  @name     : CAlgInterface::generatePsgFlow    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CAlgInterface::generatePsgFlow()
{
  uint16 psg_num = MAX_PSG_FLOW_NUM - (int)floor(50 * rand()/(double)(RAND_MAX+1));
  psg_num = 10;

  sPassengerInfo psg;

  if ( m_eFlowType == UP_FLOW )       //�ϸ߷�
  {
    for (uint16 i=0; i<psg_num; i++)
    {
      psg.m_iPsgID = i;
      psg.m_iReqCurFlr = 1;
      psg.m_dWaitTime = 0.0;
      psg.m_dAllTime = 0.0;
      psg.m_ePsgState = PSG_NONE;
      psg.m_iCurPlace = PSG_ARRIVE_PLACE;
      //Ŀ��¥����2~MAX
      psg.m_iDestFlr = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+2;
      psg.m_iDestFlr = psg.m_iDestFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iDestFlr;
      psg.m_eReqDir = (psg.m_iReqCurFlr > psg.m_iDestFlr) ? DIR_DOWN : DIR_UP;
      //����ʱ����Ҫ����һ������
      psg.m_dReqTime = generateRandTime();

      insertElement( m_passengerVec, psg );
    }
  }
  else if ( m_eFlowType == DOWN_FLOW )  //�¸߷�
  {
    for (uint16 i=0; i<psg_num; i++)
    {
      psg.m_iPsgID = i;
      psg.m_iDestFlr = 1;
      psg.m_dWaitTime = 0.0;
      psg.m_dAllTime = 0.0;
      psg.m_ePsgState = PSG_NONE;
      psg.m_iCurPlace = PSG_ARRIVE_PLACE;
      //����¥����2~MAX
      psg.m_iDestFlr = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+2;
      psg.m_iDestFlr = psg.m_iDestFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iDestFlr;
      psg.m_eReqDir = (psg.m_iReqCurFlr > psg.m_iDestFlr) ? DIR_DOWN : DIR_UP;
      //����ʱ����Ҫ����һ������
      psg.m_dReqTime = generateRandTime();

      insertElement( m_passengerVec, psg );
    }
  }
  else      //�����ͨ��
  {
    for (uint16 i=0; i<psg_num; i++)
    {
      psg.m_iPsgID = i;
      psg.m_dWaitTime = 0.0;
      psg.m_dAllTime = 0.0;
      psg.m_ePsgState = PSG_NONE;
      psg.m_iCurPlace = PSG_ARRIVE_PLACE;
      //����¥���Ŀ��¥����1~MAX
      psg.m_iReqCurFlr = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+1;
      psg.m_iReqCurFlr = psg.m_iReqCurFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iReqCurFlr;

      do 
      {
        psg.m_iDestFlr = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+1;
        psg.m_iDestFlr = psg.m_iDestFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iDestFlr;
      } while ( psg.m_iDestFlr == psg.m_iReqCurFlr );

      psg.m_eReqDir = (psg.m_iReqCurFlr > psg.m_iDestFlr) ? DIR_DOWN : DIR_UP;
      
      //����ʱ����Ҫ����һ������
      psg.m_dReqTime = generateRandTime();

      insertElement( m_passengerVec, psg );
    }
  }
  sortElement( m_passengerVec, psg); //��������ʱ������


  //////////////////////////////////////////////////////////////////////////
  //���Խű�
  sPassengerIterator psgIterEnd = m_passengerVec.end();
  for( sPassengerIterator  i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
  {
    fprintf(m_AlgFile.m_OutputFilePtr, "generatePsgFlow:Psg(%2d)-ReqFlr(%2d)-DestFlr(%2d)-ReqTime(%f)\n",i->m_iPsgID,i->m_iReqCurFlr,i->m_iDestFlr,i->m_dReqTime);	
  }
}

/********************************************************************
*  @name     : CAlgInterface::generateRandTime    
*  @brief    : 
*  @return   : float
********************************************************************/ 
double CAlgInterface::generateRandTime()
{
  //TODO:���ɷֲ���������ʱ��
  return (3 * rand()/(double)(RAND_MAX+1));
}

 /********************************************************************
 *  @name     : CAlgInterface::isAlgFinished    
 *  @brief    : Have all passengers arrived at dest floor? 
 *  @return   : bool
 ********************************************************************/ 
bool CAlgInterface::isAlgFinished()
{
  sPassengerIterator psgIterEnd = m_passengerVec.end();
  for( sPassengerIterator  i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
  {
    if (i->m_ePsgState != PSG_ARRIVE)
      return false;
  }
  return true;
}

/********************************************************************
*  @name     : CAlgInterface::generateElevatorVec    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CAlgInterface::generateElevatorVec()
{
  CElevator elvt;

  for (uint16 i=0; i<MAX_ELEVATOR_NUM; i++)
  {
    elvt.initELevator(i, m_AlgFile);
    insertElement( m_elevatorVec, elvt );
  }
}
/*********************************************************************
*********************************************************************/
