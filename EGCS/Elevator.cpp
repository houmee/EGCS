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
  m_dStartTime    = 0.0;
  m_dNextStateTime = 0.0;
  m_dLastStateTime = 0.0;
  m_eCurState     = IDLE;
  m_eRundir       = DIR_NONE;
  m_isSchedule    = false;
  m_canHandle     = false;
  m_isTrytoDispatch = false;
  m_ElvtFile        = tools;
  m_sRunTable.reserve(MAX_FLOOR_NUM);

  m_lastRunItem.m_eElvDir              = DIR_NONE;
  m_lastRunItem.m_eReqType             = IN_REQ;
  m_lastRunItem.m_iDestFlr             = 1;
  m_lastRunItem.m_iPriority            = 0x7F;
  m_lastRunItem.m_sTarVal.m_fEnergy    = 0;
  m_lastRunItem.m_sTarVal.m_fWaitTime  = 0;
  //insertRunTableItem( m_lastRunItem );
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
  fprintf(m_ElvtFile.m_OutputFilePtr, "\n--Elvt(%d) Main--\n", m_iElvtID);
  fprintf(m_ElvtFile.m_OutputFilePtr, "updateRunInfo:CurFlr(%2d)-NextFlr(%d)-RunDis(%.2f)-CurState(%d)-CurDir(%d)-LastStateTime(%.2f)-NextStateTime(%.2f)\n",m_iCurFlr,m_iNextStopFlr,m_dCurRunDis, m_eCurState,m_eRundir,m_dLastStateTime,m_dNextStateTime);	

  //////////////////////////////////////////////////////////////////////////
  //����������Ҫ����
  if ( ELVT_STOP(m_eCurState) )   //���ݴ��ڴ�����ͣ��״̬������˿�
  {
    processReqPsgFlow(psgVec);

    if ( gSystemTime > m_dLastStateTime )
      gotoNextDest();
  }
  fprintf(m_ElvtFile.m_OutputFilePtr, "updateRunInfo:CurFlr(%2d)-NextFlr(%d)-RunDis(%.2f)-CurState(%d)-CurDir(%d)-LastStateTime(%.2f)-NextStateTime(%.2f)\n",m_iCurFlr,m_iNextStopFlr,m_dCurRunDis, m_eCurState,m_eRundir,m_dLastStateTime,m_dNextStateTime);	
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
          m_dStartTime     = gSystemTime;
        }
        else if ( m_lastRunItem.m_iDestFlr > m_iCurFlr )
          m_eCurState = DOWN_PAUSE;
        else if ( m_lastRunItem.m_iDestFlr < m_iCurFlr )
          m_eCurState = UP_PAUSE;
        else
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
    tmptime = m_dStartTime;
    m_lastRunItem = m_sRunTable.at(0);
    m_dCurRunDis  = 0;
    fprintf(m_ElvtFile.m_OutputFilePtr, "gotoNextDest:Save LastItem-NextStopFlr(%2d)->(%2d)\n",tmpItem.m_iDestFlr, m_lastRunItem.m_iDestFlr);	
    
    //ɾ����ǰͣ����
    tarIter = queryElement( m_sRunTable,m_lastRunItem,tarIter );  
    fprintf(m_ElvtFile.m_OutputFilePtr, "gotoNextDest:Delete LastItem-ReqType(%d)-DestFlr(%d)\n", tarIter->m_eReqType, tarIter->m_iDestFlr);	
    deleteRunTableItem( m_lastRunItem );

    m_dStartTime  = m_dNextStateTime;
    fprintf(m_ElvtFile.m_OutputFilePtr, "gotoNextDest:Save StartTime(%.2f)->(%.2f)\n",tmptime,m_dStartTime);	
  }
  fprintf(m_ElvtFile.m_OutputFilePtr, "gotoNextDest:NextStopFlr(%2d)-Rundir(%d)-NextStateTime(%.2f)\n",m_iNextStopFlr,m_eCurState,m_dNextStateTime);	
  showElevator();  
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
    if ( m_dLastStateTime != m_dStartTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME )
    {
      fprintf(m_ElvtFile.m_OutputFilePtr, "changeNextStop:Elvt(%d)-LastStateTime(%.2f)->(%.2f)\n",m_iElvtID,m_dLastStateTime, m_dStartTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME);	
      m_dLastStateTime = m_dStartTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME;
    }

    if ( m_sRunTable.size() > 0 )   //���������һͣ��¥������
    {
      tmpFlr = m_iNextStopFlr;
      tmpTime = m_dNextStateTime;

      if ( m_sRunTable.at(0).m_iDestFlr != m_iNextStopFlr )   //�����һͣ��¥�㲻һ��
      {
        m_iNextStopFlr   = m_sRunTable.at(0).m_iDestFlr;
        m_dNextStateTime = m_sRunTable.at(0).m_sTarVal.m_fWaitTime + gSystemTime;
        fprintf(m_ElvtFile.m_OutputFilePtr, "changeNextStop:Elvt(%d)-NextStopFlr(%2d)->(%2d)--NextStateTime(%.2f)->(%.2f)\n",m_iElvtID,tmpFlr,m_iNextStopFlr,tmpTime,m_dNextStateTime);
      }
      else
      {
        if( m_dNextStateTime != m_sRunTable.at(0).m_sTarVal.m_fWaitTime + m_dStartTime ) //���ͣ��¥��һ�µ�ʱ�䲻ͬ
        {
          m_dNextStateTime = m_sRunTable.at(0).m_sTarVal.m_fWaitTime + gSystemTime;
          fprintf(m_ElvtFile.m_OutputFilePtr, "changeNextStop:Elvt(%d)-NextStateTime(%.2f)->(%.2f)\n",m_iElvtID,tmpTime,m_dNextStateTime);
        }
      }
      
    }
    //else    //û������������ͣ�������һ�������¥��
    //{
    //  m_iNextStopFlr    = m_iCurFlr;
    //  m_dNextStateTime  = gSystemTime;
    //}
  }
}

