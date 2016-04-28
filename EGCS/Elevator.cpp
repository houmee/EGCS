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

CElevator::CElevator()
{
  m_iElvtID       = -1;
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
  m_sRunTable.reserve(MAX_FLOOR_NUM);

  m_lastRunIndex.m_eElvDir              = DIR_NONE;
  m_lastRunIndex.m_eReqType             = IN_REQ;
  m_lastRunIndex.m_iDestFlr             = 1;
  m_lastRunIndex.m_iPriority            = 0x7F;
  m_lastRunIndex.m_sTarVal.m_fEnergy    = 0;
  m_lastRunIndex.m_sTarVal.m_fWaitTime  = 0;

  showElevator(1);
}



/********************************************************************
*  @name     : CElevator::initELevator    
*  @brief    : 
*  @param    : id 
*  @return   : void
********************************************************************/ 
void CElevator::initELevator(int id, CTools& tools)
{
  m_iElvtID = id;
  m_ElvtFile = tools;
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
  updateRunInfo();

  if ( ELVT_STOP(m_eCurState) )
  {
    processInnerPsgFlow(psgVec);
    gotoNextDest();
  }
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

  if ( !ELVT_STOP(m_eCurState) )    //如果电梯没有待机中
  {  
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

    if ( m_eCurState == UP_ACC || m_eCurState == DOWN_ACC )
      m_dCurRunDis = (double)runTime/ACCELERATE_TIME*ACCELERATE_LENGTH;
    else if( m_eCurState == UP_CONST || m_eCurState == DOWN_CONST )
      m_dCurRunDis = (double)( runTime - ACCELERATE_TIME )*RUN_SPEED + ACCELERATE_LENGTH;
    else if ( m_eCurState == UP_DEC || m_eCurState == DOWN_DEC )
      m_dCurRunDis = abs(m_lastRunIndex.m_iDestFlr - m_iNextStopFlr)*FLOOR_HEIGHT-(double)remainTime/DECELERATE_TIME*DECELERATE_LENGTH;

    if ( ELVT_UP(m_eCurState) )
      m_iCurFlr = (uint8)(m_dCurRunDis / FLOOR_HEIGHT) + m_lastRunIndex.m_iDestFlr;
    else if ( ELVT_DOWN(m_eCurState) )
      m_iCurFlr = m_lastRunIndex.m_iDestFlr - (uint8)(m_dCurRunDis / FLOOR_HEIGHT);

    if ( m_iCurFlr > MAX_FLOOR_NUM )
      m_iCurFlr = MAX_FLOOR_NUM;
    if ( m_iCurFlr < 0 )
      m_iCurFlr = 0;
  }

  if ( gSystemTime > m_dNextStateTime )
  {
    if ( m_eCurState != IDLE )
    {
      if ( m_eRundir == DIR_UP )
        m_eCurState = UP_PAUSE;
      else if ( m_eRundir == DIR_DOWN )
        m_eCurState = DOWN_PAUSE;
      else
        m_eCurState = IDLE;

      if ( m_iCurPsgNum == 0 && m_eCurState == DOWN_PAUSE || m_eCurState == UP_PAUSE )
        m_eCurState = IDLE;
    }
    else
    {
      m_dNextStateTime = gSystemTime;
      m_dLastStateTime = gSystemTime;
      m_dStartTime     = gSystemTime;
    }
  }
  fprintf(m_ElvtFile.m_OutputFilePtr, "updateRunInfo:CurFlr(%2d)-NextFlr(%d)-RunDis(%.2f)-CurState(%d)-LastStateTime(%.2f)-NextStateTime(%.2f)\n",m_iCurFlr,m_iNextStopFlr,m_dCurRunDis, m_eCurState,m_dLastStateTime,m_dNextStateTime);	
}

