/******************************************************************** 
修改时间:        2016/03/20 16:39
文件名称:        Elevator.cpp
文件作者:        huming 
=====================================================================
功能说明:        电梯模型设计 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
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
  //电梯运行主要部分
  if ( !m_isSchedule )
  {
    m_isSchedule = true;
    LOGA("Elevator_Main: LastSysTime(%.2f)->(%.2f)-Schedule(true)\n",m_dLastSysTime,gSystemTime);
    m_dLastSysTime = gSystemTime;
  }

  if ( gSystemTime > m_dNextStateTime )
    gotoNextDest();

  if ( ELVT_STOP(m_eCurState) )   //电梯处于待机、停靠状态，处理乘客
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
  //由停止向运动变化
  //状态变化：1.IDLE->IDLE         2.PAUSE->PAUSE     3.IDLE->ACC           4.PAUSE->ACC
  //方向变化: 1.DIR_NONE->DIR_NONE 2.DIR_NONE->DIR_UP 3.DIR_NONE->DIR_DOWN
  if ( ELVT_STOP(m_eCurState) ) //电梯在停止状态  
  {
    if ( gSystemTime > m_dLastStateTime )    //超过了电梯开关门时间
    { 
      if ( m_iNextStopFlr != m_iCurFlr )      //如果下一停靠楼层不是当前楼层，更新状态和方向
      {
        m_eCurState = m_iNextStopFlr > m_iCurFlr ? UP_ACC : DOWN_ACC;
        m_eRundir   = m_iNextStopFlr > m_iCurFlr ? DIR_UP : DIR_DOWN;
      }
      else                                    //如果下一停靠楼层是当前楼层，不改变状态和方向
      {
        if ( m_sRunTable.size() == 0 )        //如果没有任务，则将状态改为IDLE，断定此处电梯没有乘客
        {
          assert(m_iCurPsgNum == 0);
          m_eCurState = IDLE;

          m_dLastStateTime = gSystemTime;     //没有任务的时候将时间刷新
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

  if ( !ELVT_STOP(m_eCurState) ) //如果电梯在运行中
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

    if ( m_eCurState == UP_ACC || m_eCurState == DOWN_ACC )           //向上（向下）加速状态
      m_dCurRunDis = (double)runTime/ACCELERATE_TIME*ACCELERATE_LENGTH;
    else if( m_eCurState == UP_CONST || m_eCurState == DOWN_CONST )   //匀速状态
      m_dCurRunDis = (double)( runTime - ACCELERATE_TIME )*RUN_SPEED + ACCELERATE_LENGTH;
    else if ( m_eCurState == UP_DEC || m_eCurState == DOWN_DEC )      //向上（向下）加速状态
      m_dCurRunDis = abs(m_lastRunItem.m_iDestFlr - m_iNextStopFlr)*FLOOR_HEIGHT-(double)remainTime/DECELERATE_TIME*DECELERATE_LENGTH;

    //////////////////////////////////////////////////////////////////////////
    //根据上一停靠楼层、运行距离和运行方向确定电梯所在楼层
    if ( ELVT_UP(m_eCurState) )
      m_iCurFlr = (uint8)(m_dCurRunDis / FLOOR_HEIGHT) + m_lastRunItem.m_iDestFlr;
    else if ( ELVT_DOWN(m_eCurState) )
      m_iCurFlr = m_lastRunItem.m_iDestFlr - (uint8)(m_dCurRunDis / FLOOR_HEIGHT);

    if ( m_iCurFlr > MAX_FLOOR_NUM )
      m_iCurFlr = MAX_FLOOR_NUM;
    if ( m_iCurFlr < 1 )
      m_iCurFlr = 1;

    //////////////////////////////////////////////////////////////////////////
    //由运动向停止变化
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

  //保存上一停靠的状态
  if ( m_sRunTable.size() > 0 && m_iCurFlr == m_iNextStopFlr )
  {
    //保存当前停靠楼层信息
    tmpItem = m_lastRunItem;
    tmptime = m_dLastSysTime;
    m_lastRunItem = m_sRunTable.at(0);
    m_dCurRunDis  = 0;
    LOGA("gotoNextDest:Save LastItem-Elvt(%d)-NextStopFlr(%2d)->(%2d)\n",m_iElvtID,tmpItem.m_iDestFlr, m_lastRunItem.m_iDestFlr);	
    
    //删除当前停靠点
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
 
  if ( !m_isTrytoDispatch )         //只有在非TrytoDispatch操作时检查是否更新下一停靠楼层信息
  {
    if ( m_sRunTable.size() > 0 )   //如果存在下一停靠楼层任务
    {
      tmpFlr = m_iNextStopFlr;
      tmpTime = m_dNextStateTime;

      if ( m_sRunTable.at(0).m_iDestFlr != m_iNextStopFlr )   //如果下一停靠楼层不一致
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
        
        if( m_dNextStateTime != m_sRunTable.at(0).m_sTarVal.m_fWaitTime + m_dLastStateTime ) //如果停靠楼层一致但时间不同
        {
          m_dNextStateTime = m_sRunTable.at(0).m_sTarVal.m_fWaitTime + m_dLastStateTime;
          LOGA("changeNextStop:Elvt(%d)-NextStateTime(%.2f)->(%.2f)\n",m_iElvtID,tmpTime,m_dNextStateTime);
        }
      }
      
    }
    else    //没有运行任务则停靠再最后一个任务的楼层
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

  //将外部请求封装为运行表项，获取从当前位置到请求的目标值
  reqRunItem.m_eRunDir  = reqIter->m_eReqDir;
  reqRunItem.m_eReqType = OUT_REQ;
  reqRunItem.m_iDestFlr = reqIter->m_iReqCurFlr;

  m_isTrytoDispatch = true;
  //////////////////////////////////////////////////////////////////////////
  //求插入后的代价值
  tarIter = queryElement( m_sRunTable,reqRunItem,tarIter );
  if ( tarIter == m_sRunTable.end() )
  {
    //////////////////////////////////////////////////////////////////////////
    //求解插入前能耗(人数的增加导致的运行能耗和启停能耗)
    sRunItemIterator end = m_sRunTable.end();
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      /************************************************************************
      * 1.判断运行列表下一表项的目的楼层
      * 2.计算到达时间，判断其他电梯是否在达到时间内也到达目的楼层
      * 3.如果没有其他电梯，计算停靠楼层后，轿厢内乘客数目
      * 4.计算能耗
      * 5.循环计算所有表项
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
    //插入运行表项
    insertRunTableItem( reqRunItem );  //插入当前运行表同时更新表的各项目标值
    tarIter = queryElement( m_sRunTable,reqRunItem,tarIter );     //插入操作导致tarInd失效，重新查询
    
    //////////////////////////////////////////////////////////////////////////
    //求解等待时间
    end = m_sRunTable.end();
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      if ( tarIter >= i )   //在iter后面的全部增加由于插入iter增加的时间
      {
        tmpTarVal.m_fWaitTime += i->m_sTarVal.m_fWaitTime;
        tarVal.m_fWaitTime     = tmpTarVal.m_fWaitTime;
      }
      else                  //在iter前面的不增加等待时间，只增加iter的开关门时间
        tarVal.m_fWaitTime += OPEN_CLOSE_TIME + PSG_ENTER_TIME;
    }

    //////////////////////////////////////////////////////////////////////////
    //求解插入后能耗(人数的增加导致的运行能耗和启停能耗)
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      /************************************************************************
      * 1.判断运行列表下一表项的目的楼层
      * 2.计算到达时间，判断其他电梯是否在达到时间内也到达目的楼层
      * 3.如果没有其他电梯，计算停靠楼层后，轿厢内乘客数目
      * 4.计算能耗
      * 5.循环计算所有表项
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
  //删除表项
  deleteRunTableItem( reqRunItem );

  m_isTrytoDispatch = false;

  return tarVal;     //返回目标值
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
      //如果乘客处在当前电梯中且之前状态为乘梯中
      if ( i->m_iCurPlace == m_iElvtID && i->m_ePsgState == PSG_TRAVEL )  
      {
        if ( m_iCurFlr == i->m_iPsgDestFlr && ELVT_STOP( m_eCurState ) ) //如果当前层为乘客目标层
          psgLeave(i);    //离开电梯
      }
    }
  }


  for( sPassengerIterator i=psgVec.begin(); i != psgIterEnd;  ++i )
  {
    //如果乘客处于等待中且当前电梯已经到达停靠
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
  //更新乘客状态
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
  uint8 intervalFlr = abs(x.m_iDestFlr - y.m_iDestFlr);  //楼层间距

  if ( intervalFlr > 0 )
  {
    if ( intervalFlr == 1 )
    {
      targetVal.m_fWaitTime = ACCELERATE_TIME + DECELERATE_TIME + CONST_SPEED_TIME;
      targetVal.m_fEnergy   = START_STOP_ENERGY + GRAVITY_ACCELERATE*(m_iCurPsgNum*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*CONST_SPEED_LENGTH;
    }
    else if ( intervalFlr > 1 )
    {
      //等待时间包括运行时间和开关门时间
      targetVal.m_fWaitTime = (ACCELERATE_TIME + DECELERATE_TIME  +
        (intervalFlr-2)*ONE_FLOOR_TIME + 2*REMAIN_GAP_TIME);
      //能耗包括加减速能耗和载客能耗
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
      if (queryElement(m_sRunTable,runItem,indIter) != m_sRunTable.end())    //迭代器一般用!做比较
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

  if ( m_sRunTable.size() > 0 )   //只有运行列表存在表项时才会更新
  {
    sRunItemIterator end = m_sRunTable.end();
    //////////////////////////////////////////////////////////////////////////
    //确定潜在方向
    if ( m_eCurState == IDLE )
    {
      assert( m_eRundir == DIR_NONE );      //待机的时候电梯肯定没有方向

      for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )
      {
        if ( m_iCurFlr == i->m_iDestFlr )   //如果下一个运行目的楼层就是当前楼层
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
    //根据方向和楼层求优先级
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )
    {
      if ( runDir == DIR_UP )         //如果运行列表方向向上
      {
        if ( i->m_iDestFlr == m_iCurFlr && i->m_eRunDir == DIR_UP && (m_eCurState == IDLE || m_eCurState == UP_PAUSE) ) //当电梯向上停靠或待机，表项目的楼层与当前楼层一致时，优先级最高
            i->m_iPriority = 0;
        else                                //在其他状态，比如方向向下，或向上加速、匀速时，求解优先级
        {
          if ( i->m_iDestFlr > m_iCurFlr )  
          {
            if ( i->m_eRunDir == DIR_UP )         //方向向上且比当前楼层高
              i->m_iPriority = i->m_iDestFlr;     //p = i;
            else if ( i->m_eRunDir == DIR_DOWN )  //方向向下且比当前楼层高
              i->m_iPriority = 2*MAX_FLOOR_NUM - i->m_iDestFlr;   //p = 2*MAX-i;
          }
          else
          {
            if ( i->m_eRunDir == DIR_UP )         //方向向上但比当前楼层低
              i->m_iPriority = 2*MAX_FLOOR_NUM + i->m_iDestFlr;   //p = 2*MAX+i;
            else if( i->m_eRunDir == DIR_DOWN )   //方向向下但比当前楼层低
              i->m_iPriority = 2*MAX_FLOOR_NUM - i->m_iDestFlr;   //p = 2*MAX-i;
          }
        }
      }
      else if ( runDir == DIR_DOWN )       //如果运行列表方向向下
      {

        if ( i->m_iDestFlr == m_iCurFlr && i->m_eRunDir == DIR_DOWN && (m_eCurState == IDLE || m_eCurState == DOWN_PAUSE) ) //当电梯向上停靠或待机，表项目的楼层与当前楼层一致时，优先级最高
          i->m_iPriority = 0;
        else                          //在其他状态，比如方向向上，或向下加速、匀速时，求解优先级
        {
          if ( i->m_iDestFlr >= m_iCurFlr )
          {
            if ( i->m_eRunDir == DIR_UP )         //方向向上且比当前楼层高
              i->m_iPriority = MAX_FLOOR_NUM + i->m_iDestFlr;   //p = MAX+i;
            else if( i->m_eRunDir == DIR_DOWN )   //方向向下且比当前楼层高
              i->m_iPriority = 3*MAX_FLOOR_NUM - i->m_iDestFlr; //p = 3*MAX-i;
          }
          else
          {
            if ( i->m_eRunDir == DIR_UP )         //方向向上但比当前楼层低
              i->m_iPriority = MAX_FLOOR_NUM + i->m_iDestFlr;     //p = MAX+i;
            else if( i->m_eRunDir == DIR_DOWN )   //方向向下但比当前楼层低
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