/********************************************************************
*  @name     : CElevator::trytoDispatch    
*  @brief    : 
*  @param    : reqIter 
*  @return   : void
********************************************************************/ 
sTargetVal CElevator::trytoDispatch( sOutRequestIterator reqIter )
{
  sRunItem reqRunItem;
  sRunItemIterator tarIter;
  sTargetVal tarVal = {0,0}, tmpTarVal={0,0};

  //���ⲿ�����װΪ���б����ȡ�ӵ�ǰλ�õ������Ŀ��ֵ
  if ( reqIter->m_iReqCurFlr == m_iCurFlr )
    reqRunItem.m_eElvDir = DIR_NONE;
  else
    reqRunItem.m_eElvDir  =( reqIter->m_iReqCurFlr > m_iCurFlr ) ? DIR_UP : DIR_DOWN;

  reqRunItem.m_eReqType = OUT_REQ;
  reqRunItem.m_iDestFlr = reqIter->m_iReqCurFlr;

  m_isTrytoDispatch = true;
  //////////////////////////////////////////////////////////////////////////
  //������Ĵ���ֵ
  tarIter  = queryElement( m_sRunTable,reqRunItem,tarIter );
  if ( tarIter == m_sRunTable.end() )
  {
    insertRunTableItem( reqRunItem );  //���뵱ǰ���б�ͬʱ���±�ĸ���Ŀ��ֵ

    tarIter  = queryElement( m_sRunTable,reqRunItem,tarIter );     //�����������tarIndʧЧ�����²�ѯ
    sRunItemIterator end = m_sRunTable.end();
    
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      if ( i <= tarIter )
      {
        tmpTarVal.m_fEnergy   += i->m_sTarVal.m_fEnergy;
        tmpTarVal.m_fWaitTime += i->m_sTarVal.m_fWaitTime;

        tarVal = tmpTarVal;
      }
      else
      {
        //tarVal.m_fEnergy   += tmpTarVal.m_fEnergy;  //MWT�������ܺ�
        tarVal.m_fWaitTime += OPEN_CLOSE_TIME + PSG_ENTER_TIME;
      }
    }
  }

  deleteRunTableItem( reqRunItem );

  m_isTrytoDispatch = false;

  return tarVal;     //����Ŀ��ֵ
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
  runItem.m_eElvDir = (psg->m_iPsgDestFlr > m_iCurFlr) ? DIR_UP:DIR_DOWN;
  
  insertRunTableItem( runItem );
  fprintf(m_ElvtFile.m_OutputFilePtr, "onClickInnerBtn:Inner Psg(%2d)-DestFlr(%2d)-ElvDir(%d)-Table(%d)-(%d)\n",psg->m_iPsgID,runItem.m_iDestFlr,runItem.m_eElvDir,m_sRunTable.size(),m_sRunTable.capacity());	
}

