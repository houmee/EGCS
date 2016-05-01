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
void CMWT::Core_Main()
{
  generateElevatorVec();      //产生电梯群
  generatePsgFlow();          //产生乘客交通流
  //testPsgFlow();

  while ( !isAlgFinished() )  //所有人是否都已经到达目的层        
  {
    gSystemTime += SYSTEM_TIME_STEP;

    fprintf(m_AlgFile.m_OutputFilePtr, "\n++++++++++++++++Core_Main:+++%.2f++++++++++++++++\n",gSystemTime);

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
      printf("Run fail!\n");
      getchar();
      exit(1);
    }
  }

  printf("Run successfully!\n");
  fprintf(m_AlgFile.m_OutputFilePtr, "Run successfully!\n");
  getchar();
}

/********************************************************************
*  @name     : CMWT::schedule    
*  @brief    : 
*  @return   : void
********************************************************************/ 
void CMWT::schedule()
{
  sOutRequestIterator reqIterEnd = m_outReqVec.end();
  for( sOutRequestIterator i=m_outReqVec.begin(); i != reqIterEnd;  ++i )
    dispatch(i,fitness(i));       //处理每一个请求
  
  if ( m_outReqVec.size() > 0 )   //处理完所有请求，清除请求列表
    m_outReqVec.clear(); 
  
  gBatch++;
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
      fprintf(m_AlgFile.m_OutputFilePtr, "dispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
      return bestElvtIter;
    }
  }

  //如果当前层没有电梯停靠或电梯人满时，进行调度
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

  fprintf(m_AlgFile.m_OutputFilePtr, "dispatch:OutReq-CurFlr(%2d)--->Elevator(%d)\n",reqIter->m_iReqCurFlr,bestElvtIter->m_iElvtID);	
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
  sRunItem runInd;
  sRunItemIterator indIter;

  runInd.m_eReqType = OUT_REQ;
  runInd.m_iDestFlr = reqIter->m_iReqCurFlr;
  if ( reqIter->m_iReqCurFlr == elvtIter->m_iCurFlr )
    runInd.m_eElvDir = DIR_NONE;
  else
    runInd.m_eElvDir  =( reqIter->m_iReqCurFlr > elvtIter->m_iCurFlr ) ? DIR_UP : DIR_DOWN;

  elvtIter->insertRunTableItem( runInd );       //将外部呼号请求推送给电梯
  elvtIter->showElevator();
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

  //fprintf(m_AlgFile.m_OutputFilePtr, "onClickOutBtn:Psg(%2d)-ReqCurFlr(%2d)-SysTime(%.2f)\n",psg->m_iPsgID,psg->m_iReqCurFlr,gSystemTime);	
  
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
        psgCnt++;
      } 
    }
    else
      break;
  }
}
