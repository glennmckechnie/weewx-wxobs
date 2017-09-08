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

import syslog

from weewx.cheetahgenerator import SearchList

wxobs_version = "0.01"

def logmsg(level, msg):
    syslog.syslog(level, '%s' % msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)


class wxobs(SearchList):
    """
    Overkill?
    """

    def __init__(self, generator):
        SearchList.__init__(self, generator)

        """
        All we want is the database configuration values from weewx.conf
        and perhaps the time.

        Timings for displayed values in Historical section:

        ext_interval is the spacing between records; 1800 is a half-hour.
         and is the default

        arch_interval is the number of records to averge over; choose a value
         that suits you.

        $arch_interval = $ext_interval:
         this means all records for the day are involved in calcs.
        Alternatively $arch_interval can be set for a single reading if matched
        to the archive_interval value in weewx.conf (mine happens to be 60
        seconds and is the default) this becomes the equivalent of: 
        one observation taken at that archive time.
        """
        self.sql_debug ='0'

        self.ext_interval = self.generator.skin_dict['wxobs'].get('ext_interval', '1800')
        self.arch_interval = self.generator.skin_dict['wxobs'].get('arch_interval', '1800')
        if not self.arch_interval:
            self.arch_interval = self.generator.config_dict['StdArchive'] \
                .get('archive_interval')

        def_dbase = self.generator.config_dict['DataBindings'] \
            ['wx_binding'].get('database')
        if self.sql_debug >= 5 :
            logdbg("database is %s" %  def_dbase)

        if def_dbase == 'archive_mysql':
            self.dbase = 'mysql'
            self.mysql_base = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            self.mysql_host = self.generator.config_dict['DatabaseTypes'] \
                ['MySQL'].get('host')
            self.mysql_user = self.generator.config_dict['DatabaseTypes'] \
                ['MySQL'].get('user')
            self.mysql_pass = self.generator.config_dict['DatabaseTypes'] \
                ['MySQL'].get('password')
            if self.sql_debug >= 5 :
                loginf("mysql database is %s, %s, %s, %s" % (
                    self.mysql_base, self.mysql_host, self.mysql_user, self.mysql_pass))
        elif def_dbase == 'archive_sqlite':
            self.dbase = 'sqlite'
            self.sq_dbase = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            self.sq_root = self.generator.config_dict['DatabaseTypes'] \
                ['SQLite'].get('SQLITE_ROOT')

            self.sqlite_db = ("%s/%s" %(self.sq_root,self.sq_dbase))

            if self.sql_debug >= 5 :
                loginf("sqlite database is %s, %s, %s" % (
                    self.sq_dbase, self.sq_root, self.sqlite_db))

if __name__ == '__main__':
    # Hmmm!
    # use wee_reports instead, see the inline comments above.
    pass