/********************************************************************
*  @name     : CElevator::processInnerPsgFlow    
*  @brief    : 
*  @param    : sPassengerInfoVec & psgVec 
*  @return   : void
********************************************************************/ 
void CElevator::processReqPsgFlow(sPassengerInfoVec& psgVec)
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
    if (i->m_ePsgState == PSG_WAIT && i->m_iPsgCurFlr == m_iCurFlr )
      if (i->m_ePsgReqDir == m_eRundir || m_eCurState == IDLE  && (m_iCurPsgNum+1) <= MAX_INNER_PSG_NUM)
      {
        psgEnter(i);
        onClickInnerBtn(i);
      }
      if ( i->m_ePsgReqDir != m_eRundir )
      {
        fprintf(m_ElvtFile.m_OutputFilePtr, "processInnerPsgFlow:Inner Req[Psg(%2d)-PsgDest(%2d)-CUrFlr(%2d)] is refused!\n", i->m_iPsgID, i->m_iPsgDestFlr,m_iCurFlr);
      }
  }

  fprintf(m_ElvtFile.m_OutputFilePtr, "processInnerPsgFlow:\n");
  showElevator();
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
  fprintf(m_ElvtFile.m_OutputFilePtr, "psgLeave:Psg(%2d) Leaves Elevator(%d)-CurVol(%2d)\n",psg->m_iPsgID,m_iElvtID,m_iCurPsgNum);	
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
  fprintf(m_ElvtFile.m_OutputFilePtr, "psgEnter:Psg(%2d) Enters Elevator(%d)-CurVol(%2d)\n",psg->m_iPsgID,m_iElvtID,m_iCurPsgNum);	
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
      targetVal.m_fWaitTime = ACCELERATE_TIME + DECELERATE_TIME + CONST_SPEED_TIME + OPEN_CLOSE_TIME+PSG_ENTER_TIME;
      targetVal.m_fEnergy   = START_STOP_ENERGY + GRAVITY_ACCELERATE*(m_iCurPsgNum*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*CONST_SPEED_LENGTH;
    }
    else if ( intervalFlr > 1 )
    {
      //�ȴ�ʱ���������ʱ��Ϳ�����ʱ��
      targetVal.m_fWaitTime = (ACCELERATE_TIME + DECELERATE_TIME + OPEN_CLOSE_TIME + PSG_ENTER_TIME +
        (intervalFlr-2)*ONE_FLOOR_TIME + 2*REMAIN_GAP_TIME);
      //�ܺİ����Ӽ����ܺĺ��ؿ��ܺ�
      targetVal.m_fEnergy = (GRAVITY_ACCELERATE*(m_iCurPsgNum*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*
        ((intervalFlr-2)*FLOOR_HEIGHT + 2*REMAIN_GAP_LENGTH) +START_STOP_ENERGY);
    }  
  }
  else
  {
    targetVal.m_fWaitTime = OPEN_CLOSE_TIME + PSG_ENTER_TIME;
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
void CElevator::insertRunTableItem( sRunItem runInd )
{
  sRunItemIterator indIter;

  if ( runInd.m_eReqType == IN_REQ )
  {
    if ( m_eCurState == IDLE || m_eCurState == DOWN_PAUSE || m_eCurState == UP_PAUSE ||
       ( runInd.m_eElvDir == DIR_DOWN && ELVT_DOWN(m_eCurState)) ||
       ( runInd.m_eElvDir == DIR_UP   && ELVT_UP(m_eCurState))
       )
    {
      if (queryElement(m_sRunTable,runInd,indIter) != m_sRunTable.end())    //������һ����!���Ƚ�
        return ;
      else
      {
        getTaskPriority( runInd );
        insertElement( m_sRunTable, runInd );
        sortElement( m_sRunTable, runInd );
        updateRunTable();
        changeNextStop();
      }
    }
  }
  else
  {
    if ( queryElement(m_sRunTable,runInd,indIter) != m_sRunTable.end() )
      return;
    else
    {
      getTaskPriority( runInd );
      insertElement( m_sRunTable, runInd );
      sortElement( m_sRunTable, runInd );
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
void CElevator::deleteRunTableItem( sRunItem runInd )
{
  sRunItemIterator tarInd;
  deleteElement( m_sRunTable,runInd,tarInd );
  sortElement( m_sRunTable, runInd );
  updateRunTable();
  changeNextStop();
}

/********************************************************************
*  @name     : getTaskPriority    
*  @brief    : calculate priority according to current dir and request dir
*  @param    : ind 
*  @return   : void
********************************************************************/ 
void CElevator::getTaskPriority(sRunItem& ind)
{
  //////////////////////////////////////////////////////////////////////////
  //���ݴ��������е��������ڼ�ʹ��������¥�������¥����һ�㣬��������ʱ�Ѿ�������������������ȼ���ܵ�
  if ( m_eCurState == IDLE )        //������ݴ���
  {
    if ( ind.m_eElvDir == DIR_UP )
      ind.m_iPriority = ind.m_iDestFlr > m_iCurFlr ? ind.m_iDestFlr : (MAX_FLOOR_NUM+ind.m_iDestFlr);
    else if (ind.m_eElvDir == DIR_DOWN )
      ind.m_iPriority = MAX_FLOOR_NUM*2-ind.m_iDestFlr;
    else
      ind.m_iPriority = 0;          //�������ΪNONE��˵�����ڱ��㣬���ȼ����
  }
  else if ( ELVT_UP(m_eCurState) )  //��������
  {
    if ( ind.m_eElvDir == DIR_UP )
      ind.m_iPriority = ind.m_iDestFlr > m_iCurFlr ? ind.m_iDestFlr : (MAX_FLOOR_NUM+ind.m_iDestFlr);
    else if (ind.m_eElvDir == DIR_DOWN )
      ind.m_iPriority = MAX_FLOOR_NUM*2-ind.m_iDestFlr;
    else                            //�������ΪNONE��˵���������иչ������ȼ��ܵ�
      ind.m_iPriority = MAX_FLOOR_NUM*2+ind.m_iDestFlr;    
  }
  else if ( ELVT_DOWN(m_eCurState) )
  {
    if (ind.m_eElvDir == DIR_UP )
      ind.m_iPriority = ind.m_iDestFlr;
    else if (ind.m_eElvDir == DIR_DOWN )
      ind.m_iPriority = ind.m_iDestFlr > m_iCurFlr ? -ind.m_iDestFlr : (MAX_FLOOR_NUM+ind.m_iDestFlr);
    else
      ind.m_iPriority = MAX_FLOOR_NUM*2+ind.m_iDestFlr;
  }
  else
    ind.m_iPriority = 0x7F;
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
    fprintf(m_ElvtFile.m_OutputFilePtr, "showElevator:ReqType(%d)-eElvDir(%d)-DestFlr(%d)-Time(%.2f)\n",
      i->m_eReqType,i->m_eElvDir,i->m_iDestFlr,i->m_sTarVal.m_fWaitTime);	
  }
}
/*********************************************************************
*********************************************************************/

