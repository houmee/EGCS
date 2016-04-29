/******************************************************************** 
修改时间:        2016/03/20 16:41
文件名称:        AlgInterface.cpp
文件作者:        huming 
=====================================================================
功能说明:        算法接口
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "AlgInterface.h"
#include "template_file.h"

/*********************************************************************
 * GLOBAL VALUE
 */
int    gUnusedRand = (srand((unsigned)time(NULL)),0);
double gSystemTime;        //系统时间
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
  m_outReqVec.reserve(MAX_OUT_REQUEST);     //为vector数组预留空间
  m_passengerVec.reserve(MAX_PSG_FLOW_NUM);
  m_eventVec.reserve(MAX_PSG_FLOW_NUM*2);
  m_elevatorVec.reserve(MAX_ELEVATOR_NUM);
  m_AlgFile.InitTools();
  m_eFlowType = RAND_FLOW;
  //m_eFlowType = UP_FLOW;
  //m_eFlowType = DOWN_FLOW;
} 

/********************************************************************
*  @name     : CAlgInterface::~CAlgInterface    
*  @brief    : 
*  @return   : 
********************************************************************/ 
CAlgInterface::~CAlgInterface()
{
  m_AlgFile.CloseTools(1);  //关闭文件流
}
 
/********************************************************************
*  @name     : CAlgInterface::testPsgFlow    
*  @brief    : 
*  @return   : void
********************************************************************/
void CAlgInterface::testPsgFlow()
{
  sPassengerInfo psg0 = {0, 1, 8, DIR_UP  , 0.1, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg0 );
  sPassengerInfo psg1 = {1, 2, 5, DIR_UP  , 0.3, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg1 );
  sPassengerInfo psg2 = {2, 4,10, DIR_UP  , 0.4, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg2 );
  sPassengerInfo psg3 = {3,10, 8, DIR_DOWN, 0.8, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg3 );
  sPassengerInfo psg4 = {4, 3, 6, DIR_UP  , 1.1, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg4 );
  sPassengerInfo psg5 = {5, 6, 5, DIR_DOWN, 1.3, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg5 );
  sPassengerInfo psg6 = {6, 2,10, DIR_UP  , 1.9, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg6 );
  sPassengerInfo psg7 = {7, 1, 3, DIR_UP  , 2.1, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg7 );
  sPassengerInfo psg8 = {8, 3, 1, DIR_DOWN, 2.3, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg8 );
  sPassengerInfo psg9 = {9, 4, 7, DIR_UP  , 3.1, 0.0, 0.0, PSG_NONE, PSG_ARRIVE_PLACE };
  insertElement( m_passengerVec, psg9 );

  //////////////////////////////////////////////////////////////////////////
  //测试脚本
  sPassengerIterator psgIterEnd = m_passengerVec.end();
  for( sPassengerIterator  i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
  {
    fprintf(m_AlgFile.m_OutputFilePtr, "generatePsgFlow:Psg(%2d)-ReqFlr(%2d)-DestFlr(%2d)-ReqTime(%f)\n",i->m_iPsgID,i->m_iReqCurFlr,i->m_iDestFlr,i->m_dReqTime);	
    fprintf(m_AlgFile.m_PsgFilePtr, "%2d %2d %2d %2d %.2f %.2f %.2f %2d %2d\n",
      i->m_iPsgID,i->m_iReqCurFlr,i->m_iDestFlr,i->m_eReqDir, i->m_dReqTime, i->m_dWaitTime, i->m_dAllTime, i->m_ePsgState, i->m_iCurPlace);	
  }
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

#if defined( TEST )
  if ( m_eFlowType == UP_FLOW )       //上高峰
  {
    for (uint16 i=0; i<psg_num; i++)
    {
      psg.m_iPsgID      = i;
      psg.m_iReqCurFlr  = 1;          //请求楼层为1
      psg.m_dWaitTime   = 0.0;
      psg.m_dAllTime    = 0.0;
      psg.m_ePsgState   = PSG_NONE;
      psg.m_iCurPlace   = PSG_ARRIVE_PLACE;
      //目的楼层是2~MAX
      psg.m_iDestFlr    = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+2;
      psg.m_iDestFlr    = psg.m_iDestFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iDestFlr;
      psg.m_eReqDir     = DIR_UP;
      psg.m_dReqTime    = generateRandTime();

      insertElement( m_passengerVec, psg );
    }
  }
  else if ( m_eFlowType == DOWN_FLOW )  //下高峰
  {
    for (uint16 i=0; i<psg_num; i++)
    {
      psg.m_iPsgID      = i;
      psg.m_iDestFlr    = 1;       //目的楼层为1
      psg.m_dWaitTime   = 0.0;
      psg.m_dAllTime    = 0.0;
      psg.m_ePsgState   = PSG_NONE;
      psg.m_iCurPlace   = PSG_ARRIVE_PLACE;
      //请求楼层是2~MAX
      psg.m_iReqCurFlr  = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+2;
      psg.m_iReqCurFlr  = psg.m_iReqCurFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iReqCurFlr;
      psg.m_eReqDir     = DIR_DOWN ;
      psg.m_dReqTime    = generateRandTime();

      insertElement( m_passengerVec, psg );
    }
  }
  else      //随机交通流
  {
    for (uint16 i=0; i<psg_num; i++)
    {
      psg.m_iPsgID = i;
      psg.m_dWaitTime = 0.0;
      psg.m_dAllTime = 0.0;
      psg.m_ePsgState = PSG_NONE;
      psg.m_iCurPlace = PSG_ARRIVE_PLACE;
      //请求楼层和目的楼层是1~MAX
      psg.m_iReqCurFlr = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+1;
      psg.m_iReqCurFlr = psg.m_iReqCurFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iReqCurFlr;

      do 
      {
        psg.m_iDestFlr = (int)floor(MAX_FLOOR_NUM * rand()/(double)(RAND_MAX+1))+1;
        psg.m_iDestFlr = psg.m_iDestFlr > MAX_FLOOR_NUM ? MAX_FLOOR_NUM : psg.m_iDestFlr;
      } while ( psg.m_iDestFlr == psg.m_iReqCurFlr );

      psg.m_eReqDir = (psg.m_iReqCurFlr > psg.m_iDestFlr) ? DIR_DOWN : DIR_UP;
      
      //到达时间需要按照一定规律
      psg.m_dReqTime = generateRandTime();

      insertElement( m_passengerVec, psg );
    }
  }
#else
  ////////////////////////////////////////////////////////////////////////
  //读取文件(测试用)
  for (uint16 i=0; i<psg_num; i++)
  {
    if (fscanf_s( m_AlgFile.m_PsgFilePtr, "%d %d %d %d %lf %lf %lf %d %d\n",
                 &psg.m_iPsgID,&psg.m_iReqCurFlr,&psg.m_iDestFlr,&psg.m_eReqDir, &psg.m_dReqTime, &psg.m_dWaitTime, &psg.m_dAllTime, &psg.m_ePsgState, &psg.m_iCurPlace) == EOF)
    {
      fprintf(m_AlgFile.m_PsgFilePtr,"Reading psg file meets error!\n");
      getchar();
      exit(1);
    }	
    insertElement( m_passengerVec, psg );
   }
  sortElement( m_passengerVec, psg); //根据请求时间排序
#endif

  //////////////////////////////////////////////////////////////////////////
  //测试脚本
  sPassengerIterator psgIterEnd = m_passengerVec.end();
  for( sPassengerIterator  i=m_passengerVec.begin(); i != psgIterEnd;  ++i )
  {
    fprintf(m_AlgFile.m_OutputFilePtr, "generatePsgFlow:Psg(%2d)-ReqFlr(%2d)-DestFlr(%2d)-ReqTime(%f)\n",i->m_iPsgID,i->m_iReqCurFlr,i->m_iDestFlr,i->m_dReqTime);	

#if defined (TEST)   
    fprintf(m_AlgFile.m_PsgFilePtr, "%2d %2d %2d %2d %.2f %.2f %.2f %2d %2d\n",
      i->m_iPsgID,i->m_iReqCurFlr,i->m_iDestFlr,i->m_eReqDir, i->m_dReqTime, i->m_dWaitTime, i->m_dAllTime, i->m_ePsgState, i->m_iCurPlace);	
#endif 
  }
}


/********************************************************************
*  @name     : CAlgInterface::generateRandTime    
*  @brief    : 
*  @return   : float
********************************************************************/ 
double CAlgInterface::generateRandTime()
{
  //TODO:泊松分布产生到达时间
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
