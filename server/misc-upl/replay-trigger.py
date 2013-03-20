#! /usr/bin/env python

# this program will feed the trigmem_test.qcn_trigger_memory table fake triggers (usually on qcn-data but could be qcn-web)
# it can "mimic" past events such as previously detected QCN or USGS earthquakes from historical trigger data

# this will be run manually for now, can be extended for other areas of course

# output file is:  http://qcn-upl.stanford.edu/trigger/job/qcn_scedc.zip

#contacts:
#Ellen Yu
#eyu@gps.caltech.edu
#
#Aparna Bhaskaran
#aparnab@gps.caltech.edu
#

# CMC note -- need to install 3rd party MySQLdb libraries for python
import sys, os, math, traceback, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
#from zipfile import ZIP_STORED
from time import strptime, mktime, sleep
#from qcnutil import getSACMetadata
#from qcnutil import getFanoutDirFromZip
from optparse import OptionParser

global PROGNAME
global DBHOST 
global DBUSER
global DBPASSWD
global DBNAME
global DB_TRIGMEM
global SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT

PROGNAME = "replay-trigger.py"
DBHOST = "localhost"
DBUSER = "qcn"
DBPASSWD = "qcntest"
DBNAME = "sensor"
DB_TRIGMEM = "trigmem_test"

global DBNAME_CONTINUAL
global DBNAME_JOB
global IS_ARCHIVE
global sqlQuery
global ZIP_CMD
global UNZIP_CMD
global DATE_MIN_ORIG
global DATE_MAX_ORIG
global DATE_MIN
global DATE_MAX
global LAT_MIN
global LAT_MAX
global LNG_MIN
global LNG_MAX
global DOWNLOAD_WEB_DIR

DOWNLOAD_WEB_DIR = ""

def procRequest(dbconn):

  sqlQuery = """select
    m.mydb, m.id, m.hostid, 
    from_unixtime(m.time_trigger) time_trig, FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6) utime_trig, from_unixtime(m.time_received) time_rec, 
    from_unixtime(m.time_sync) time_synchronized, m.sync_offset, m.magnitude,  
    m.significance,  m.latitude, m.longitude, 
    m.file, m.numreset, m.dt,
    case m.varietyid when 0 then 'N' when 1 then 'P' when 2 then 'C' end trigtype, m.hostipaddrid,m.geoipaddrid,
    s.description sensor, IFNULL(a.description,'') alignment,IFNULL(m.levelvalue,'') level, 
    IFNULL(l.description,'') level_type, from_unixtime(q.time_utc) quake_time, q.depth_km quake_depth_km, 
    q.latitude quake_lat, q.longitude quake_lon, q.magnitude, 
    q.id, q.description quake_desc, q.guid
from
(
select 'Q' mydb, 
t.id, t.qcn_quakeid, t.hostid, t.time_trigger, t.time_received, t.time_sync, t.sync_offset, t.magnitude, t.significance, t.latitude, t.longitude,
t.file, t.numreset, t.alignid, t.levelid, t.levelvalue, t.qcn_sensorid,
t.dt, t.varietyid,t.hostipaddrid,t.geoipaddrid
from %s.qcn_trigger t
where time_trigger between %d and %d
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between %f and %f and longitude between %f and %f
UNION
select 'C' mydb, 
tt.id, tt.qcn_quakeid, tt.hostid, tt.time_trigger, tt.time_received, tt.time_sync, tt.sync_offset, tt.magnitude, tt.significance, tt.latitude, tt.longitude,
tt.file, tt.numreset, tt.alignid, tt.levelid, tt.levelvalue, tt.qcn_sensorid,
tt.dt, tt.varietyid,tt.hostipaddrid,tt.geoipaddrid
from %s.qcn_trigger tt
where time_trigger between %d and %d
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between %f and %f and longitude between %f and %f
) m
LEFT JOIN sensor.qcn_sensor s ON m.qcn_sensorid = s.id
LEFT OUTER JOIN sensor.qcn_align a ON m.alignid = a.id
LEFT OUTER JOIN sensor.qcn_level l ON m.levelid = l.id
LEFT OUTER JOIN sensor.qcn_quake q ON q.id = m.qcn_quakeid
where m.qcn_sensorid=s.id
order by time_trigger,hostid"""  \
  % ( \
      DBNAME, DATE_MIN, DATE_MAX, LAT_MIN, LAT_MAX, LNG_MIN, LNG_MAX, \
      DBNAME_CONTINUAL, DATE_MIN, DATE_MAX, LAT_MIN, LAT_MAX, LNG_MIN, LNG_MAX  \
    )

  strHeader = "db, triggerid, hostid, time_trig_utc, time_trig_micros, time_received, time_sync, sync_offset, magnitude, significance, latitude, longitude, file, " +\
     "numreset, dt, trig type, hostipaddrid, geoipaddrid, sensor, alignment, level_value, level_type, usgs_quake_time, quake_depth_km, quake_lat, quake_lon, quake_mag, quake_id, quake_desc, quake_guid\n"

  tmpdir = tempfile.mkdtemp()
  myCursor = dbconn.cursor()

  myCursor.execute(sqlQuery)

  # get the resultset as a tuple
  result = myCursor.fetchall()
  numbyte = 0
  myzipout = None
  errlevel = 0

  try:
 
   # open a zip output file - allow zip64 compression for large (>2GB) files
   errlevel = 1
   #print zipoutpath, "  ", 'w', "  ", str(ZIP_STORED), "  ", str(True)
   myzipout = zipfile.ZipFile(zipoutpath, "w", ZIP_STORED, True)

   # iterate through resultset
   curdir = os.getcwd()   # save current directory and go to the temp dir (so paths aren't stored in zip's)
   os.chdir(tmpdir)
   for rec in result:

