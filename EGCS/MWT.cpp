/******************************************************************** 
�޸�ʱ��:        2016/04/06 21:45
�ļ�����:        MWT.cpp
�ļ�����:        huming 
=====================================================================
����˵��:        ��С�ȴ�ʱ���㷨 
--------------------------------------------------------------------- 
�汾���:        
--------------------------------------------------------------------- 
����˵��:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "MWT.h"
#include "template_file.h"


int psgCnt = 0;

/********************************************************************
*  @name     : CMWT::CMWT    
*  @brief    : 
*  @return   : 
********************************************************************/ 
CMWT::CMWT()
{ 
}

/********************************************************************
*  @name     : CMWT::Core_Main    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CMWT::Core_Main()
{
  generateElevatorVec();      //��������Ⱥ
  generatePsgFlow();          //�����˿ͽ�ͨ��
  //testPsgFlow();

  while ( !isAlgFinished() )  //�������Ƿ��Ѿ�����Ŀ�Ĳ�        
  {
    gSystemTime += SYSTEM_TIME_STEP;

    LOGE("\n++++++++++++++++Core_Main:+++%.2f++++++++++++++++\n",gSystemTime);

    //���µ���״̬
    CElevatorIterator elvtIterEnd = m_elevatorVec.end();
    for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
      i->updateRunInfo();

    //�����ⲿ����
    processOuterReqFlow();    //�����ⲿ����
    schedule();               //���ⲿ������ɵ���

    //���ݿ���
    for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
      i->Elevator_Main(m_outReqVec, m_passengerVec);

    //////////////////////////////////////////////////////////////////////////
    //�쳣�˳�
    if ( gSystemTime > 200)
    {
      m_AlgFile.CloseTools(1);
      LOGE("Run fail!\n");
      puts("Run fail!\n");
      exit(1);
    }
  }
  LOGE("Run successfully!\n");
  puts("Run successfully!\n");
}

/********************************************************************
*  @name     : CMWT::schedule    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CMWT::schedule()
{
  CElevatorIterator elvtIter;

  sOutRequestIterator reqIterEnd = m_outReqVec.end();
  for( sOutRequestIterator i=m_outReqVec.begin(); i != reqIterEnd;  ++i )
  {
    elvtIter = fitness(i);
    dispatch(i,elvtIter);       //����ÿһ������
    //i->m_iReqElvtID = elvtIter->m_iElvtID;
  }
  
  if ( m_outReqVec.size() > 0 )   //����������������������б�
    m_outReqVec.clear(); 
}
 
/********************************************************************
*  @name     : CMWT::fitness    
*  @brief    : 
*  @param    : reqIter 
*  @return   : void
********************************************************************/ 
CElevatorIterator CMWT::fitness(sOutRequestIterator& reqIter)
{
  sTargetVal tarVal;
  sTargetVal MintargetVal = {MAX_WAIT_TIME,MAX_ENERGY};
  CElevatorIterator bestElvtIter;
  CElevatorIterator elvtIterEnd = m_elevatorVec.end();

  /**
  * 1.�жϵ�ǰ¥���Ƿ��е����ſ���
  * 2.��ǰ¥���Ƿ��е���ͣ��
  * 3.���֮ǰ�г˿ͱ����䣬���ɷ���˿͸�֮ǰ����
  */
  for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
  {
    //������ݾ����������ڲ��ҵ��ݴ��ڴ���������δ��ʱ��֮�䷵�ص��ݺ�
    if ( i->m_iCurFlr == reqIter->m_iReqCurFlr && ELVT_STOP(i->m_eCurState) && i->m_iCurPsgNum < MAX_INNER_PSG_NUM )
    {
      bestElvtIter = i;
      LOGE( "dispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
      return bestElvtIter;
    }
  }

  //�����ǰ��û�е���ͣ�����������ʱ�����е���
  for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
  {  
    tarVal = i->trytoDispatch(reqIter);
    if ( tarVal.m_fWaitTime != 0 )
    {
      if ( tarVal.m_fWaitTime < MintargetVal.m_fWaitTime )
      {
        bestElvtIter = i;
        MintargetVal = tarVal;
      }
    }
    else
    {
      bestElvtIter = i;
      break;
    }

  }

  LOGE("\ndispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
  return bestElvtIter;
}

/********************************************************************
*  @name     : CMWT::dispatch    
*  @brief    : 
*  @param    : reqIter 
*  @return   : void
********************************************************************/ 
void CMWT::dispatch(sOutRequestIterator& reqIter, CElevatorIterator& elvtIter)
{
  if ( !elvtIter->m_isSchedule )
  {
    elvtIter->m_isSchedule = true;
    LOGE("dispatch: LastSysTime(%.2f)->(%.2f)-Schedule(true)\n",elvtIter->m_dLastSysTime,gSystemTime);
    elvtIter->m_dLastSysTime = gSystemTime;
  }
  
  elvtIter->acceptRunTableItem(reqIter);
}

/********************************************************************
*  @name     : CMWT::onClickOutBtn    
*  @brief    : generate a new out request
*  @return   : void
********************************************************************/ 
void CMWT::onClickOutBtn(sPassengerIterator& psg)
{
  sOutRequest out_req;
  sOutRequestIterator reqIter;
 
  //���ݳ˿��ⲿ������������ʵ��
  out_req.m_iPassagerID = psg->m_iPsgID;
  out_req.m_iReqCurFlr  = psg->m_iPsgCurFlr;
  out_req.m_iReqDestFlr = psg->m_iPsgDestFlr;
  out_req.m_dReqTime    = psg->m_dPsgReqTime;
  out_req.m_eReqDir     = (psg->m_iPsgDestFlr > psg->m_iPsgCurFlr) ? DIR_UP : DIR_DOWN; 

  LOGE("onClickOutBtn:Psg(%2d)-[ReqCurFlr(%2d)-ReqDestFlr(%2d)-ReqDir(%d)]\n",psg->m_iPsgID,psg->m_iPsgCurFlr,psg->m_iPsgDestFlr,out_req.m_eReqDir);	
  
  if ( queryElement( m_outReqVec,out_req,reqIter ) != m_outReqVec.end() )
    return;
  else
    insertElement( m_outReqVec, out_req );

  sortElement( m_outReqVec, out_req );     //��������¥���������
}

/********************************************************************
*  @name     : CMWT::processPsgFlow    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CMWT::processOuterReqFlow()
{
  psgCnt = 0;
  sPassengerIterator psgIterEnd = m_passengerVec.end();
  for( sPassengerIterator i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
  {
    //�˿͵�����ʱ����ϵͳʱ��Ƚ�
    if ( i->m_dPsgReqTime < gSystemTime )
    {
      if ( i->m_ePsgState == PSG_NONE )
      {
        onClickOutBtn(i);
        i->m_ePsgState = PSG_WAIT;
        psgCnt++;
      } 
    }
    else
      break;
  }
}