/********************************************************************
*  @name     : CElevator::gotoNextDest    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::gotoNextDest()
{
  m_dLastStateTime = m_dStartTime + PSG_ENTER_TIME + OPEN_CLOSE_TIME;

  //保存上一停靠的状态
  if ( m_sRunTable.size() > 0 && m_iCurFlr == m_sRunTable.at(0).m_iDestFlr )
  {
    m_lastRunIndex = m_sRunTable[0];
    m_dCurRunDis = 0;
    m_dStartTime = m_dNextStateTime;
    sRunItemIterator deleteIter = find( m_sRunTable.begin(), m_sRunTable.end(), m_lastRunIndex );
    fprintf(m_ElvtFile.m_OutputFilePtr, "gotoNextDest: Delete item-ReqType(%d)-DestFlr(%d)\n", deleteIter->m_eReqType, deleteIter->m_iDestFlr);	
    m_sRunTable.erase( deleteIter );
  }

  fprintf(m_ElvtFile.m_OutputFilePtr, "gotoNextDest:NextStopFlr(%2d)-Rundir(%d)-NextStateTime(%.2f)\n",m_iNextStopFlr,m_eCurState,m_dNextStateTime);	
  showElevator(1);  
}


/********************************************************************
*  @name     : CElevator::getElevatorInfo    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CElevator::changeNextStop()
{
  int tmpFlr;
  double tmpTime;

  if ( m_sRunTable.size() > 0 )
  {
    if ( m_sRunTable.at(0).m_iDestFlr != m_iNextStopFlr )
    {
      tmpFlr = m_iNextStopFlr;
      tmpTime = m_dNextStateTime;
      m_iNextStopFlr   = m_sRunTable.at(0).m_iDestFlr;
      m_dNextStateTime = m_sRunTable.at(0).m_sTarVal.m_fWaitTime + gSystemTime;

      if ( m_iNextStopFlr == m_iCurFlr )
      {
        m_eCurState = IDLE;
        m_eRundir   = DIR_NONE;
      }
      else
      {
        m_eCurState = m_iNextStopFlr > m_iCurFlr ? UP_ACC : DOWN_ACC;
        m_eRundir   = m_iNextStopFlr > m_iCurFlr ? DIR_UP : DIR_DOWN;
      }  

      fprintf(m_ElvtFile.m_OutputFilePtr, "changeNextStop:Elvt(%d)-NextStopFlr(%2d)->(%2d)---NextStateTime(%.2f)->(%.2f)\n",m_iElvtID,tmpFlr,m_iNextStopFlr,tmpTime,m_dNextStateTime);
    }
  }
  else
  {
    m_iNextStopFlr    = m_iCurFlr;
    m_eCurState       = IDLE;
    m_eRundir         = DIR_NONE;
    m_dNextStateTime  = gSystemTime;
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
  sRunItem reqRunInd;
  sRunItemIterator tarInd;
  sTargetVal tarVal = {0,0}, tmpTarVal={0,0};

  //将外部请求封装为运行表项，获取从当前位置到请求的目标值
  if ( reqIter->m_iReqCurFlr == m_iCurFlr )
    reqRunInd.m_eElvDir = DIR_NONE;
  else
    reqRunInd.m_eElvDir  =( reqIter->m_iReqCurFlr > m_iCurFlr ) ? DIR_UP : DIR_DOWN;

  reqRunInd.m_eReqType = OUT_REQ;
  reqRunInd.m_iDestFlr = reqIter->m_iReqCurFlr;

  //////////////////////////////////////////////////////////////////////////
  //求插入后的代价值
  tarInd  = queryElement( m_sRunTable,reqRunInd,tarInd );
  if ( tarInd == m_sRunTable.end() )
  {
    insertRunTableItem( reqRunInd );  //插入当前运行表同时更新表的各项目标值

    tarInd  = queryElement( m_sRunTable,reqRunInd,tarInd );     //插入操作导致tarInd失效，重新查询
    sRunItemIterator end = m_sRunTable.end();
    
    for( sRunItemIterator i=m_sRunTable.begin(); i != end; ++i )   
    {
      if ( i <= tarInd )
      {
        tmpTarVal.m_fEnergy   += i->m_sTarVal.m_fEnergy;
        tmpTarVal.m_fWaitTime += i->m_sTarVal.m_fWaitTime;

        tarVal = tmpTarVal;
      }
      else
      {
        //tarVal.m_fEnergy   += tmpTarVal.m_fEnergy;  //MWT不考虑能耗
        tarVal.m_fWaitTime += OPEN_CLOSE_TIME + PSG_ENTER_TIME;
      }
    }
  }

  deleteRunTableItem( reqRunInd );
  return tarVal;     //返回目标值
}

/********************************************************************
*  @name     : CElevator::onClickInnerBtn    
*  @brief    : 
*  @param    : psg 
*  @return   : void
********************************************************************/ 
void CElevator::onClickInnerBtn(sPassengerIterator& psg)
{
  sRunItem runInd;
  sRunItemIterator indIter;

  runInd.m_eReqType = IN_REQ;
  runInd.m_iDestFlr = psg->m_iDestFlr;
  runInd.m_eElvDir = (psg->m_iDestFlr > m_iCurFlr) ? DIR_UP:DIR_DOWN;
  
  if (m_sRunTable.size() == 1)
  {
    int a = 0;
  }
  insertRunTableItem( runInd );
  fprintf(m_ElvtFile.m_OutputFilePtr, "onClickInnerBtn:Inner Psg(%2d)-DestFlr(%2d)-ElvDir(%d)-Table(%d)-(%d)\n",psg->m_iPsgID,runInd.m_iDestFlr,runInd.m_eElvDir,m_sRunTable.size(),m_sRunTable.capacity());	
}

