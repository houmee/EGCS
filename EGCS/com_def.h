/******************************************************************** 
�޸�ʱ��:        2016/03/20 16:35
�ļ�����:        com_def.h
�ļ�����:        huming 
=====================================================================
����˵��:        �����ļ����� 
--------------------------------------------------------------------- 
�汾���:        0.1
--------------------------------------------------------------------- 
����˵��:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#ifndef		COM_DEF_H
#define		COM_DEF_H

#include "com_type.h"
#include <stdarg.h>
//#define NDEBUG 
#include <assert.h>

/*********************************************************************
 * DEFINE
 */
#define ELVT_UP(x)    ( x==UP_ACC   || x==UP_CONST   || x==UP_DEC   || x== UP_PAUSE )
#define ELVT_DOWN(x)  ( x==DOWN_ACC || x==DOWN_CONST || x==DOWN_DEC || x== DOWN_PAUSE )
#define ELVT_STOP(x)  ( x==IDLE || x==UP_PAUSE || x==DOWN_PAUSE )

#define LOGE(fm, ...) {fprintf(m_AlgFile.m_OutputFilePtr,fm,__VA_ARGS__);printf(fm,__VA_ARGS__);}

/*********************************************************************
 * GLOBAL VALUE
 */
extern double gSystemTime;

/*********************************************************************
 * CONSTANTS
 */
#define ELEVATOR_HEIGHT			28				//��������߶�
#define MAX_OUT_REQUEST     10        //һ�����ݽ�������������Ŀ
#define MAX_PRIORITY_NUM	  100				//���������б�������ȼ�
#define MAX_PSG_FLOW_NUM    200       //�˿����������
#define SYSTEM_TIME_STEP    1       //ϵͳʱ���������(s)
#define GRAVITY_ACCELERATE  9.7       //�������ٶ�
#define MAX_WAIT_TIME       1000
#define MAX_ENERGY          999999
#define PSG_ARRIVE_PLACE    0x7F

//��������
#define ONE_FLOOR_TIME      2         //��������һ��¥ʱ��(s)
//�Ӽ�������
#define CONST_SPEED_TIME    1.33      //����ʱ��(s)
#define ACCELERATE_TIME     1         //����ʱ��(s)
#define DECELERATE_TIME     1         //����ʱ��(s)
#define REMAIN_GAP_TIME     1.67      //һ����Ӽ���ʣ����������ƶ���ʱ��s)
#define CONST_SPEED_LENGTH  2         //�����ƶ�����(m)
#define ACCELERATE_LENGTH   0.5       //�����ƶ�����(m)
#define DECELERATE_LENGTH   0.5       //�����ƶ�����(m)
#define REMAIN_GAP_LENGTH   2.5       //�Ӽ���ʣ�����(m)

//���в���
#define START_STOP_ENERGY   22500     //��ͣ�ܺ�(J)
#define CAR_WEIGHT          800       //��������(kg)
#define COUNT_WEIGHT        900       //��������(kg)
#define NET_CAR_WEIGHT      -100      //���������þ�������(kg)
#define PSG_AVG_WEIGHT      65        //�˿�ƽ������(kg)
#define MAX_INNER_PSG_NUM   10        //�������������
#define MAX_FLOOR_NUM				10				//¥����
#define MAX_ELEVATOR_NUM		2				  //���ݣ����壩��
#define FLOOR_HEIGHT        3         //¥���߶�(m)
#define RUN_SPEED           1.5       //���������ٶ�(m/s)
#define ACCELERATE_SPEED    1         //���ݼ��ٶ�(m/s2)
#define JERK_SPEED          1.8       //���ݼӼ��ٶ�(m/s3)
#define WEIGHT_CAPACITY     1000      //�����������(kg)
#define OPEN_CLOSE_TIME			2				  //������ʱ��(s)
#define PSG_ENTER_TIME			1				  //�˿ͽ���/�뿪ʱ��ʱ��(s)
    

//#define  TEST
/*********************************************************************
 * ENUMS
 */
enum FlowType			          //�˿�������
{
  UP_FLOW,
  DOWN_FLOW,
  RAND_FLOW
};

enum ReqType			          //������������
{
  OUT_REQ,
  IN_REQ
};

enum ElvtSchedState			//�����������״̬
{
  ACTIVE,
  SUSPEND
};

enum ElvtRunState			//������������״̬
{
	IDLE,				        //����״̬0
  UP_ACC,             //���ϼ���1
  UP_CONST,           //��������2
  UP_DEC,             //���ϼ���3
  UP_PAUSE,			      //������ͣ4
  DOWN_ACC,           //���¼���5
  DOWN_CONST,         //��������6
  DOWN_DEC,           //���¼���7
	DOWN_PAUSE,			    //������ͣ8
	MALFUNCTION				  //����9

