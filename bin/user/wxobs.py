#
#    Copyright (c) 2017 Glenn McKechnie glenn.mckechnie@gmail.com>
#    Credit to Tom Keffer <tkeffer@gmail.com>, Matthew Wall and the core
#    weewx team, all from whom I've borrowed heavily.
#    Mistakes are mine, corrections and or improvements welcomed
#       https://github.com/glennmckechnie/weewx-sqlbackup
#
#    See the file LICENSE.txt for your full rights.
#
#

import os
import errno
import sys
import shutil
import gzip
import subprocess
import syslog
import time
import datetime

import weewx.engine
import weewx.manager
import weewx.units
from weewx.wxengine import StdService
from weewx.cheetahgenerator import SearchList
from weeutil.weeutil import to_bool

wxobs_version = "0.01"

def logmsg(level, msg):
    syslog.syslog(level, '%s' % msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def tlwrite(txt): # unused
    tl = open(self.tail_file, 'w')
    tl.write(txt)
    tl.close()

class wxobs(SearchList):
    """
    Overkill?
    """

    def __init__(self, generator):
        SearchList.__init__(self, generator)

        """
        All we want is the database configuration values from weewx.conf
        and perhaps the time.
        """


#        global skin_name
#        skin_name =  self.generator.skin_dict['skin']
#        self.skin_name = skin_name # for export to the template / html

        self.sql_debug ='5'

        def_dbase = self.generator.config_dict['DataBindings'] \
            ['wx_binding'].get('database')
        # return archive_mysql or archive_sqlite
        if self.sql_debug >= 5 :
            logdbg("database is %s" %  def_dbase)

        if def_dbase == 'archive_mysql':
            self.dbase = 'mysql'
            self.mysql_base = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            # weatherpi
            self.mysql_host = self.generator.config_dict['DatabaseTypes'] \
                ['MySQL'].get('host')
            # localhost
            #self.mysql_user = self.generator.config_dict['DatabaseTypes'] \
            self.mysql_user = self.generator.config_dict['DatabaseTypes'] \
                ['MySQL'].get('user')
            # weatherpi
            #self.mysql_pass = self.generator.config_dict['DatabaseTypes'] \
            self.mysql_pass = self.generator.config_dict['DatabaseTypes'] \
                ['MySQL'].get('password')
            # weewx
            v_alues = ["<?php\n $dbase = 'mysql';\n $mysql_base = '%s';\n"
                " $mysql_host = '%s';\n $mysql_user = '%s';\n $mysql_pass = '%s';\n?>"
                % (self.mysql_base, self.mysql_host, self.mysql_user, self.mysql_pass)]
            if self.sql_debug >= 5 :
                loginf("mysql database is %s, %s, %s, %s" % (
                    self.mysql_base, self.mysql_host, self.mysql_user, self.mysql_pass))
        elif def_dbase == 'archive_sqlite':
            self.dbase = 'sqlite'
            self.sq_dbase = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            # weewx.sdb
            self.sq_root = self.generator.config_dict['DatabaseTypes'] \
                ['SQLite'].get('SQLITE_ROOT')
            # /var/lib/weewx

            self.sqlite_db = "self.sq_root/self.sq_dbase"
            v_alues = ["<?php\n $dbase = 'sqlite';\n $sqlite_db = '%s';\n?>" % self.sqlite_db]

            if self.sql_debug >= 5 :
                loginf("sqlite database is %s, %s, %s" % (
                    self.sq_dbase, self.sq_root, sqlite_db))

#        self.phpinc_file = '/tmp/wxobs_inc.php'
#
#        php_inc=open(self.phpinc_file, 'w')
#        php_inc.writelines(v_alues)
#        php_inc.close()


if __name__ == '__main__':
    # Hmmm!
    # use wee_reports instead, see the inline comments above.
    pass