# 00| mydb 
# 01| id       
# 02| hostid 
# 03| time_trig           
# 04| utime_trig 
# 05| time_rec            
# 06| time_synchronized   
# 07| sync_offset 
# 08| magnitude 
# 09| significance 
# 10| latitude  
# 11| longitude  
# 12| file                                         
# 13| numreset 
# 14| dt   
# 15| trigtype 
# 16| hostipaddrid 
# 17| geoipaddrid 
# 18| sensor               
# 19| alignment 
# 20| level 
# 21| level_type 
# 22| quake_time 
# 23| quake_depth_km 
# 24| quake_lat 
# 25| quake_lon 
# 26| quake_magnitude 
# 27| quake_id   
# 28| quake_description 
# 29| quake_guid

      errlevel = 2
      #print "    ", rec[0] , "  ", rec[1], "  ", rec[2], "  ", rec[3], "  ", rec[4], "  ", rec[5], "  ", rec[6]
      #"db, triggerid, hostid, time_utc, time_us, magnitude, significance, latitude, longitude, file, " +\
      # "numreset, sensor, alignment, level_value, level_type\n"

      # test for valid zip file
      try:
        dbzipfile = rec[12]
        if IS_ARCHIVE:  # need to get fanout directory 
          fandir, dtime = getFanoutDirFromZip(dbzipfile)
          if (rec[0] == "Q"):
            fullpath = os.path.join(UPLOAD_WEB_DIR, fandir)
            zipinpath = os.path.join(fullpath, dbzipfile)
          else:
            fullpath = os.path.join(UPLOAD_WEB_DIR_CONTINUAL, fandir)
            zipinpath = os.path.join(fullpath, dbzipfile)
        else:
          if (rec[0] == "Q"):
            zipinpath = os.path.join(UPLOAD_WEB_DIR, dbzipfile)
          else:
            zipinpath = os.path.join(UPLOAD_WEB_DIR_CONTINUAL, dbzipfile)

        myzipin = zipfile.ZipFile(zipinpath, "r")
        if os.path.isfile(zipinpath) and myzipin.testzip() == None:
           errlevel = 3
           # valid zip file so add to myzipout, first close
           zipinlist = myzipin.namelist()
           myzipin.extractall(tmpdir)
           myzipin.close()

           for zipinname in zipinlist:
             errlevel = 4
             #zipinpath = os.path.join(tmpdir, zipinname)
             # OK - at this point the zip file requested has been unzipped, so we need to process metadata here
  #   m.mydb, m.id, m.hostid,   0 1 2
  #  from_unixtime(m.time_trigger) time_trig, FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6) utime_trig, 3 4
  #  m.magnitude,  m.significance,  5 6
  #  m.latitude, m.longitude, m.file, m.numreset,  7 8 9 10
  #  s.description sensor, IFNULL(a.description,'') alignment,  11 12 
  #   IFNULL(m.levelvalue,'') level, IFNULL(l.description,'') level_type 13 14 
        #def getSACMetadata(zipinname, hostid, latTrig, lonTrig, lvlTrig, lvlType, idQuake, timeQuake, depthKmQuake, latQuake, lonQuake, magQuake):
             getSACMetadata(zipinname, rec[2], rec[10], rec[11], rec[20], rec[21], rec[27], rec[22], rec[23], rec[24], rec[25], rec[26])
             myzipout.write(zipinname)
             os.remove(zipinname)


      except:
        print "Error " + str(errlevel) + " in myzipin " + zipinpath
        traceback.print_exc()
        #exit(3)
        continue

   fileCSV.close()
   fileSQL.close()
   os.chdir(DOWNLOAD_WEB_DIR)   # go to download dir to zip up csv & sql files
   os.chdir(curdir)   # go back to regular directory so tmpdir can be erased
   myzipout.close() 
   numbyte = os.path.getsize(zipoutpath)
   shutil.rmtree(tmpdir)    # remove temp directory
   myCursor.close();
   return numbyte

  except zipfile.error:
   print "Error " + str(errlevel) + " in " + zipoutpath + " or " + zipinpath +\
        " is an invalid zip file (tmpdir=" + tmpdir + ")"
   #dbconn.rollback()
   traceback.print_exc()
   shutil.rmtree(tmpdir)    # remove temp directory
   myCursor.close();
   if (myzipout != None):
      myzipout.close() 
      os.remove(zipoutpath)
   return 0
  except:
   print "Error " + str(errlevel) + " in " + zipoutpath + " or " + zipinpath + " (tmpdir=" + tmpdir + ")"
   #dbconn.rollback()
   traceback.print_exc()
   shutil.rmtree(tmpdir)    # remove temp directory
   myCursor.close();
   if (myzipout != None):
      myzipout.close() 
      os.remove(zipoutpath)
   return 0

