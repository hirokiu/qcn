#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
using std::string;
using std::vector;

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"
// re-use the data structures from the qcn_trigger scheduler stuff
#include "../trigger/qcn_trigger.h"
#include "structs.h"

#define FILE_NAME_TRIGGER_LAPTOP  "/var/www/qcn/rt_image/rt_triggers_LTN.xyz"
#define FILE_NAME_TRIGGER_DESKTOP "/var/www/qcn/rt_image/rt_triggers_DTN.xyz"

#define CSH_PLOT_CMD  "/bin/csh /var/www/qcn/rt_image/inc/rt_images.csh"

// returns quakeid if event found/created (0 if not)
int getQCNQuakeID(
    const double& dLat, 
    const double& dLng, 
    const int& iCtr, 
    const double& dTimeMin, 
    const double& dTimeMax);

void close_db();
void do_delete_trigmem();
void setQueries();
int do_display(int hid[]);

#endif //_DISPLAY_H_
