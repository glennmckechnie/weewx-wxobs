#
#    Copyright (c) 2017 Glenn McKechnie glenn.mckechnie@gmail.com>
#    Credit to Tom Keffer <tkeffer@gmail.com>, Matthew Wall and the core
#    weewx team, all from whom I've borrowed heavily.
#    Mistakes are mine, corrections and or improvements welcomed
#       https://github.com/glennmckechnie/weewx-wxobs
#
#    See the file LICENSE.txt for your full rights.
#
#

import syslog

from weeutil.weeutil import to_bool
from weewx.cheetahgenerator import SearchList

wxobs_version = "0.03"

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
    Nah!, just right.
    """

    def __init__(self, generator):
        SearchList.__init__(self, generator)

        """
        This is a minimal SLE - all we want is to make the php configuration
        easier by transferring the database values as used by weewx, plus a few
        other variables.

        In wxobs/skin.conf:
        ext_interval: is the spacing between records; 1800 is a half-hour
        and is the default

        arch_interval: is the number of records to averge over. The default is to
        use the weewx.conf value for archive_interval which will return the
        equivalent of: one observation taken at that archive time.
        You could choose to set another value?

        The other alternative is to set the $arch_interval to be equal to the
        value for $ext_interval. This will ensure that all records for the day
        will be involved in the calcs and the average will be returned as the
        result.

        app_Temp: This is a recent addition to weewx and is not enabled by
        default. The calculation is performed but there is no field in the stock
        database. This variable allows for the substitution with another value
        - windchill is just one suggestion.
        Keep it to the group_degrees (because the label is hard coded in.)

        wind_adjust: This can be used to supply a constant to adjust the
        displayed value returned from the database. This satisfies a quirk in
        my setup where the database is in m/s but I want k/hour displayed.

        us_note: An information message will appear on the report page
        (index.php) if the database is in US (imperial) units. This is to
        inform you that the delta-T calcs will be adjusted for the incoming
        units, and there may be further work required, especially if the script
        doesn't detect your incoming unit types.
        This is a switch (boolean) to turn it off.

        wxobs_debug: Allow index.php to include debugging info if set to 1
        or if set to 5, this script to the weewx log

        """
        self.wxobs_debug = int(self.generator.skin_dict['wxobs'].get(
            'wxobs_debug', '0'))

        self.ext_interval = self.generator.skin_dict['wxobs'].get(
            'ext_interval', '1800')
        self.arch_interval = self.generator.skin_dict['wxobs'].get(
            'arch_interval')
        if not self.arch_interval:
            self.arch_interval = self.generator.config_dict['StdArchive'] \
                .get('archive_interval')
        self.appTemp = self.generator.skin_dict['wxobs'].get(
            'app_Temp', 'appTemp')
        self.wind_adjust = self.generator.skin_dict['wxobs'].get(
            'wind_adjust', '1')
        self.us_note = to_bool(self.generator.skin_dict['wxobs'].get(
            'show_usnote', True))


#       target_unit = METRICWX    # Options are 'US', 'METRICWX', or 'METRIC'
        self.targ_unit = self.generator.config_dict['StdConvert'].get(
            'target_unit')

        def_dbase = self.generator.config_dict['DataBindings'] \
            ['wx_binding'].get('database')
        if self.wxobs_debug >= 5:
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
            if self.wxobs_debug >= 5:
                loginf("mysql database is %s, %s, %s, %s" % (
                    self.mysql_base, self.mysql_host,
                    self.mysql_user, self.mysql_pass))
        elif def_dbase == 'archive_sqlite':
            self.dbase = 'sqlite'
            self.sq_dbase = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            self.sq_root = self.generator.config_dict['DatabaseTypes'] \
                ['SQLite'].get('SQLITE_ROOT')

            self.sqlite_db = ("%s/%s" %(self.sq_root, self.sq_dbase))

            if self.wxobs_debug >= 5:
                loginf("sqlite database is %s, %s, %s" % (
                    self.sq_dbase, self.sq_root, self.sqlite_db))

if __name__ == '__main__':
    # Hmmm!
    # use wee_reports instead, see the inline comments above.
    pass
