#!/bin/bash
FILE_BACKUP=/tmp/qcn-archive-backup.sql
FILE_BACKUP_GZ=/tmp/qcn-archive-backup.sql.gz
DIR_QCN_DATA=/data/QCN/
/bin/rm -f $FILE_BACKUP
/bin/rm -f $FILE_BACKUP_GZ
/bin/nice -n19 /usr/local/mysql/bin/mysqldump -u root -pPWD --databases sensor_archive continual_archive > $FILE_BACKUP
/bin/nice -n19 /usr/bin/gzip -f $FILE_BACKUP
/bin/nice -n19 /bin/mv $FILE_BACKUP_GZ $DIR_QCN_DATA
