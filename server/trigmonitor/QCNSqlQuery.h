//  QCNSqlQuery class

#ifndef QCNSqlQuery_H
#define QCNSqlQuery_H

#include <string>
#include <vector>
#include <mysql.h>

using namespace std;
class FFError
{
public:
    std::string    Label;

    FFError( ) { Label = (char *)"Generic Error"; }
    FFError( char *message ) { Label = message; }
    ~FFError() { }
    inline const char*   GetMessage  ( void )   { return Label.c_str(); }
};



class QCNSqlQuery
{

public :

    //enum { VERIFICATION_TEST = 0,DIFFUSION_TEST = 1, WAVE_CURRENT = 2, DEPTH_INDUCED = 3, WAVE_GROWTH = 4};
    //constructors
    explicit QCNSqlQuery(
             const string& host_name, const string& user_id, 
             const string& passwd, const string&
             );
    ~QCNSqlQuery();

    /*  Data structure for events. To be used with QCN location
        program. This structure written by Battalgazi Yildirim (August 2013) - 
        Contact: yildirim@stanford.edu                                                  */

   int query(long int t_start, long int t_end);
   vector<double> longitude;
   vector<double> latitude;
   vector<double> magnitude;
   vector<int> hostid; 
   vector<string> file;
   vector<double> time_trigger;
   vector<double> time_received;
   vector<double> significance;
   vector<float> mxy1p;
   vector<float> mz1p;
   vector<float> mxy1a;
   vector<float> mz1a;
   vector<float> mxy2a;
   vector<float> mz2a;
   vector<float> mxy4a;
   vector<float> mz4a;




private :

    void  init();
    void  order_data();
//    bool  comp_func(int i, int j){
//        return (i<j);
//    }
    MYSQL* _SQLConnection;
    MYSQL* _SQLConRetVal;
    MYSQL_RES* _sqlResult;

    string _hostName;
    string _userID;
    string _password;
    string _DB;

    vector<int> _indx; //to trace after the reordering by time_received



};


#endif

