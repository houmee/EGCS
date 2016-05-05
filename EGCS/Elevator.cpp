/******************************************************************** 
�޸�ʱ��:        2016/03/20 16:39
�ļ�����:        Elevator.cpp
�ļ�����:        huming 
=====================================================================
����˵��:        ����ģ����� 
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "Elevator.h"
#include "template_file.h"

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

CElevator::CElevator(int id, CTools& tools)
{
  m_iElvtID       = id;
  m_iCurFlr       = 1;
  m_iNextStopFlr  = 1;
  m_dCurRunDis    = 0.0;
  m_iCurPsgNum    = 0;
  m_dLastSysTime  = 0.0;
  m_dNextStateTime = 0.0;
  m_dLastStateTime = 0.0;
  m_eCurState     = IDLE;
  m_eRundir       = DIR_NONE;
  m_isSchedule    = false;
  m_canHandle     = false;
  m_isTrytoDispatch = false;
  m_AlgFile        = tools;
  m_sRunTable.reserve(MAX_FLOOR_NUM);

  m_lastRunItem.m_eRunDir              = DIR_NONE;
  m_lastRunItem.m_eReqType             = IN_REQ;
  m_lastRunItem.m_iDestFlr             = 1;
  m_lastRunItem.m_iPriority            = 0x7F;
  m_lastRunItem.m_sTarVal.m_fEnergy    = 0;
  m_lastRunItem.m_sTarVal.m_fWaitTime  = 0;
}

/********************************************************************
*  @name     : CElevator::Elevator_Main    
*  @brief    : 
*  @param    : reqIter 
*  @param    : psg 
*  @param    : tools 
*  @return   : void
********************************************************************/ 
void CElevator::Elevator_Main(sOutRequestVec& reqVec, sPassengerInfoVec& psgVec)
{
  LOGE("\n--Elvt(%d) Main--\n", m_iElvtID);
  LOGE("updateRunInfo:CurFlr(%2d)-NextFlr(%d)-RunDis(%.2f)-CurState(%d)-CurDir(%d)-CurVol(%2d)-LastStateTime(%.2f)-NextStateTime(%.2f)\n",m_iCurFlr,m_iNextStopFlr,m_dCurRunDis, m_eCurState,m_eRundir,m_iCurPsgNum,m_dLastStateTime,m_dNextStateTime);	

  //////////////////////////////////////////////////////////////////////////
  //����������Ҫ����
  if ( !m_isSchedule )
  {
    m_isSchedule = true;
    LOGA("Elevator_Main: LastSysTime(%.2f)->(%.2f)-Schedule(true)\n",m_dLastSysTime,gSystemTime);
    m_dLastSysTime = gSystemTime;
  }

  if ( gSystemTime > m_dNextStateTime )
    gotoNextDest();

  if ( ELVT_STOP(m_eCurState) )   //���ݴ��ڴ�����ͣ��״̬������˿�
    processReqPsgFlow(psgVec);


  LOGE("Elevator_Main\n");
  showElevator();
  LOGE("updateRunInfo:CurFlr(%2d)-NextFlr(%d)-RunDis(%.2f)-CurState(%d)-CurDir(%d)-CurVol(%2d)-LastStateTime(%.2f)-NextStateTime(%.2f)\n",m_iCurFlr,m_iNextStopFlr,m_dCurRunDis, m_eCurState,m_eRundir,m_iCurPsgNum,m_dLastStateTime,m_dNextStateTime);	
}