def sendEmail(Username, ToEmailAddr, DLURL, NumMB):
  global SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT
  # sends email that job is done
  FromEmailAddr = "noreply@qcn.stanford.edu"
  server=smtplib.SMTP_SSL(SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT)
  msg = "Hello " + Username + ":\n\n" + "Your requested files are available for download " +\
    "over the next 24 hours from the following URL:\n\n" + DLURL +\
    "\n\nThe file size to download is approximately " + str(NumMB) + " megabytes." +\
    "\n\nNote that this email is automated - please do not reply!"
  if typeRunning == "C":
    subj = "QCN Continual Download Archive Completed"
  else:
    subj = "QCN Sensor Download Archive Completed"

  MessageText = """\
From: %s
To: %s
Subject: %s

%s
""" % (FromEmailAddr, ToEmailAddr, subj, msg)

  server.sendmail(FromEmailAddr, ToEmailAddr, MessageText)
  server.quit()

def updateRequest(dbconn, jobid, numbyte, outfilename, url):
   myCursor = dbconn.cursor()
   query = "UPDATE " + DBNAME_JOB + ".job SET finish_time=unix_timestamp(), " +\
                "url='" + url + "', local_path='" + outfilename + "', size=" + str(numbyte) +\
                " WHERE id=" + str(jobid)
   #print query
   myCursor.execute(query)
   dbconn.commit();
   myCursor.close();


# makes sure that the necessary paths are in place as defined above
def checkPaths():
   global UPLOAD_WEB_DIR
   global UPLOAD_WEB_DIR_CONTINUAL
   global DOWNLOAD_WEB_DIR

   if not os.access(UPLOAD_WEB_DIR, os.F_OK | os.W_OK):
      print UPLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR does not exist or not writable!"
      return 1
   
   if not os.access(UPLOAD_WEB_DIR_CONTINUAL, os.F_OK | os.W_OK):
      print UPLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR_CONTINUAL does not exist or not writable!"
      return 1

   if not os.access(DOWNLOAD_WEB_DIR, os.F_OK | os.W_OK):
      print DOWNLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR does not exist or not writable!"
      return 1
   
   return 0

# print out last 'numquake' quakes & count of triggers
def printQuakeList(dbconn, numquake):
   print "Last " + str(numquake) + " Quakes"
   print "QuakeID, Num Triggers, Date/Time (UTC), Latitude, Longitude, Magnitude, Description"

   strSQL = """select q.id, from_unixtime(q.time_utc), q.magnitude, q.latitude, q.longitude, q.description,
((select count(*) from sensor.qcn_trigger t1 where t1.qcn_quakeid=q.id) +
(select count(*) from continual.qcn_trigger t2 where t2.qcn_quakeid=q.id)) trigcnt
from sensor.qcn_quake q 
group by q.id having trigcnt>0
order by q.id desc
 limit """ + str(numquake)
   #print strSQL
   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   result = myCursor.fetchall()
   myCursor.close()
   for rec in result:
     print rec[0], rec[6], rec[1], rec[3], rec[4], rec[2], rec[5]

   return 0

