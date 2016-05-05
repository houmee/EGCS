/******************************************************************** 
�޸�ʱ��:        2016/04/06 21:45
�ļ�����:        MPE.cpp
�ļ�����:        huming 
=====================================================================
����˵��:        ��С�ܺ��㷨 
--------------------------------------------------------------------- 
�汾���:        
--------------------------------------------------------------------- 
����˵��:         
*********************************************************************/

/*********************************************************************
* INCLUDES
*/
#include "MPE.h"
#include "template_file.h"

/********************************************************************
*  @name     : CMPE::CMPE    
*  @brief    : 
*  @return   : 
********************************************************************/ 
CMPE::CMPE()
{ 
}

/********************************************************************
*  @name     : CMWT::Core_Main    
*  @brief    : 
*  @return   : void
********************************************************************/ 
bool CMPE::Core_Main()
{
  generateElevatorVec();      //��������Ⱥ
  generatePsgFlow();          //�����˿ͽ�ͨ��

  while ( !isAlgFinished() )  //�������Ƿ��Ѿ�����Ŀ�Ĳ�        
  {
    gSystemTime += SYSTEM_TIME_STEP;

    LOGE("\n++++++++++++++++Core_Main:+++%.2f++++++++++++++++\n",gSystemTime);

    //////////////////////////////////////////////////////////////////////////
    //���Բ���

    //////////////////////////////////////////////////////////////////////////
    //�����˵���Ŀ�ĵ�
    int myCnt =0;
    sPassengerIterator psgIterEnd = m_passengerVec.end();
    for( sPassengerIterator  i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
    {
      if ( i->m_ePsgState == PSG_ARRIVE )
        myCnt++;
      else if ( i->m_ePsgState == PSG_TRAVEL)
      {
        //LOGE("Psg(%3d) is traveling at Place(%d)\n",i->m_iPsgID,i->m_iCurPlace);
      }
      else
      {
        //LOGE("Psg(%3d) is waiting  at Place(%d)\n",i->m_iPsgID,i->m_iCurPlace);
      }
    }
    LOGE("Total (%3d)Psg have arrived!\n",myCnt);

    //////////////////////////////////////////////////////////////////////////
    //���ٵȴ�ʱ�䳬��һ��ʱ��
    for( sPassengerIterator i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
    {
      if ( (gSystemTime-i->m_dPsgReqTime > 50) && i->m_ePsgState == PSG_WAIT )
        LOGE("Psg(%3d) waits for (%.2f) seconds!\n",i->m_iPsgID,gSystemTime-i->m_dPsgReqTime);

      if ( (gSystemTime- i->m_dPsgReqTime- i->m_dWaitTime > 50) && i->m_ePsgState == PSG_TRAVEL )
        LOGE("Psg(%3d) travels for (%.2f) seconds!\n",i->m_iPsgID,gSystemTime-i->m_dPsgReqTime);

      if ( i->m_iCurPlace == 0x5A )   //����˿ͱ����Ϊ"�ٴ�����"
      {
        i->m_iCurPlace = 0x11;
        LOGE("Psg(%3d)-ReqTime(%.2f)->(%.2f)\n",i->m_iPsgID,i->m_dPsgReqTime,gSystemTime + 5);
        i->m_dPsgReqTime = gSystemTime + 5;   //�޸�����ʱ��ʵ���ٴ�����
      }
    }

    //////////////////////////////////////////////////////////////////////////
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
    if ( gSystemTime > 1000)
    {
      m_AlgFile.CloseTools(1);
      LOGE("Run fail!\n");
      puts("Run fail!\n");
      return false;
    }
  }
  LOGE("Run successfully!\n");
  puts("Run successfully!\n");
  return true;
}

/********************************************************************
*  @name     : CMWT::schedule    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CMPE::schedule()
{
  CElevatorIterator elvtIter;

  sOutRequestIterator reqIterEnd = m_outReqVec.end();
  for( sOutRequestIterator i=m_outReqVec.begin(); i != reqIterEnd;  ++i )
  {
    elvtIter = fitness(i);
    if ( elvtIter != m_elevatorVec.end() )  //�������ֵ��Ϊend
      dispatch(i,elvtIter);                 //����ÿһ������
  }

  if ( m_outReqVec.size() > 0 )             //����������������������б�
    m_outReqVec.clear(); 
}

/********************************************************************
*  @name     : CMWT::fitness    
*  @brief    : 
*  @param    : reqIter 
*  @return   : void
********************************************************************/ 
CElevatorIterator CMPE::fitness(sOutRequestIterator& reqIter)
{
  sTargetVal tarVal;
  sTargetVal MintargetVal = {MAX_WAIT_TIME,MAX_ENERGY};
  CElevatorIterator bestElvtIter = m_elevatorVec.begin();
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
      LOGA( "dispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
      return bestElvtIter;
    }
  }

  //�����ǰ��û�е���ͣ�����������ʱ�����е���
  uint8 cnt = 0;
  for(CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
  {  
    if ( i->m_iCurPsgNum < MAX_INNER_PSG_NUM )                //ֻ�е���δ�����ܵ���
    {
      tarVal = i->trytoDispatch(reqIter, m_passengerVec, m_elevatorVec);
      if ( tarVal.m_fEnergy != 0 )
      {
        if ( tarVal.m_fEnergy < MintargetVal.m_fEnergy )
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
    else      //���������������
      cnt++;  //��¼�Ƿ����е��ݶ�δ��Ӧ�˴�����
  }

  if ( cnt == m_elevatorVec.size() )        //������е��ݶ�ô����Ӧ�˴�����
  {
    uint16 psgid = reqIter->m_iPassagerID;  //��¼����˿���Ϣ
    sPassengerIterator psgIterEnd = m_passengerVec.end();
    for( sPassengerIterator  j=m_passengerVec.begin(); j != psgIterEnd;  ++j )
    {
      if (j->m_iPsgID == psgid )
      {
        j->m_iCurPlace = 0x5A;              //���˿ͱ��Ϊ�����ٴ�����
        j->m_ePsgState = PSG_NONE;
      }
    }
    LOGA("dispatch:Elvts are all full,Psg(%3d) is refused!\n",psgid);

    bestElvtIter = m_elevatorVec.end();     //����ֵ����Ϊend
    LOGA("dispatch:outReq[Psg(%3d)-ReqCurFlr(%2d)-ReqDestFlr(%2d)-ReqTime(%.2f)-ReqDir(%d)] is refused!\n",
      reqIter->m_iPassagerID,reqIter->m_iReqCurFlr,reqIter->m_iReqDestFlr,reqIter->m_dReqTime,reqIter->m_eReqDir);
  }
  else
    LOGA("dispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
  return bestElvtIter;
}

/********************************************************************
*  @name     : CMWT::dispatch    
*  @brief    : 
*  @param    : reqIter 
*  @return   : void
********************************************************************/ 
void CMPE::dispatch(sOutRequestIterator& reqIter, CElevatorIterator& elvtIter)
{
  if ( !elvtIter->m_isSchedule )
  {
    elvtIter->m_isSchedule = true;
    LOGA("dispatch: LastSysTime(%.2f)->(%.2f)-Schedule(true)\n",elvtIter->m_dLastSysTime,gSystemTime);
    elvtIter->m_dLastSysTime = gSystemTime;
  }

  elvtIter->acceptRunTableItem(reqIter);
}

/********************************************************************
*  @name     : CMWT::onClickOutBtn    
*  @brief    : generate a new out request
*  @return   : void
********************************************************************/ 
void CMPE::onClickOutBtn(sPassengerIterator& psg)
{
  sOutRequest out_req;
  sOutRequestIterator reqIter;

  //���ݳ˿��ⲿ������������ʵ��
  out_req.m_iPassagerID = psg->m_iPsgID;
  out_req.m_iReqCurFlr  = psg->m_iPsgCurFlr;
  out_req.m_iReqDestFlr = psg->m_iPsgDestFlr;
  out_req.m_dReqTime    = psg->m_dPsgReqTime;
  out_req.m_eReqDir     = (psg->m_iPsgDestFlr > psg->m_iPsgCurFlr) ? DIR_UP : DIR_DOWN; 

  LOGA("onClickOutBtn:Psg(%3d)-[ReqCurFlr(%2d)-ReqDestFlr(%2d)-ReqDir(%d)]\n",psg->m_iPsgID,psg->m_iPsgCurFlr,psg->m_iPsgDestFlr,out_req.m_eReqDir);	

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
void CMPE::processOuterReqFlow()
{
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
        i->m_iCurPlace = PSG_ARRIVE_PLACE;
      } 
    }
  }
}
