#include <iostream>
#include <cassert>
#include <sstream>
#include <map>
#include <list>
#include <algorithm>

#include "QCNSqlQuery.h"


using namespace std;

bool  comp_func(int i, int j){
     return (i<j);
}

class comp_map{
public:
    bool operator() (int lhs, int rhs) const
    {return lhs<rhs;}
};

//Constructors
QCNSqlQuery::QCNSqlQuery(
             const string& host_name, const string& user_id, 
             const string& passwd, const string& db
             ) :
_SQLConnection(NULL),
_SQLConRetVal(NULL),
_hostName(host_name),
_userID(user_id),
_password(passwd),
_DB(db)
{
    init();
}

//Destructor
QCNSqlQuery::~QCNSqlQuery()
{
    //close database connection
    mysql_close(_SQLConnection);
 
}

/////////////////////////////////PUBLIC METHODS::///////////////////////////////////////////
int
QCNSqlQuery::query(long int t_start, long int t_end)
{
   //convert the times to string
   ostringstream tStart;
   ostringstream tEnd;
   tStart << t_start;
   tEnd   << t_end;
   // # of Columns (mFields) in the latest results set
   //int numFields = mysql_field_count(_SQLConnection);
   //cout << "numfilds = " << numFields << endl;

   try{
       //trigger_id, dis
       string varList = " longitude,latitude,magnitude,hostid,"
                        "file,time_trigger,time_received,"
                        "significance,mxy1p,mz1p,mxy1a,mz1a,"
                        "mxy2a,mz2a,mxy4a,mz4a ";

       string tb_1 = " sensor_archive.qcn_trigger ";
       string tb_2 = " continual_archive.qcn_trigger ";
       string tb_3 = " sensor.qcn_trigger ";
       string tb_4 = " continual.qcn_trigger ";

       string whereStr = " WHERE  time_trigger BETWEEN " + 
                        tStart.str() + " AND " +  tEnd.str() +  
                       " AND varietyid = 0 AND qcn_sensorid >= 100"
                       " AND numreset < 10";
       string limitStr = " ORDER BY time_trigger ASC ";
       string selectStr = " SELECT" + varList + "  FROM ";
       string SQLString = "(" + selectStr +  tb_2 +  whereStr + ")" + 
                          " UNION " + 
                          "(" + selectStr +  tb_1 +  whereStr + ")" + 
                          " UNION " + 
                          "(" + selectStr +  tb_3 +  whereStr + ")" +   
                          " UNION " + 
                          "(" + selectStr +  tb_4 +  whereStr + ")" +
                           limitStr; 

        int sqlStatus = mysql_query( 
              _SQLConnection, SQLString.c_str() 
            );
        _sqlResult = NULL;
        if (sqlStatus){
            throw FFError( (char*) mysql_error(_SQLConnection) );
        }else{
           // Get the Result Set
            if(!(_sqlResult = mysql_store_result(_SQLConnection))){
                cout << " no result is returned" << endl;
            } 
        }

#if 0
        MYSQL_FIELD* sqlFields = mysql_fetch_fields(_sqlResult);
        // Returns the number of columns in a result set specified
        int numFields = mysql_num_fields(_sqlResult);
        for(int jj=0; jj < numFields; jj++) {
            cout << sqlFields[jj].name <<  endl;
        }
#endif

#if 0    
        MYSQL_ROW sqlRow;
        while( (sqlRow = mysql_fetch_row(_sqlResult)) != NULL){
            cout << sqlRow[5] << "   " << sqlRow[6] << "   " 
                 << sqlRow[7] << "   " << sqlRow[8] << endl; 
        }
#endif
         //order data
        order_data();
        
        if(_sqlResult){
            mysql_free_result(_sqlResult);
            _sqlResult = NULL;
        }
   } catch( FFError e ) {
        cout << e.Label << endl;
        mysql_close(_SQLConnection);
        return 1;
    }
    
   
    return 0;


}



////////////////PRIVATE METHODS//////////////////////////////
void
QCNSqlQuery::init()
{

    _SQLConnection = mysql_init( NULL );
    try {
#if 0
         cout << _hostName.c_str() << endl;
         cout << _userID.c_str() << endl;
         cout << _password.c_str() << endl;
         cout << _DB.c_str() << endl;
#endif
        _SQLConRetVal = mysql_real_connect( _SQLConnection,
                                            _hostName.c_str(), 
                                            _userID.c_str(), 
                                            _password.c_str(), 
                                            _DB.c_str(), 
                                            0, 
                                            NULL,
                                            0 );
         
        if (_SQLConRetVal == NULL)
            throw FFError( (char*) mysql_error(_SQLConnection) );
#if 0   
        cout << "MySQL Connection Info: "   
             <<   mysql_get_host_info(_SQLConnection)   
             << endl;
        cout << "MySQL Client     Info: "
             <<  mysql_get_client_info()                
             << endl;
        cout << "MySQL Server     Info: " 
             <<  mysql_get_server_info(_SQLConnection) 
             << endl;
#endif
    }
    catch ( FFError& e )
    {
        cout << e.Label << endl;
    }
     

}