/********************************************************************
*  @name     : CElevator::updateRunInfo    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::updateRunInfo()
{
  uint16 runlen = 0;
  double runTime = 0, remainTime = 0;

  LOGE("updateRunInfo:Elvt(%d)-CurState(%d)-SystemTime(%.2f)-LastStateTime(%.2f)-NextStateTime(%.2f)\n",m_iElvtID,m_eCurState,gSystemTime,m_dLastStateTime,m_dNextStateTime);
  //////////////////////////////////////////////////////////////////////////
  //��ֹͣ���˶��仯
  //״̬�仯��1.IDLE->IDLE         2.PAUSE->PAUSE     3.IDLE->ACC           4.PAUSE->ACC
  //����仯: 1.DIR_NONE->DIR_NONE 2.DIR_NONE->DIR_UP 3.DIR_NONE->DIR_DOWN
  if ( ELVT_STOP(m_eCurState) ) //������ֹͣ״̬  
  {
    if ( gSystemTime > m_dLastStateTime )    //�����˵��ݿ�����ʱ��
    { 
      if ( m_iNextStopFlr != m_iCurFlr )      //�����һͣ��¥�㲻�ǵ�ǰ¥�㣬����״̬�ͷ���
      {
        m_eCurState = m_iNextStopFlr > m_iCurFlr ? UP_ACC : DOWN_ACC;
        m_eRundir   = m_iNextStopFlr > m_iCurFlr ? DIR_UP : DIR_DOWN;
      }
      else                                    //�����һͣ��¥���ǵ�ǰ¥�㣬���ı�״̬�ͷ���
      {
        if ( m_sRunTable.size() == 0 )        //���û��������״̬��ΪIDLE���϶��˴�����û�г˿�
        {
          assert(m_iCurPsgNum == 0);
          m_eCurState = IDLE;

          m_dLastStateTime = gSystemTime;     //û�������ʱ��ʱ��ˢ��
          m_dNextStateTime = gSystemTime;
          m_dLastSysTime   = gSystemTime;
        }
      }
    }
    else
    {
      if ( m_lastRunItem.m_iDestFlr > m_iCurFlr )
        m_eCurState = DOWN_PAUSE;
      else if ( m_lastRunItem.m_iDestFlr < m_iCurFlr )
        m_eCurState = UP_PAUSE;
      else
      {
        m_eCurState = IDLE;
        m_eRundir   = DIR_NONE;
      }
    }
  }

  if ( !ELVT_STOP(m_eCurState) ) //���������������
  { 
    assert( gSystemTime >= m_dLastStateTime );

    runTime    = (gSystemTime >= m_dLastStateTime) ? (gSystemTime-(double)m_dLastStateTime) : 0 ;
    remainTime = (gSystemTime <= m_dNextStateTime) ? ((double)m_dNextStateTime-gSystemTime) : 0 ;

    if ( runTime <= ACCELERATE_TIME )
    {
      if ( m_eRundir == DIR_UP )
        m_eCurState = UP_ACC;
      else if ( m_eRundir == DIR_DOWN )
        m_eCurState = DOWN_ACC;
    }
    else if ( remainTime <= DECELERATE_TIME )
    {
      if ( m_eRundir == DIR_UP )
        m_eCurState = UP_DEC;
      else if ( m_eRundir == DIR_DOWN )
        m_eCurState = DOWN_DEC;
    }
    else
    {
      if ( m_eRundir == DIR_UP )
        m_eCurState = UP_CONST;
      else if ( m_eRundir == DIR_DOWN )
        m_eCurState = DOWN_CONST;
    }

    if ( m_eCurState == UP_ACC || m_eCurState == DOWN_ACC )           //���ϣ����£�����״̬
      m_dCurRunDis = (double)runTime/ACCELERATE_TIME*ACCELERATE_LENGTH;
    else if( m_eCurState == UP_CONST || m_eCurState == DOWN_CONST )   //����״̬
      m_dCurRunDis = (double)( runTime - ACCELERATE_TIME )*RUN_SPEED + ACCELERATE_LENGTH;
    else if ( m_eCurState == UP_DEC || m_eCurState == DOWN_DEC )      //���ϣ����£�����״̬
      m_dCurRunDis = abs(m_lastRunItem.m_iDestFlr - m_iNextStopFlr)*FLOOR_HEIGHT-(double)remainTime/DECELERATE_TIME*DECELERATE_LENGTH;

    //////////////////////////////////////////////////////////////////////////
    //������һͣ��¥�㡢���о�������з���ȷ����������¥��
    if ( ELVT_UP(m_eCurState) )
      m_iCurFlr = (uint8)(m_dCurRunDis / FLOOR_HEIGHT) + m_lastRunItem.m_iDestFlr;
    else if ( ELVT_DOWN(m_eCurState) )
      m_iCurFlr = m_lastRunItem.m_iDestFlr - (uint8)(m_dCurRunDis / FLOOR_HEIGHT);

    if ( m_iCurFlr > MAX_FLOOR_NUM )
      m_iCurFlr = MAX_FLOOR_NUM;
    if ( m_iCurFlr < 1 )
      m_iCurFlr = 1;

    //////////////////////////////////////////////////////////////////////////
    //���˶���ֹͣ�仯
    if ( gSystemTime >= m_dNextStateTime )
    {
      assert( m_iCurFlr == m_iNextStopFlr );

      if ( m_eRundir == DIR_UP )
      {
        assert( m_eCurState == UP_DEC );
        m_eCurState = UP_PAUSE;
      }
      else if ( m_eRundir == DIR_DOWN )
      {
        assert( m_eCurState == DOWN_DEC );
        m_eCurState = DOWN_PAUSE;
      }
    }
  }

  if ( gSystemTime >= m_dNextStateTime )
  {
    m_isSchedule = false;
    LOGE("updateRunInfo: Elvt(%d)-Schedule is false.\n", m_iElvtID);
  }
}

/********************************************************************
*  @name     : CElevator::gotoNextDest    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::gotoNextDest()
{
  sRunItemIterator tarIter;
  sRunItem tmpItem;
  double tmptime;

  //������һͣ����״̬
  if ( m_sRunTable.size() > 0 && m_iCurFlr == m_iNextStopFlr )
  {
    //���浱ǰͣ��¥����Ϣ
    tmpItem = m_lastRunItem;
    tmptime = m_dLastSysTime;
    m_lastRunItem = m_sRunTable.at(0);
    m_dCurRunDis  = 0;
    LOGA("gotoNextDest:Save LastItem-Elvt(%d)-NextStopFlr(%2d)->(%2d)\n",m_iElvtID,tmpItem.m_iDestFlr, m_lastRunItem.m_iDestFlr);	
    
    //ɾ����ǰͣ����
    tarIter = queryElement( m_sRunTable,m_lastRunItem,tarIter );  
    LOGA("gotoNextDest:Delete LastItem-Elvt(%d)-ReqType(%d)-DestFlr(%d)\n", m_iElvtID,tarIter->m_eReqType, tarIter->m_iDestFlr);	
    deleteRunTableItem( m_lastRunItem );

    LOGA("gotoNextDest:Elvt(%d)-NextStopFlr(%2d)-Rundir(%d)-NextStateTime(%.2f)\n",m_iElvtID,m_iNextStopFlr,m_eCurState,m_dNextStateTime);	
    //showElevator();  
  }
}


/********************************************************************
*  @name     : CElevator::getElevatorInfo    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::changeNextStop()
{
  int    tmpFlr;
  double tmpTime;
 
  if ( !m_isTrytoDispatch )         //ֻ���ڷ�TrytoDispatch����ʱ����Ƿ������һͣ��¥����Ϣ
  {
    if ( m_sRunTable.size() > 0 )   //���������һͣ��¥������
    {
      tmpFlr = m_iNextStopFlr;
      tmpTime = m_dNextStateTime;

      if ( m_sRunTable.at(0).m_iDestFlr != m_iNextStopFlr )   //�����һͣ��¥�㲻һ��
      {
        m_iNextStopFlr   = m_sRunTable.at(0).m_iDestFlr;
        m_dLastStateTime = m_dLastSysTime;
        m_dNextStateTime = m_sRunTable.at(0).m_sTarVal.m_fWaitTime + m_dLastStateTime;
        
        if ( ELVT_STOP(m_eCurState) )
        {
          if ( m_eRundir == DIR_UP )
          {
            if ( m_sRunTable.at(0).m_iDestFlr < m_iCurFlr )
            {
              m_eRundir   = DIR_NONE;
              m_eCurState = IDLE;
              
              LOGA("changeNextStop:Elvt(%d) changes dir-DestFlr(%2d)->CurFlr(%2d)-Rundir(%d)-CurState(%d)\n",
                m_iElvtID,m_sRunTable.at(0).m_iDestFlr,m_iCurFlr,m_eRundir,m_eCurState);
            }
          }
          else if ( m_eRundir == DIR_DOWN )
          {          
            if ( m_sRunTable.at(0).m_iDestFlr > m_iCurFlr )
            {
              m_eRundir   = DIR_NONE;
              m_eCurState = IDLE;

              LOGA("changeNextStop:Elvt(%d) changes dir-DestFlr(%2d)->CurFlr(%2d)-Rundir(%d)-CurState(%d)\n",
                m_iElvtID,m_sRunTable.at(0).m_iDestFlr,m_iCurFlr,m_eRundir,m_eCurState);
            }
          }
          
          LOGA("changeNextStop:Elvt(%d) changes dir-DestFlr(%2d)->CurFlr(%2d)-Rundir(%d)-CurState(%d)\n",
                m_iElvtID,m_sRunTable.at(0).m_iDestFlr,m_iCurFlr,m_eRundir,m_eCurState);
        }
 

        LOGA("changeNextStop:Elvt(%d)-NextStopFlr(%2d)->(%2d)--NextStateTime(%.2f)->(%.2f)\n",m_iElvtID,tmpFlr,m_iNextStopFlr,tmpTime,m_dNextStateTime);
        LOGA("changeNextStop:Elvt(%d)-LastStateTime(%.2f)->(%.2f)\n",m_iElvtID,m_dLastStateTime, m_dLastStateTime);	
      }
      else  
      {
        if ( m_iNextStopFlr == m_iCurFlr )
        {
          if ( m_dLastStateTime != m_dLastSysTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME )
          {
            LOGA("changeNextStop:Elvt(%d)-LastStateTime(%.2f)->(%.2f)\n",m_iElvtID,m_dLastStateTime, m_dLastSysTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME);	
            m_dLastStateTime = m_dLastSysTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME;
          }
        }
        
        if( m_dNextStateTime != m_sRunTable.at(0).m_sTarVal.m_fWaitTime + m_dLastStateTime ) //���ͣ��¥��һ�µ�ʱ�䲻ͬ
        {
          m_dNextStateTime = m_sRunTable.at(0).m_sTarVal.m_fWaitTime + m_dLastStateTime;
          LOGA("changeNextStop:Elvt(%d)-NextStateTime(%.2f)->(%.2f)\n",m_iElvtID,tmpTime,m_dNextStateTime);
        }
      }
      
    }
    else    //û������������ͣ�������һ�������¥��
    {
      m_eRundir         = DIR_NONE;
      m_eCurState       = IDLE;
      m_iNextStopFlr    = m_iCurFlr;
      m_dNextStateTime  = gSystemTime;
    }
  }
}

/********************************************************************
*  @name     : CElevator::trytoDispatch    
*  @brief    : 
*  @param    : reqIter 
*  @return   : void
********************************************************************/ 
sTargetVal CElevator::trytoDispatch( sOutRequestIterator reqIter, sPassengerInfoVec& psgVec, CElevatorVec& elvtVec )
{
  sRunItem reqRunItem;
  sRunItemIterator tarIter;
  sTargetVal tarVal = {0,0}, tmpTarVal={0,0};
  sTargetVal preTarVal ={0,0}, NextTarVal ={0,0};

  //���ⲿ�����װΪ���б����ȡ�ӵ�ǰλ�õ������Ŀ��ֵ
  reqRunItem.m_eRunDir  = reqIter->m_eReqDir;
  reqRunItem.m_eReqType = OUT_REQ;
  reqRunItem.m_iDestFlr = reqIter->m_iReqCurFlr;

  m_isTrytoDispatch = true;
  //////////////////////////////////////////////////////////////////////////
  //������Ĵ���ֵ
  tarIter = queryElement( m_sRunTable,reqRunItem,tarIter );
  if ( tarIter == m_sRunTable.end() )
  {
    //////////////////////////////////////////////////////////////////////////
    //������ǰ�ܺ�(���������ӵ��µ������ܺĺ���ͣ�ܺ�)
    sRunItemIterator end = m_sRunTable.end();
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      /************************************************************************
      * 1.�ж������б���һ�����Ŀ��¥��
      * 2.���㵽��ʱ�䣬�ж����������Ƿ��ڴﵽʱ����Ҳ����Ŀ��¥��
      * 3.���û���������ݣ�����ͣ��¥��󣬽����ڳ˿���Ŀ
      * 4.�����ܺ�
      * 5.ѭ���������б���
      ************************************************************************/

      if (i == m_sRunTable.begin() )
      {
        preTarVal.m_fEnergy += i->m_sTarVal.m_fEnergy;
      }
      else
      {
        uint16 psgCnt=0;
        sPassengerIterator psgIterEnd = psgVec.end();
        for( sPassengerIterator j=psgVec.begin(); j != psgIterEnd;  ++j )
        {
          if ( j->m_iPsgCurFlr == i->m_iDestFlr && j->m_ePsgState == PSG_WAIT && (j->m_ePsgReqDir == m_eRundir || m_eCurState == IDLE ) && psgCnt < MAX_INNER_PSG_NUM )
            psgCnt++;
          else if ( j->m_iCurPlace == m_iElvtID && j->m_ePsgState == PSG_TRAVEL && j->m_iPsgDestFlr == i->m_iDestFlr )
            psgCnt--;
        }

        preTarVal.m_fEnergy += START_STOP_ENERGY + GRAVITY_ACCELERATE*( (m_iCurPsgNum + psgCnt)*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*CONST_SPEED_LENGTH;
      }
    }

    //////////////////////////////////////////////////////////////////////////
    //�������б���
    insertRunTableItem( reqRunItem );  //���뵱ǰ���б�ͬʱ���±�ĸ���Ŀ��ֵ
    tarIter = queryElement( m_sRunTable,reqRunItem,tarIter );     //�����������tarIndʧЧ�����²�ѯ
    
    //////////////////////////////////////////////////////////////////////////
    //���ȴ�ʱ��
    end = m_sRunTable.end();
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      if ( tarIter >= i )   //��iter�����ȫ���������ڲ���iter���ӵ�ʱ��
      {
        tmpTarVal.m_fWaitTime += i->m_sTarVal.m_fWaitTime;
        tarVal.m_fWaitTime     = tmpTarVal.m_fWaitTime;
      }
      else                  //��iterǰ��Ĳ����ӵȴ�ʱ�䣬ֻ����iter�Ŀ�����ʱ��
        tarVal.m_fWaitTime += OPEN_CLOSE_TIME + PSG_ENTER_TIME;
    }

    //////////////////////////////////////////////////////////////////////////
    //��������ܺ�(���������ӵ��µ������ܺĺ���ͣ�ܺ�)
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      /************************************************************************
      * 1.�ж������б���һ�����Ŀ��¥��
      * 2.���㵽��ʱ�䣬�ж����������Ƿ��ڴﵽʱ����Ҳ����Ŀ��¥��
      * 3.���û���������ݣ�����ͣ��¥��󣬽����ڳ˿���Ŀ
      * 4.�����ܺ�
      * 5.ѭ���������б���
      ************************************************************************/

      if (i == m_sRunTable.begin() )
      {
        NextTarVal.m_fEnergy += i->m_sTarVal.m_fEnergy;
      }
      else
      {
        uint16 psgCnt=0;
        sPassengerIterator psgIterEnd = psgVec.end();
        for( sPassengerIterator j=psgVec.begin(); j != psgIterEnd;  ++j )
        {
          if ( j->m_iPsgCurFlr == i->m_iDestFlr && j->m_ePsgState == PSG_WAIT && (j->m_ePsgReqDir == m_eRundir || m_eCurState == IDLE ) && psgCnt < MAX_INNER_PSG_NUM )
            psgCnt++;
          else if ( j->m_iCurPlace == m_iElvtID && j->m_ePsgState == PSG_TRAVEL && j->m_iPsgDestFlr == i->m_iDestFlr )
            psgCnt--;
        }
        
        NextTarVal.m_fEnergy += START_STOP_ENERGY + GRAVITY_ACCELERATE*((m_iCurPsgNum + psgCnt)*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*CONST_SPEED_LENGTH;
      }
    }
  }

  tarVal.m_fEnergy = NextTarVal.m_fEnergy - preTarVal.m_fEnergy;

  //////////////////////////////////////////////////////////////////////////
  //ɾ������
  deleteRunTableItem( reqRunItem );

  m_isTrytoDispatch = false;

  return tarVal;     //����Ŀ��ֵ
}