# get the tuples of the triggers
def getTriggerTupleTime(strWhere, iDelay, dbconn):
   return 0

# get the tuples of the triggers
def getTriggerTupleQuakeID(idQuake, iDelay, dbconn):
   # strWhere is the where clause ie quakeid=5445 --- iDelay is the delay time # of seconds from first trigger
   #strSQL = "select * from (select 'sensor', s.* from sensor.qcn_trigger s " + strWhere + " UNION " +\
   #         "select 'continual', c.* from continual.qcn_trigger c " + strWhere + ") t order by time_trigger"
   strWhere = "WHERE varietyid=0 and qcn_quakeid=" + str(idQuake)

   # should perhaps get first trigger for start time
   strSQL = "select min(trigtime) from (select min(time_trigger) trigtime from sensor.qcn_trigger s " + strWhere + " UNION " +\
            "select min(time_trigger) trigtime from continual.qcn_trigger c " + strWhere + ") t "

   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   row = myCursor.fetchone()
   myCursor.close()
   if row == None or len(row) == 0:
     return None

   timeStart = row[0] - iDelay

   # now get end time of event
   strSQL = "select max(trigtime) from (select max(time_trigger) trigtime from sensor.qcn_trigger s " + strWhere + " UNION " +\
            "select max(time_trigger) trigtime from continual.qcn_trigger c " + strWhere + ") t "

   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   row = myCursor.fetchone()
   myCursor.close()
   if row == None or len(row) == 0:
     return None

   timeEnd = row[0] + iDelay

   strSQL = "select * from (select 'sensor', s.* from sensor.qcn_trigger s " +\
          "where time_trigger between " +\
            str(timeStart) + " AND " + str(timeEnd) +\
             " UNION " +\
            "select 'continual', c.* from continual.qcn_trigger c " +\
          "where time_trigger between " +\
            str(timeStart) + " AND " + str(timeEnd) +\
                ") t order by time_trigger"

   #print timeStart, timeEnd 
   #print strSQL
   #sys.exit()

   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   result = myCursor.fetchall()
   myCursor.close()
   return result

def processTriggerTuple(tt, iDelay, dbconn):
   if len(tt) == 0:
     print "No triggers found for your criteria!"
     print "Type ./'" + PROGNAME + " -h' for help"
     return 1
   timeStart = tt[0][5]
   play = 0
   print "Trigger replay starting in " + str(iDelay) + " seconds...."
   SLEEP_INTERVAL = 0.01
   for t in tt:
     while play < t[5]-timeStart+iDelay:
        sleep(SLEEP_INTERVAL)
        play = play + SLEEP_INTERVAL
     print t[0], t[1], t[2], t[3], t[4], t[5]-timeStart+iDelay, t[6], t[7], t[8]

     # insert into sensor/continual.qcn_trigger table here
     # get new trigger insert ID # and update t[] for trimem_test insert below

     # insert into trigmem_test.qcn_trigger_memory table here

   return 0

def deleteTrigMemTest(dbconn):
   # delete mem triggers older than 5 minutes
   strSQL = "DELETE FROM trigmem_test.qcn_trigger_memory WHERE time_trigger < unix_timestamp() - 300"
   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   myCursor.close()
   return 0

