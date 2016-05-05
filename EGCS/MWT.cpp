/******************************************************************** 
修改时间:        2016/04/06 21:45
文件名称:        MWT.cpp
文件作者:        huming 
=====================================================================
功能说明:        最小等待时间算法 
--------------------------------------------------------------------- 
版本编号:        
--------------------------------------------------------------------- 
其他说明:         
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
bool CMWT::Core_Main()
{
  generateElevatorVec();      //产生电梯群
  generatePsgFlow();          //产生乘客交通流
  //testPsgFlow();

  while ( !isAlgFinished() )  //所有人是否都已经到达目的层        
  {

    gSystemTime += SYSTEM_TIME_STEP;

    LOGE("\n++++++++++++++++Core_Main:+++%.2f++++++++++++++++\n",gSystemTime);

    //////////////////////////////////////////////////////////////////////////
    //测试部分
    //多少人到达目的地
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
    //多少等待时间超过一定时间
    for( sPassengerIterator  i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
    {
      if ( (gSystemTime-i->m_dPsgReqTime > 50) && i->m_ePsgState == PSG_WAIT )
        LOGE("Psg(%3d) waits for (%.2f) seconds!\n",i->m_iPsgID,gSystemTime-i->m_dPsgReqTime);

      if ( (gSystemTime- i->m_dPsgReqTime- i->m_dWaitTime > 50) && i->m_ePsgState == PSG_TRAVEL )
        LOGE("Psg(%3d) travels for (%.2f) seconds!\n",i->m_iPsgID,gSystemTime-i->m_dPsgReqTime);

      if ( i->m_iCurPlace == 0x5A )
      {
        i->m_iCurPlace = 0x11;
        LOGE("Psg(%3d)-ReqTime(%.2f)->(%.2f)\n",i->m_iPsgID,i->m_dPsgReqTime,gSystemTime + 5);
        i->m_dPsgReqTime = gSystemTime + 5;
      }
    }

    //////////////////////////////////////////////////////////////////////////

    //更新电梯状态
    CElevatorIterator elvtIterEnd = m_elevatorVec.end();
    for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
      i->updateRunInfo();

    //处理外部请求
    processOuterReqFlow();    //处理外部请求
    schedule();               //将外部请求分派调度

    //电梯控制
    for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
      i->Elevator_Main(m_outReqVec, m_passengerVec);

    //////////////////////////////////////////////////////////////////////////
    //异常退出
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
void CMWT::schedule()
{
  CElevatorIterator elvtIter;

  sOutRequestIterator reqIterEnd = m_outReqVec.end();
  for( sOutRequestIterator i=m_outReqVec.begin(); i != reqIterEnd;  ++i )
  {
    elvtIter = fitness(i);
    if ( elvtIter != m_elevatorVec.end() )
      dispatch(i,elvtIter);       //处理每一个请求
  }
  
  if ( m_outReqVec.size() > 0 )   //处理完所有请求，清除请求列表
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
  CElevatorIterator bestElvtIter = m_elevatorVec.begin();
  CElevatorIterator elvtIterEnd = m_elevatorVec.end();

  /**
  * 1.判断当前楼层是否有电梯门开着
  * 2.当前楼层是否有电梯停靠
  * 3.如果之前有乘客被分配，依旧分配乘客给之前电梯
  */
  for( CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
  {
    //如果电梯就在请求所在层且电梯处于待机、人数未满时，之间返回电梯号
    if ( i->m_iCurFlr == reqIter->m_iReqCurFlr && ELVT_STOP(i->m_eCurState) && i->m_iCurPsgNum < MAX_INNER_PSG_NUM )
    {
      bestElvtIter = i;
      LOGA( "dispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
      return bestElvtIter;
    }
  }

  //如果当前层没有电梯停靠或电梯人满时，进行调度
  uint8 cnt = 0;

  for(CElevatorIterator i=m_elevatorVec.begin(); i != elvtIterEnd;  ++i )
  {  
    if ( i->m_iCurPsgNum < MAX_INNER_PSG_NUM )
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
    else
    {
      cnt++;
      uint16 psgid = reqIter->m_iPassagerID;
      
      sPassengerIterator psgIterEnd = m_passengerVec.end();
      for( sPassengerIterator  j=m_passengerVec.begin(); j != psgIterEnd;  ++j )
      {
        if (j->m_iPsgID == psgid )
        {
          j->m_iCurPlace = 0x5A;
          j->m_ePsgState = PSG_NONE;
        }
      }
      LOGA("dispatch:Elvt(%d) is full,Psg(%3d) is refused!\n",i->m_iElvtID,psgid);
    }
  }

  if ( cnt == m_elevatorVec.size() )
  {
    bestElvtIter = m_elevatorVec.end();
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
void CMWT::dispatch(sOutRequestIterator& reqIter, CElevatorIterator& elvtIter)
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
void CMWT::onClickOutBtn(sPassengerIterator& psg)
{
  sOutRequest out_req;
  sOutRequestIterator reqIter;
 
  //根据乘客外部请求生成申请实体
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

  sortElement( m_outReqVec, out_req );     //根据请求楼层进行排序
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
    //乘客的请求时间与系统时间比较
    if ( i->m_dPsgReqTime < gSystemTime )
    {
      if ( i->m_ePsgState == PSG_NONE )
      {
        onClickOutBtn(i);
        i->m_ePsgState = PSG_WAIT;
        i->m_iCurPlace = PSG_ARRIVE_PLACE;
        psgCnt++;
      } 
    }
   /* else
      break;*/
  }
}