/********************************************************************
*  @name     : CElevator::acceptRunTableItem    
*  @brief    : 
*  @param    : sOutRequestIterator reqIter 
*  @return   : void
********************************************************************/
void CElevator::acceptRunTableItem( sOutRequestIterator reqIter )
{
  sRunItem reqRunItem;
  sRunItemIterator indIter;

  reqRunItem.m_eReqType = OUT_REQ;
  reqRunItem.m_iDestFlr = reqIter->m_iReqCurFlr;
  reqRunItem.m_eRunDir  = reqIter->m_eReqDir;
  
  insertRunTableItem( reqRunItem );
  //showElevator();
}

/********************************************************************
*  @name     : CElevator::onClickInnerBtn    
*  @brief    : 
*  @param    : psg 
*  @return   : void
********************************************************************/ 
void CElevator::onClickInnerBtn(sPassengerIterator& psg)
{
  sRunItem runItem;
  sRunItemIterator indIter;

  runItem.m_eReqType = IN_REQ;
  runItem.m_iDestFlr = psg->m_iPsgDestFlr;
  runItem.m_eRunDir = (psg->m_iPsgDestFlr > m_iCurFlr) ? DIR_UP:DIR_DOWN;
  
  insertRunTableItem( runItem );
  LOGA("onClickInnerBtn:Inner Psg(%3d)-DestFlr(%2d)-ElvDir(%d)-Table(%d)-(%d)\n",psg->m_iPsgID,runItem.m_iDestFlr,runItem.m_eRunDir,m_sRunTable.size(),m_sRunTable.capacity());	
}