/********************************************************************
*  @name     : CElevator::processInnerPsgFlow    
*  @brief    : 
*  @param    : sPassengerInfoVec & psgVec 
*  @return   : void
********************************************************************/ 
void CElevator::processInnerPsgFlow(sPassengerInfoVec& psgVec)
{
  sPassengerIterator psgIterEnd = psgVec.end();

  if ( m_iCurPsgNum > 0 )
  {
    for( sPassengerIterator i=psgVec.begin(); i != psgIterEnd;  ++i )
    {
      //如果乘客处在当前电梯中且之前状态为乘梯中
      if ( i->m_iCurPlace == m_iElvtID && i->m_ePsgState == PSG_TRAVEL )  
      {
        if ( m_iCurFlr == i->m_iDestFlr && ELVT_STOP( m_eCurState ) ) //如果当前层为乘客目标层
          psgLeave(i);    //离开电梯
      }
    }
  }

  if ( m_sRunTable.size() > 0 )
  {
    for( sPassengerIterator i=psgVec.begin(); i != psgIterEnd;  ++i )
    {
      //如果乘客处于等待中且当前电梯已经到达停靠
      if (i->m_ePsgState == PSG_WAIT && i->m_iReqCurFlr == m_iCurFlr )
        if (i->m_eReqDir == m_eRundir || m_eCurState == IDLE  && (m_iCurPsgNum+1) <= MAX_INNER_PSG_NUM)
        {
          psgEnter(i);
          onClickInnerBtn(i);
        }
    }
  }

  fprintf(m_ElvtFile.m_OutputFilePtr, "processInnerPsgFlow:\n");
  showElevator(1);
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
      targetVal.m_fWaitTime = ACCELERATE_TIME + DECELERATE_TIME + CONST_SPEED_TIME + OPEN_CLOSE_TIME+PSG_ENTER_TIME;
      targetVal.m_fEnergy   = START_STOP_ENERGY + GRAVITY_ACCELERATE*(m_iCurPsgNum*PSG_AVG_WEIGHT+NET_CAR_WEIGHT)*CONST_SPEED_LENGTH;
    }
    else if ( intervalFlr > 1 )
    {
      //等待时间包括运行时间和开关门时间
      targetVal.m_fWaitTime = (ACCELERATE_TIME + DECELERATE_TIME + OPEN_CLOSE_TIME + PSG_ENTER_TIME +
        (intervalFlr-2)*ONE_FLOOR_TIME + 2*REMAIN_GAP_TIME);
      //能耗包括加减速能耗和载客能耗
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
  psg->m_dAllTime = gSystemTime - psg->m_dReqTime;
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
  psg->m_dWaitTime = gSystemTime - psg->m_dReqTime;
  psg->m_ePsgState = PSG_TRAVEL;
  psg->m_iCurPlace = m_iElvtID;
  fprintf(m_ElvtFile.m_OutputFilePtr, "psgEnter:Psg(%2d) Enters Elevator(%d)-CurVol(%2d)\n",psg->m_iPsgID,m_iElvtID,m_iCurPsgNum);	
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
      i->m_sTarVal = runfromXtoY( m_lastRunIndex, preRunInd );
    }
    else
    {
      nowRunInd = *i;
      i->m_sTarVal = runfromXtoY( preRunInd, nowRunInd );
      preRunInd = *i;
    }
  }

  changeNextStop();
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
      if (queryElement(m_sRunTable,runInd,indIter) != m_sRunTable.end())    //迭代器一般用!做比较
        return ;
      else
      {
        getTaskPriority( runInd );
        insertElement( m_sRunTable, runInd );
        sortElement( m_sRunTable, runInd );
        updateRunTable();
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
  //电梯待机和上行的区别在于即使电梯所在楼层和请求楼层在一层，但是上行时已经启动，本次请求的优先级会很低
  if ( m_eCurState == IDLE )        //如果电梯待机
  {
    if ( ind.m_eElvDir == DIR_UP )
      ind.m_iPriority = ind.m_iDestFlr > m_iCurFlr ? ind.m_iDestFlr : (MAX_FLOOR_NUM+ind.m_iDestFlr);
    else if (ind.m_eElvDir == DIR_DOWN )
      ind.m_iPriority = MAX_FLOOR_NUM*2-ind.m_iDestFlr;
    else
      ind.m_iPriority = 0;          //如果请求为NONE，说明就在本层，优先级最高
  }
  else if ( ELVT_UP(m_eCurState) )  //电梯上行
  {
    if ( ind.m_eElvDir == DIR_UP )
      ind.m_iPriority = ind.m_iDestFlr > m_iCurFlr ? ind.m_iDestFlr : (MAX_FLOOR_NUM+ind.m_iDestFlr);
    else if (ind.m_eElvDir == DIR_DOWN )
      ind.m_iPriority = MAX_FLOOR_NUM*2-ind.m_iDestFlr;
    else                            //如果请求为NONE，说明电梯上行刚过，优先级很低
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
*  @param    : op 
*  @return   : void
********************************************************************/ 
void CElevator::showElevator(uint8 op)
{
  if (op == 1)
  {  
    for (int i=0;i!=m_sRunTable.size();i++) 
    {
      //printf("showElevator:Elvt(%d)-ReqType(%d)-eElvDir(%d)-DestFlr(%d)\n",
      //  m_iElvtID,m_sRunTable.at(i).m_eReqType,m_sRunTable.at(i).m_eElvDir,m_sRunTable.at(i).m_iDestFlr);	
      fprintf(m_ElvtFile.m_OutputFilePtr, "showElevator:ReqType(%d)-eElvDir(%d)-DestFlr(%d)-Time(%.2f)\n",
        m_sRunTable.at(i).m_eReqType,m_sRunTable.at(i).m_eElvDir,m_sRunTable.at(i).m_iDestFlr,m_sRunTable.at(i).m_sTarVal.m_fWaitTime);	
    }
  }
}

//指针模式访问
//typedef  vector<sRunIndexPtr>:: iterator   VIntIterator;
//VIntIterator  end = m_sRunTable.begin();
//sRunIndexPtr ptr;
//for( VIntIterator  i=m_sRunTable.begin(); i != end;  ++i )
//{
//  ptr = *i;
//  printf("DIR:%d--DEST_FLR:%d\n", ptr->ElvDir,ptr->);
//}

//非指针模式访问
//typedef  vector<sRunIndex>:: iterator   VIntIterator;
//VIntIterator end = m_sRunTable.begin();

//for( VIntIterator  i=m_sRunTable.begin(); i != end;  ++i )
//{
//  printf("DIR:%d--DEST_FLR:%d\n", i->ElvDir,i->DestFlr);
//}

/********************************************************************
*  @name     : CElevator::deleteTask    
*  @brief    : delete task from run table
*  @param    : ind 
*  @return   : bool
********************************************************************/ 
//bool CElevator::deleteTask(sRunIndex ind)
//{
//  sRunIndexIterator j = find( m_sRunTable.begin(), m_sRunTable.end(), ind );
//  if( j != m_sRunTable.end() )
//  {
//    m_sRunTable.erase(j);
//    return true;
//  }
//  else
//    return false;
//}

/********************************************************************
*  @name     : CElevator::insertTask    
*  @brief    : insert task in run table according to priority
*  @param    : ind 
*  @return   : void
********************************************************************/ 
//void CElevator::insertTask(sRunIndex ind)
//{
//  getTaskPriority( &ind );
//  m_sRunTable.push_back(ind);
//  sortTask();
//}

/********************************************************************
*  @name     : CElevator::sortTask    
*  @brief    : 
*  @param    :  
*  @return   : void
********************************************************************/ 
//void CElevator::sortTask(void)
//{
//  sort(m_sRunTable.begin(), m_sRunTable.end(), less<sRunIndex>());
//}

//模板运算
//sRunIndex runind = {0,MAX_PRIORITY_NUM,DIR_NONE,0};
//insertElement(&m_sRunTable,runind);

//runind.m_iDestFlr = 1;
//insertElement(&m_sRunTable,runind);

//runind.m_iDestFlr = 2;
//insertElement(&m_sRunTable,runind);

//runind.m_iDestFlr = 3;
//insertElement(&m_sRunTable,runind);
//
//sRunIndexIterator it;
//deleteElement( &m_sRunTable, runind, it);


////预留空间，但不一定初始化
//for (uint8 i=0; i<MAX_FLOOR_NUM; i++)
//{
//  sRunIndex runind = {MAX_PRIORITY_NUM,DIR_NONE,0};
//  insertTask(runind);
//}

//at模式访问元素与溢出处理
//try
//{
//  m_sRunTable.at(12);
//}
//catch (const exception& e)
//{
//  printf("ERROR:%s--%s--Line%d",e.what(),__FILE__,__LINE__);
//}
/*********************************************************************
*********************************************************************/

