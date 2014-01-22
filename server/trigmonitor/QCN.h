//  QCN class

#ifndef QCN_H
#define QCN_H

#include <vector>
#include <string>
#include <iostream>

#ifdef ONLINE
#include "boinc_db.h"
#include "sched_config.h"
#endif 

#include "QCNTrigger.h"


using namespace std;

//forward decleartion
//class QCNTrigger;
class QCNEvent;
class QCNSqlQuery;

#ifdef ONLINE
class DB_CONN;
struct SCHED_CONFIG;
#endif 

class Crust2;

class QCN
{

public :
    enum eOutput { OUT_EVENT, OUT_STATION, OUT_INTENSITY_MAP, OUT_CONT_TIME, OUT_CONT_LABEL, OUT_TIME_SCATTER };
    enum CalMagMethod  { TYPE_I=1, TYPE_II=2, TYPE_III=3, TYPE_CI=101, TYPE_CII=102, TYPE_CIII=103};
    //constructor
    QCN(const Crust2& crust2);
    //destructor
    ~QCN();
#ifdef OFFLINE
    bool getTriggersOffline(const string& fname, const double fetch_interval);
    bool getTriggersOffline(long int t1, long int t2, int fetch_interval, int query_interval);
#endif

#ifdef ONLINE
    void execute();
#endif
    void detectEvent();

    void setSleepInterval(double dSleepInterval){
        if (dSleepInterval < 0) {
            _dSleepInterval = TRIGGER_SLEEP_INTERVAL;
        } else {
            _dSleepInterval = dSleepInterval;
        }
    }

    /////////////////////SET METHODS////////////////////
    void setTriggerTimeInterval(int iTriggerTimeInterval){
        if (iTriggerTimeInterval < 0) {
            _iTriggerTimeInterval = TRIGGER_TIME_INTERVAL;
        } else {
            _iTriggerTimeInterval = iTriggerTimeInterval;
        }
    }


    void setTriggerDeleteInterval(int iTriggerDeleteInterval){
        _iTriggerDeleteInterval = iTriggerDeleteInterval;
    }
    
    
    void setEVENT_URL_BASE(const string& str){
         EVENT_URL_BASE = str;
    }
    
    void setEVENT_PATH(const string& str){
         EVENT_PATH = str;
    }
    
    void setGMT_MAP_PHP(const string& str){
         GMT_MAP_PHP = str;
    }
    
    void setEMAIL_PATH(const string& str){
         EMAIL_PATH = str;
    }
    
    void setEMAIL_DIR(const string& str){
         EMAIL_DIR = str;
    }
    
    void setEMAIL_INC(const string& str){
         EMAIL_INC = str;
    }
    
    void addTrigger(const QCNTrigger& trig){
         _vt.push_back(trig);
    }     

    
     void setMagCalc( const int type ){
          _methodToCalcMag = type;
     }


     void setMaxEventDuration(int t_max){
          T_MAX_EVENT = t_max;
     }

     void setMaxTriggerDuration(int t_max){
          T_MAX_TRIGGER  = t_max;
     }


     void setCorrSpaceRange(double d_max){
          D_MAX_TRIGGER = d_max;
     }


     void  setEventTriggerMaxDist(double dlat, double dlon){
          D_MAXLAT_EVENT_TRIGGER = dlat;
          D_MAXLON_EVENT_TRIGGER = dlon;
     }

     void  setMinCorrCount(int  c_cnt_min){
          C_CNT_MIN = c_cnt_min;
     }

     //limit locate algorithm to c_cnt_max correlatin
     void  setMaxCorrCount(int  c_cnt_max){
          C_CNT_MAX = c_cnt_max;
     }

     void  setMaxMisfitTime(double tmax){
           MAX_MISFIT_TIME = tmax;
     }
     
     void  setMinR2(double r2_min){
           R2_MIN = r2_min;
     }
private :

    void  init();

#ifdef ONLINE
    int   DBOpen();
    void  DBClose();
#endif
    bool eventLocate(const bool bEventFound,  QCNEvent& e, const int ciOff);

    float ang_dist_km(const float lon1, const float lat1, const float lon2, const float lat2);
    float correlate(const vector<float>& datx, vector<float>& daty, const int ndat);
    float stdDev(const vector<float>& dat, const int ndat, const float dat_ave);   
    float stdDev(const vector<float>& dat, const int ndat);
    float average(const vector<float>& dat, const int ndat);

    void  estimateMagnitudeI   (QCNEvent& e, const int ciOff);
    void  estimateMagnitudeII  (QCNEvent& e, const int ciOff);  
    void  estimateMagnitudeIII (QCNEvent& e, const int ciOff);
    void  estimateMagnitudeCI  (QCNEvent& e, const int ciOff);
    void  estimateMagnitudeCII (QCNEvent& e, const int ciOff);  
    void  estimateMagnitudeCIII(QCNEvent& e, const int ciOff);

    const vector<double> getTimeAtoB(const vector<float>& A, const vector<float>& B, const float dis);
#ifdef ONLINE
    void  updateQuake(const bool bInsert, QCNEvent& e, const int ciOff);
    bool  sendTriggerFileRequest(const char* strFile, const char* strResult, const int hostid, const char* strDB);
    void  php_event_email(const QCNEvent& e, const char* epath);
    int getTriggers();
    void  do_delete_trigmem();
    long int getMySQLUnixTime();
#endif
    int   intensityMap(const bool bInsertEvent, QCNEvent& e, const int ciOff);
    int   intensityMapGMT(const char* epath);

    const Crust2&  _crust2;
    const int ID_USB_SENSOR_START;
    int C_CNT_MIN;
    int C_CNT_MAX;
    const int N_LONG;
    int T_MAX_TRIGGER;       //maximum time the triggers are correlated
    int T_MAX_EVENT;         //maximum event duration
    const double Vs;         // S wave velocity (km/s)
    const double Vp;         // P wave velocity (km/s)
    double D_MAX_TRIGGER;       // Maximum distance between triggers in km
    double D_MAXLON_EVENT_TRIGGER;
    double D_MAXLAT_EVENT_TRIGGER; 
    double MAX_MISFIT_TIME;
    double R2_MIN;

    const int MAX_PATH;
    const double TRIGGER_SLEEP_INTERVAL;
    const int    TRIGGER_TIME_INTERVAL;
    string  EVENT_URL_BASE;
    string  EVENT_PATH;
    string  PHP_CMD;
    string GMT_MAP_PHP;
    string CSHELL_CMD;
    string EMAIL_PATH;
    string EMAIL_DIR;
    string EMAIL_INC;

    vector<QCNTrigger>  _vt;
    vector<QCNEvent>    _ve;
    double _dSleepInterval;
    int    _iTriggerTimeInterval;
    int    _iTriggerDeleteInterval;
    double _curRefTime;
    double _curRefTimeZero;
    time_t  _tBeg;
    time_t  _tEnd;
    int _methodToCalcMag;
    QCNSqlQuery* _qcnSQL;
    string _hostName;
    string _userID;
    string _passwd;
    string _db;
    //      DB_CONN       _boincDB;
    //       DB_CONN       _trigmemDB;
    //      SCHED_CONFIG  _config;

};


#endif