/********************************************************************
*  @name     : CElevator::processInnerPsgFlow    
*  @brief    : 
*  @param    : sPassengerInfoVec & psgVec 
*  @return   : void
********************************************************************/ 
void CElevator::processReqPsgFlow( sPassengerInfoVec& psgVec )
{
  sPassengerIterator psgIterEnd = psgVec.end();

  if ( m_iCurPsgNum > 0 )
  {
    for( sPassengerIterator i=psgVec.begin(); i != psgIterEnd;  ++i )
    {
      //����˿ʹ��ڵ�ǰ��������֮ǰ״̬Ϊ������
      if ( i->m_iCurPlace == m_iElvtID && i->m_ePsgState == PSG_TRAVEL )  
      {
        if ( m_iCurFlr == i->m_iPsgDestFlr && ELVT_STOP( m_eCurState ) ) //�����ǰ��Ϊ�˿�Ŀ���
          psgLeave(i);    //�뿪����
      }
    }
  }


  for( sPassengerIterator i=psgVec.begin(); i != psgIterEnd;  ++i )
  {
    //����˿ʹ��ڵȴ����ҵ�ǰ�����Ѿ�����ͣ��
    if ( i->m_ePsgState == PSG_WAIT && i->m_iPsgCurFlr == m_iCurFlr )
    {
      if ( m_iCurPsgNum <  MAX_INNER_PSG_NUM )
      {
        if ( i->m_ePsgReqDir == m_eRundir || m_eCurState == IDLE )
        {
          psgEnter(i);
          onClickInnerBtn(i);
        }
        else if ( i->m_ePsgReqDir != m_eRundir )
        {
          LOGA("processInnerPsgFlow:Inner Req[Psg(%3d)-PsgCurFlr(%2d)-PsgDest(%2d)-Dir(%d)] is denied!\n", i->m_iPsgID, i->m_iPsgCurFlr,i->m_iPsgDestFlr,i->m_ePsgReqDir);
        }
      }
      else if ( m_iCurPsgNum ==  MAX_INNER_PSG_NUM )
      {
        i->m_iCurPlace = 0x5A;
        i->m_ePsgState = PSG_NONE;
        LOGA("processInnerPsgFlow:Elvt is full,Psg(%3d)-CurPlace(%3d)-PsgState(%d)is refused!\n",i->m_iPsgID,i->m_iCurPlace,i->m_ePsgState);
      }
    }
  }
}