  //IDLE,				        //����״̬
  //UP_RUN,             //��������
  //UP_PAUSE,			      //������ͣ
  //DOWN_RUN,           //��������
  //DOWN_PAUSE,			    //������ͣ
  //MALFUNCTION				  //����
};

enum RunDir			//�������з������ƣ�
{
  DIR_NONE,				    //�޷�����0
	DIR_UP,					    //����1
  DIR_DOWN				    //����2
};

enum EvtType			//�¼�����
{
	EVT_PASSAGER_IN,		//�˿ͽ���
	EVT_PASSAGER_OUT    //�˿��뿪
};

enum PsgState			//�˿�״̬
{
  PSG_NONE,
  PSG_WAIT,		    //�ȴ�
  PSG_TRAVEL,     //����
  PSG_ARRIVE      //����Ŀ�Ĳ�
};

/*********************************************************************
 * TYPEDEFS
 */
typedef struct TargetVal
{ 
  double m_fWaitTime;
  double m_fEnergy;
} sTargetVal;

typedef struct OutRequest
{ 
  uint16		    m_iPassagerID;      //����˿ͱ��
	RunDir     	  m_eReqDir;          //�ⲿ������
	uint8		      m_iReqCurFlr;       //�ⲿ��������¥���
  uint8		      m_iReqDestFlr;      //�ⲿ����Ŀ��¥���
	double		    m_dReqTime;         //�ⲿ����ʱ��
  uint16        m_iReqNum;          //��ͬ����ͬһʱ���������
  uint8         m_iReqElvtID;       //��������ĵ���

  bool operator <(const OutRequest& rhs) const // ��������ʱ����д�ĺ���
  {
    return (m_iReqCurFlr < rhs.m_iReqCurFlr);
  }

  bool operator >(const OutRequest& rhs) const // ��������ʱ����д�ĺ���
  {
    return (m_iReqCurFlr > rhs.m_iReqCurFlr);
  }

  bool operator == (const OutRequest& rhs) const
  {
    return ( m_iReqCurFlr == rhs.m_iReqCurFlr && m_eReqDir == rhs.m_eReqDir );
  }
} sOutRequest;

typedef struct  Passenger
{ 
  uint16	  m_iPsgID;          //�˿����
  uint8		  m_iPsgCurFlr;      //��������¥���
  uint8		  m_iPsgDestFlr;     //Ŀ��¥��
  RunDir    m_ePsgReqDir;      //�ⲿ������
  double    m_dPsgReqTime;     //����ʱ��
  double    m_dWaitTime;       //�ȴ�ʱ��
  double	  m_dAllTime;        //����ʱ��
  PsgState  m_ePsgState;       //�˿�״̬
  uint8     m_iCurPlace;       //�˿�����λ��

  bool operator <(const Passenger& rhs) const // ��������ʱ����д�ĺ���
  {
    return (m_dPsgReqTime < rhs.m_dPsgReqTime);
  }

  bool operator >(const Passenger& rhs) const // ��������ʱ����д�ĺ���
  {
    return (m_dPsgReqTime > rhs.m_dPsgReqTime);
  }

  bool operator == (const Passenger& rhs) const
  {
    return (m_iPsgID == rhs.m_iPsgID);
  }

} sPassengerInfo;

typedef struct
{
  uint8		m_iEvtid;          //�¼����
	EvtType m_eType;           //�¼�����
	uint8		m_iPsgID;          //�˿����
	uint16  m_iEvtTime;        //�¼�ʱ��
}sEvent;

typedef struct RunItem
{ 
  ReqType     m_eReqType;        //���������Դ(�ڲ����ⲿ)
  int8        m_iPriority;       //�������ȼ�
  RunDir		  m_eElvDir;         //���з���
  uint8       m_iDestFlr;        //����Ŀ�Ĳ�
  sTargetVal  m_sTarVal;

  bool operator <(const RunItem& rhs) const // ��������ʱ����д�ĺ���
  {
    return (m_iPriority < rhs.m_iPriority);
  }
  bool operator >(const RunItem& rhs) const // ��������ʱ����д�ĺ���
  {
    return (m_iPriority > rhs.m_iPriority);
  }

  bool operator == (const RunItem& rhs) 
  {
    return ( m_iDestFlr == rhs.m_iDestFlr && m_eElvDir == rhs.m_eElvDir );
  }
}sRunItem;

/*********************************************************************
*********************************************************************/
#endif /* COM_DEF_H */