# main proc
def main():
   global UPLOAD_WEB_DIR
   global DOWNLOAD_WEB_DIR
   global DOWNLOAD_URL
   global UPLOAD_WEB_DIR_CONTINUAL
   global DBNAME
   global DBNAME_CONTINUAL
   global IS_ARCHIVE

   global DATE_MIN_ORIG
   global DATE_MAX_ORIG
   global DATE_MIN
   global DATE_MAX
   global TIME_MIN
   global TIME_MAX
   global LAT_MIN
   global LAT_MAX
   global LNG_MIN
   global LNG_MAX
   global DELAY_TIME

   class FinishedException(Exception):
     def __init__(self, value):
       self.value = value
     def __str__(self):
       return repr(self.value)

   strDescription = "This program will 'replay' past triggers into the trigmem_test.qcn_trigger_memory table on qcn-data"

   # get cmd-line arguments
   parser = OptionParser()
   parser = OptionParser(description=strDescription)
   parser.add_option("--date_start", dest="DATE_MIN", type="string", help="Enter Start Date in YYYY-MM-DD format", metavar="DATE_MIN")
   parser.add_option("--time_start", dest="TIME_MIN", type="int", help="Enter Start Time in 24-hr UTC format e.g. HHMM", metavar="TIME_MIN")
   parser.add_option("--date_end", dest="DATE_MAX", type="string", help="Enter End Date in YYYY-MM-DD format", metavar="DATE_MAX")
   parser.add_option("--time_end", dest="TIME_MAX", type="int", help="Enter End Time in 24-hr UTC format e.g. HHMM", metavar="TIME_MAX")
   #parser.add_option("--lat_min", dest="LAT_MIN", type="float", help="Enter Minimum Latitude [-90,90]", metavar="LAT_MIN")
   #parser.add_option("--lat_max", dest="LAT_MAX", type="float", help="Enter Maximum Latitude [-90,90]", metavar="LAT_MAX")
   #parser.add_option("--lng_min", dest="LNG_MIN", type="float", help="Enter Minimum Longitude [-180,180]", metavar="LNG_MIN")
   #parser.add_option("--lng_max", dest="LNG_MAX", type="float", help="Enter Maximum Longitude [-180,180]", metavar="LNG_MAX")
   parser.add_option("--delay", dest="DELAY_TIME", type="int", help="Number of seconds to delay until start of first trigger (default 5s)", metavar="sec")
   parser.add_option("--quake_id", dest="QUAKE_ID", type="int", help="Enter Quake ID # (run script with --quake_list n to get last n events", metavar="id")
   parser.add_option("--quake_list", dest="QUAKE_LIST", type="int", help="Show last 'n' earthquakes with matching triggers", metavar="n")
   (options, args) = parser.parse_args();

   #lat/lng
   # default to SCEDC run
   #if options.LAT_MIN == None:
   #  options.LAT_MIN  = 31.5
   #if options.LAT_MAX == None:
   #  options.LAT_MAX  = 37.5
   #if options.LNG_MIN == None:
   #  options.LNG_MIN  = -121.0
   #if options.LNG_MAX == None:
   #  options.LNG_MAX  = -114.0

   #dates
   if options.DATE_MAX == None and options.DATE_MIN != None:
     options.DATE_MAX = options.DATE_MIN
   if options.DATE_MIN == None and options.DATE_MAX != None:
     options.DATE_MIN = options.DATE_MAX

   #times
   if options.TIME_MIN != None or options.TIME_MAX != None:
     if options.TIME_MIN < 0 or options.TIME_MAX < 0 or options.TIME_MIN > 2359 or options.TIME_MAX > 2359:
       print "Check your times - should be between 0000 and 2359."
       print "Type ./'" + PROGNAME + " -h' for help"
       sys.exit()

   if options.LAT_MIN != None or options.LAT_MAX != None:
     if options.LAT_MIN < -90 or options.LAT_MIN > 90 or options.LAT_MIN > options.LAT_MAX:
       print "Incorrect Minimum Latitude, must be between -90 and 90 and less than Maximum Latitude entered."
       print "Type ./'" + PROGNAME + " -h' for help"
       sys.exit()

     if options.LAT_MAX < -90 or options.LAT_MAX > 90 or options.LAT_MAX < options.LAT_MIN:
       print "Incorrect Maximum Latitude, must be between -90 and 90 and greater than Minimum Latitude entered."
       print "Type ./'" + PROGNAME + " -h' for help"
       sys.exit()

     if options.LNG_MIN < -180 or options.LNG_MIN > 180 or options.LNG_MIN > options.LNG_MAX:
       print "Incorrect Minimum Longitude, must be between -180 and 180 and less than Maximum Longitude entered."
       print "Type ./'" + PROGNAME + " -h' for help"
       sys.exit()

     if options.LNG_MAX < -180 or options.LNG_MAX > 180 or options.LNG_MAX < options.LNG_MIN:
       print "Incorrect Maximum Longitude, must be between -180 and 180 and greater than Minimum Longitude entered."
       print "Type ./'" + PROGNAME + " -h' for help"
       sys.exit()

   LAT_MIN = options.LAT_MIN
   LAT_MAX = options.LAT_MAX
   LNG_MIN = options.LNG_MIN
   LNG_MAX = options.LNG_MAX

   if options.DELAY_TIME == None:
     options.DELAY_TIME = 5

   try:
      cnt = 0

      #open database connection
      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      if dbconn == None:
         print "Database error - check connection settings"
         raise

      deleteTrigMemTest(dbconn)

      # test dates
      if options.DATE_MIN == None and options.DATE_MAX == None:
        sqlts = "SELECT DATE_SUB(CURDATE(), INTERVAL 1 DAY) " 
        myCursor = dbconn.cursor()
        myCursor.execute(sqlts)
        res = myCursor.fetchall()
        myCursor.close()
        if res[0][0] == None:
          print "Cannot make SQL query to get date"
          print "Type './trigger_replay.py -h' for help"
          dbconn.close()
          sys.exit()
        options.DATE_MIN = str(res[0][0])
        options.DATE_MAX = options.DATE_MIN
        # if using the default filename, add the date sub to filename

      DATE_MIN_ORIG = options.DATE_MIN + " 00:00:00"
      DATE_MAX_ORIG = options.DATE_MAX + " 23:59:59"
      DATE_MIN = DATE_MIN_ORIG
      DATE_MAX = DATE_MAX_ORIG

      # first get archive time
      sqlts = "select value_int, unix_timestamp('" + DATE_MIN + "'), unix_timestamp('" + DATE_MAX + "'), DATE_SUB(CURDATE(), INTERVAL 1 DAY) " +\
       "from sensor.qcn_constant where description='ArchiveTime'"
      myCursor = dbconn.cursor()
      myCursor.execute(sqlts)

      res = myCursor.fetchall()
      myCursor.close()
      if res[0][0] == None or res[0][1] == None or res[0][2] == None:
         print "Error - cannot retrieve archive trigger time or validate min/max dates"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      timeArchive = int(res[0][0])
      DATE_MIN = int(res[0][1])
      DATE_MAX = int(res[0][2])

      if DATE_MIN == None or DATE_MIN < 1e6 or DATE_MAX == None or DATE_MAX < 1e6:
         print "Error - Invalid start or end date"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()


      if DATE_MIN > DATE_MAX:
         print "Error - start date greater than end date"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      if DATE_MIN >= timeArchive and DATE_MAX >= timeArchive:
         IS_ARCHIVE = False
         UPLOAD_WEB_DIR           = "/data/QCN/trigger/"
         UPLOAD_WEB_DIR_CONTINUAL = "/data/QCN/trigger/continual/"
         DBNAME                   = "sensor"
         DBNAME_CONTINUAL         = "continual"
      elif DATE_MIN < timeArchive and DATE_MAX <= timeArchive:
         IS_ARCHIVE = True
         UPLOAD_WEB_DIR           = "/data/QCN/trigger/archive/"
         UPLOAD_WEB_DIR_CONTINUAL = "/data/QCN/trigger/archive/continual/"
         DBNAME                   = "sensor_archive"
         DBNAME_CONTINUAL         = "continual_archive"
      else: # error! 
         print "Error - dates straddle the archive time - not yet supported"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      # first make sure all the necessary paths are in place
      #if (checkPaths() != 0):
      #   sys.exit(2)

      bDone = false 
      #procRequest(dbconn)
      # quake list supercedes all ie print quake ID & info & return
      if !bDone and options.QUAKE_LIST != None:
        if options.QUAKE_LIST == 0:
           options.QUAKE_LIST = 10
        printQuakeList(dbconn, options.QUAKE_LIST)
        bDone = true

      if !bDone and options.QUAKE_ID != None:
        tt = getTriggerTupleQuakeID(options.QUAKE_ID, options.DELAY_TIME, dbconn)
        processTriggerTuple(tt, options.DELAY_TIME, dbconn)
        bDone = true
        #raise FinishedException(2)

      if !bDone and options.DATE_MIN != None and options.DATE_MAX != None:
        tt = getTriggerTupleTime(DATE_MIN, DATE_MAX, options.DELAY_TIME, dbconn)
        processTriggerTuple(tt, options.DELAY_TIME, dbconn)
        bDone = true
        #raise FinishedException(3)

      deleteTrigMemTest(dbconn)
      dbconn.close()
      dbconn = None
      print "Finished!"
      return 0


   except FinishedException: #  as e: 
      # just to signify we're done
      if dbconn != None:
        dbconn.close()
      return 0

   except:
      traceback.print_exc()
      if dbconn != None:
         dbconn.close()
      return 3

if __name__ == '__main__':
    main()