/********************************************************************
*  @name     : CElevator::psgLeave    
*  @brief    : 
*  @param    : psg 
*  @return   : void
********************************************************************/ 
void CElevator::psgLeave(sPassengerIterator& psg)
{
  //���³˿�״̬
  m_iCurPsgNum--;                 
  psg->m_ePsgState = PSG_ARRIVE;
  psg->m_dAllTime = gSystemTime - psg->m_dPsgReqTime;
  psg->m_iCurPlace = PSG_ARRIVE_PLACE;
  LOGA("psgLeave:Psg(%3d)-CurPlace(%d) Leaves Elevator(%d)-CurVol(%2d)\n",psg->m_iPsgID,psg->m_iCurPlace,m_iElvtID,m_iCurPsgNum);	
}

/********************************************************************
*  @name     : CElevator::psgEnter    
*  @brief    : 
*  @param    : psg 
*  @return   : void
********************************************************************/ 
void CElevator::psgEnter(sPassengerIterator& psg)
{
  m_iCurPsgNum++;
  psg->m_dWaitTime = gSystemTime - psg->m_dPsgReqTime;
  psg->m_ePsgState = PSG_TRAVEL;
  psg->m_iCurPlace = m_iElvtID;
  LOGA("psgEnter:Psg(%3d)-CurPlace(%d) Enters Elevator(%d)-CurVol(%2d)\n",psg->m_iPsgID,psg->m_iCurPlace,m_iElvtID,m_iCurPsgNum);	
}

