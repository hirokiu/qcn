#! /usr/bin/env python

import sys, string, MySQLdb
from time import time

DBNAME = "sensor"
DBHOST = "localhost"
DBUSER = "root"
DBPASSWD = "PASSWORD"

def main():
   try:
      t1 = time()
      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)
      cMain = dbconn.cursor()
      cMain.execute("set autocommit=1");
      cMain.execute("call do_stats();")
      cMain.execute("update sensor.result set outcome=1,validate_state=1 where server_state=5 and (outcome!=1 or validate_state!=1)")
      cMain.execute("update continual.result set outcome=1,validate_state=1 where server_state=5 and (outcome!=1 or validate_state!=1)")
      dbconn.close()
      t2 = time()
      print "do_stats Successful on " + str(t2) + " - Elapsed Time = " + str(t2-t1) + " seconds"
   except:
      t2 = time()
      print "do_stats Database error at " + str(t2) + " - Elapsed Time = " + str(t2-t1) + " seconds"
      sys.exit(1)

if __name__ == '__main__':
    main()