void
QCNSqlQuery::order_data()
{
   //point to the head row
    mysql_data_seek(_sqlResult,0);
    //first get row size
    unsigned long row_size = mysql_num_rows(_sqlResult);
    //tempororay time_recived
    vector<int> time(row_size,0);
    multimap<int, int, comp_map>  timeToIndexMap;
    for(unsigned long i = 0; i < row_size; i++ ){
        MYSQL_ROW row = mysql_fetch_row(_sqlResult);
        istringstream ss(row[6]);
        ss >> time[i];
        timeToIndexMap.insert(pair<int,int>(time[i],i)); 
    }

    //sorting time vector (the oldest data will be first elements)
    sort(time.begin(), time.end(), comp_func);
    //allocate _index
    _indx.resize(row_size);
    int cnt = 0;
    //loop over to get indexing
    for(unsigned long i = 0; i < row_size; i++ ){
        multimap<int,int>::iterator it, itlow, itup;
        itlow = timeToIndexMap.lower_bound(time[i]);
        itup  = timeToIndexMap.upper_bound(time[i]);
        for(it=itlow; it!=itup; ++it){
             _indx[cnt] = it->second;
             cnt++;
             timeToIndexMap.erase(it);
        }
    }
 
    //resize the vectors 
    longitude.resize(row_size, 0.0); 
    latitude.resize (row_size, 0.0); 
    magnitude.resize(row_size, 0.0);
    hostid.resize   (row_size, 0);
    file.resize     (row_size);
    time_trigger.resize (row_size,0.0);
    time_received.resize(row_size,0.0);
    significance.resize(row_size,0.0);
    mxy1p.resize(row_size,0.0);
    mz1p.resize (row_size,0.0);
    mxy1a.resize(row_size,0.0); 
    mz1a.resize (row_size,0.0); 
    mxy2a.resize(row_size,0.0); 
    mz2a.resize (row_size,0.0); 
    mxy4a.resize(row_size,0.0); 
    mz4a.resize (row_size,0.0); 

    //assign the vectors
    vector<unsigned long int> traceDeleteElement;
    for(unsigned long i = 0; i < row_size; i++){
        //fetch the dta
        mysql_data_seek(_sqlResult,_indx[i]);
        MYSQL_ROW row = mysql_fetch_row(_sqlResult);
        //we don't want to add if any of 0-7 becomes NULL
        bool isValid = row[0] && row[1] && row[2] && row[3] &&
                       row[4] && row[5] && row[6] && row[7]; 
 
        if ( isValid ){ 
            {istringstream ss(row[0]); ss >> longitude[i];} 
            {istringstream ss(row[1]); ss >> latitude[i];} 
            {istringstream ss(row[2]); ss >>magnitude[i];}  
            {istringstream ss(row[3]); ss >>hostid[i];} 
            {istringstream ss(row[4]); ss >>file[i];} 
            {istringstream ss(row[5]); ss >>time_trigger[i];}
            {istringstream ss(row[6]); ss >>time_received[i];}
            {istringstream ss(row[7]); ss >>significance [i];}
         } else {
#if 0
             traceDeleteElement.push_back(i);
#endif 
             cout << " null element found " << endl;
         }
        //we can go with the default values without worrying NULL
        //for the following variables
        if (row[8])
            {istringstream ss(row[8]); ss >>mxy1p[i];}   
        if (row[9])
            {istringstream ss(row[9]); ss >>mz1p[i];} 
        if (row[10])
            {istringstream ss(row[10]); ss >>mxy1a[i];} 
        if (row[11])
            {istringstream ss(row[11]); ss >>mz1a[i];} 
        if (row[12])
            {istringstream ss(row[12]); ss >>mxy2a[i];} 
        if (row[13])
            {istringstream ss(row[13]); ss >>mz2a[i];} 
        if (row[14])
            {istringstream ss(row[14]); ss >>mxy4a[i];} 
        if (row[15])
            {istringstream ss(row[15]); ss >>mz4a[i];} 
    
       // cout << " time_receved = " << long(time_received[i]) <<
       //         " hostid = " << hostid[i] << endl; 
     }

#if 0
     //erase elements have NULL values
     for(size_t i = 0; i <  traceDeleteElement.size(); i++){
           unsigned long indx = traceDeleteElement[i]; 
          longitude.erase( longitude.begin() + indx);
          latitude.erase ( latitude.begin()  + indx);
          magnitude.erase( magnitude.begin() + indx);
          hostid.erase   ( hostid.begin()    + indx);
          file.erase     ( file.begin()      + indx);
          time_trigger.erase ( time_trigger.begin()  + indx);
          time_received.erase( time_received.begin() + indx); 
          significance.erase ( significance.begin()  + indx);
          mxy1p.erase( mxy1p.begin() + indx );
          mz1p.erase ( mxy1p.begin() + indx );
          mxy1a.erase( mxy1a.begin() + indx );
          mz1a.erase ( mxy1a.begin() + indx );
          mxy2a.erase( mxy2a.begin() + indx );
          mz2a.erase ( mxy2a.begin() + indx );
          mxy4a.erase( mxy4a.begin() + indx );
          mz4a.erase ( mxy4a.begin() + indx );
     }
#endif 

#if 0
    //test  if they are ordered
    for(unsigned long i = 0; i < row_size; i++ ){
         mysql_data_seek(_sqlResult,_indx[i]);
         MYSQL_ROW row = mysql_fetch_row(_sqlResult);
         int val;
         istringstream ss(row[6]);
         ss >> val;
         assert( time[i] == val );
         //cout <<  time[i] -val << endl;
    }
         
#endif            

 
}