/********************************************************************
*  @name     : CElevator::runfromXtoY    
*  @brief    : 
*  @param    : x 
*  @param    : y 
*  @return   : sTargetVal
********************************************************************/ 
sTargetVal CElevator::runfromXtoY(sRunItem x, sRunItem y)
{
  sTargetVal targetVal = {0.0,0.0};
  uint8 intervalFlr = abs(x.m_iDestFlr - y.m_iDestFlr);  //¥����

  if ( intervalFlr > 0 )
  {
    if ( intervalFlr == 1 )
    {
      targetVal.m_fWaitTime = ACCELERATE_TIME + DECELERATE_TIME + CONST_SPEED_TIME;
      targetVal.m_fEnergy   = START_STOP_ENERGY + GRAVITY_ACCELERATE*(m_iCurPsgNum*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*CONST_SPEED_LENGTH;
    }
    else if ( intervalFlr > 1 )
    {
      //�ȴ�ʱ���������ʱ��Ϳ�����ʱ��
      targetVal.m_fWaitTime = (ACCELERATE_TIME + DECELERATE_TIME  +
        (intervalFlr-2)*ONE_FLOOR_TIME + 2*REMAIN_GAP_TIME);
      //�ܺİ����Ӽ����ܺĺ��ؿ��ܺ�
      targetVal.m_fEnergy = (GRAVITY_ACCELERATE*(m_iCurPsgNum*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*
        ((intervalFlr-2)*FLOOR_HEIGHT + 2*REMAIN_GAP_LENGTH) +START_STOP_ENERGY);
    }  
  }
  else
  {
    targetVal.m_fWaitTime = 0;
    targetVal.m_fEnergy   = 0;
  }
  return targetVal;
}

/********************************************************************
*  @name     : CElevator::updateRunTable    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::updateRunTable()
{
  sRunItem preRunInd,nowRunInd;

  sRunItemIterator end = m_sRunTable.end();
  for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )
  {
    if ( i == m_sRunTable.begin() )
    {
      preRunInd = *i;
      i->m_sTarVal = runfromXtoY( m_lastRunItem, preRunInd );
    }
    else
    {
      nowRunInd = *i;
      i->m_sTarVal = runfromXtoY( preRunInd, nowRunInd );
      preRunInd = *i;
    }
  }
}

/********************************************************************
*  @name     : CElevator::insertRunTable    
*  @brief    : 
*  @param    : sRunIndex runInd 
*  @return   : void
********************************************************************/ 
void CElevator::insertRunTableItem( sRunItem runItem )
{
  sRunItemIterator indIter;

  if ( runItem.m_eReqType == IN_REQ )
  {
    if ( m_eCurState == IDLE || m_eCurState == DOWN_PAUSE || m_eCurState == UP_PAUSE ||
       ( runItem.m_eRunDir == DIR_DOWN && ELVT_DOWN(m_eCurState)) ||
       ( runItem.m_eRunDir == DIR_UP   && ELVT_UP(m_eCurState))
       )
    {
      if (queryElement(m_sRunTable,runItem,indIter) != m_sRunTable.end())    //������һ����!���Ƚ�
        return ;
      else
      {
        insertElement( m_sRunTable, runItem );
        updateItemPriority();
        sortElement( m_sRunTable, runItem );
        updateRunTable();
        changeNextStop();
      }
    }
  }
  else
  {
    if ( queryElement(m_sRunTable,runItem,indIter) != m_sRunTable.end() )
      return;
    else
    {
      insertElement( m_sRunTable, runItem );
      updateItemPriority();
      sortElement( m_sRunTable, runItem );
      updateRunTable();
      changeNextStop();
    }
  }
 
}

/********************************************************************
*  @name     : CElevator::deleteRunTableItem    
*  @brief    : 
*  @param    : sRunIndex runInd 
*  @return   : void
********************************************************************/ 
void CElevator::deleteRunTableItem( sRunItem runItem )
{
  sRunItemIterator tarInd;
  deleteElement( m_sRunTable,runItem,tarInd );
  sortElement( m_sRunTable, runItem );
  updateRunTable();
  changeNextStop();
}

/********************************************************************
*  @name     : getTaskPriority    
*  @brief    : calculate priority according to current dir and request dir
*  @return   : void
********************************************************************/ 
void CElevator::updateItemPriority()
{
  uint8 tmpFlr=MAX_FLOOR_NUM;
  RunDir runDir=DIR_NONE;

  if ( m_sRunTable.size() > 0 )   //ֻ�������б���ڱ���ʱ�Ż����
  {
    sRunItemIterator end = m_sRunTable.end();
    //////////////////////////////////////////////////////////////////////////
    //ȷ��Ǳ�ڷ���
    if ( m_eCurState == IDLE )
    {
      assert( m_eRundir == DIR_NONE );      //������ʱ����ݿ϶�û�з���

      for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )
      {
        if ( m_iCurFlr == i->m_iDestFlr )   //�����һ������Ŀ��¥����ǵ�ǰ¥��
        {
          runDir = i->m_eRunDir;
          break;
        }
        else if ( abs(i->m_iDestFlr-m_iCurFlr) < tmpFlr )
        {
          runDir = m_iCurFlr>i->m_iDestFlr ? DIR_DOWN : DIR_UP;
          if ( runDir == i->m_eRunDir )
            tmpFlr = abs(m_iCurFlr-i->m_iDestFlr);
        }
      }
    }
    else
      runDir = m_eRundir;

    //////////////////////////////////////////////////////////////////////////
    //���ݷ����¥�������ȼ�
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )
    {
      if ( runDir == DIR_UP )         //��������б�������
      {
        if ( i->m_iDestFlr == m_iCurFlr && i->m_eRunDir == DIR_UP && (m_eCurState == IDLE || m_eCurState == UP_PAUSE) ) //����������ͣ�������������Ŀ��¥���뵱ǰ¥��һ��ʱ�����ȼ����
            i->m_iPriority = 0;
        else                                //������״̬�����緽�����£������ϼ��١�����ʱ��������ȼ�
        {
          if ( i->m_iDestFlr > m_iCurFlr )  
          {
            if ( i->m_eRunDir == DIR_UP )         //���������ұȵ�ǰ¥���
              i->m_iPriority = i->m_iDestFlr;     //p = i;
            else if ( i->m_eRunDir == DIR_DOWN )  //���������ұȵ�ǰ¥���
              i->m_iPriority = 2*MAX_FLOOR_NUM - i->m_iDestFlr;   //p = 2*MAX-i;
          }
          else
          {
            if ( i->m_eRunDir == DIR_UP )         //�������ϵ��ȵ�ǰ¥���
              i->m_iPriority = 2*MAX_FLOOR_NUM + i->m_iDestFlr;   //p = 2*MAX+i;
            else if( i->m_eRunDir == DIR_DOWN )   //�������µ��ȵ�ǰ¥���
              i->m_iPriority = 2*MAX_FLOOR_NUM - i->m_iDestFlr;   //p = 2*MAX-i;
          }
        }
      }
      else if ( runDir == DIR_DOWN )       //��������б�������
      {

        if ( i->m_iDestFlr == m_iCurFlr && i->m_eRunDir == DIR_DOWN && (m_eCurState == IDLE || m_eCurState == DOWN_PAUSE) ) //����������ͣ�������������Ŀ��¥���뵱ǰ¥��һ��ʱ�����ȼ����
          i->m_iPriority = 0;
        else                          //������״̬�����緽�����ϣ������¼��١�����ʱ��������ȼ�
        {
          if ( i->m_iDestFlr >= m_iCurFlr )
          {
            if ( i->m_eRunDir == DIR_UP )         //���������ұȵ�ǰ¥���
              i->m_iPriority = MAX_FLOOR_NUM + i->m_iDestFlr;   //p = MAX+i;
            else if( i->m_eRunDir == DIR_DOWN )   //���������ұȵ�ǰ¥���
              i->m_iPriority = 3*MAX_FLOOR_NUM - i->m_iDestFlr; //p = 3*MAX-i;
          }
          else
          {
            if ( i->m_eRunDir == DIR_UP )         //�������ϵ��ȵ�ǰ¥���
              i->m_iPriority = MAX_FLOOR_NUM + i->m_iDestFlr;     //p = MAX+i;
            else if( i->m_eRunDir == DIR_DOWN )   //�������µ��ȵ�ǰ¥���
              i->m_iPriority = MAX_FLOOR_NUM - i->m_iDestFlr;     //p = MAX-i;   
          }
        }
      }
    }
  }
}

/********************************************************************
*  @name     : CElevator::showElevator    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::showElevator()
{
  sRunItemIterator end = m_sRunTable.end();
  for( sRunItemIterator  i=m_sRunTable.begin(); i != end;  ++i )
  {
    LOGE("showElevator:ReqType(%d)-eElvDir(%d)-DestFlr(%d)-Priority(%2d)-Time(%.2f)\n",
      i->m_eReqType,i->m_eRunDir,i->m_iDestFlr,i->m_iPriority,i->m_sTarVal.m_fWaitTime);	
  }
}
/*********************************************************************
*********************************************************************/

