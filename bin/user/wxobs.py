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

wxobs_version = "0.05"

def logmsg(level, msg):
    syslog.syslog(level, '%s' % msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)


class wxobs(SearchList):

    def __init__(self, generator):
        SearchList.__init__(self, generator)

        """
        This is a minimal SLE - all we want is to make the php configuration
        easier by transferring the database values as used by weewx, plus a few
        other variables.

        In wxobs/skin.conf:
        ext_interval: is the spacing between records; 1800 is a half-hour
        and is the default

        arch_interval: is the number of records to averge over. The default is
        to use the weewx.conf value for archive_interval which will return the
        equivalent of: one observation taken at that archive time.
        You could choose to set another value?

        The other alternative is to set the $arch_interval to be equal to the
        value for $ext_interval. This will ensure that all records for the day
        will be involved in the calcs and the average will be returned as the
        result.

        app_Temp: This is a recent addition to weewx and is not enabled by
        default. The calculation is performed but there is no field in the
        stock database. This variable allows for the substitution with another
        value.
        The default is to use windchill.
        Keep it to the group_degrees (because the label is hard coded in.)

        shift_rain: For rain accounting times other than midnight to midnight
        set this to True 
        If no other options are given the accounting time will be the australian
        rain day which starts at 9 a.m.
        default is false - start at midnight 00:00:00 through to the next day.

        rain_start: used to shift time (in seconds) to something other than 9a.m.
        default is 32400

        rain_label: the o'clock label for the rain_start above.
        default is 9

        show_warning: An information message will appear on the report page
        (index.php) if the database is in US units (imperial) or units are
        detected that don't match the native units required for the delta-T
        calcs.
        An information div is included in the report page when this occurs.
        This is a switch (boolean) to turn it off.

        wxobs_debug: Allow index.php to include debugging info if set to...
        1 and above is low level, variables, some logic.
        3 only for delta-T final values (low level - if enabled)
        4 only for delta-T unit conversion calcs (verbose) - if enabled
        5 only for ordinalCompass conversion calcs (N, NE...CALM) (verbose)

        calculate_deltaT: Whether to generate deltaT for the report page.
        Default is not to generate that data.
        This is a switch (boolean) to turn it on.

        tempConvert:
        speedConvert:
        pressConvert:
        rainConvert: These are all used to convert the database units to ones
        for display by the php generated report.
        Because we are bypassing weewx to generate the historical data, we
        can't utilize the inbuilt weewx functions for unit conversion therefore
        we need to affect them ourselves.
        This is performed (if needed) by specifying the conversion to be done
        from the [[PHPUnits]] section of the skin.conf file.
        The default is to perform no conversion, to accept the units as they
        are.
        """

        self.wxobs_version = wxobs_version
        self.wxobs_debug = int(self.generator.skin_dict['wxobs'].get(
            'wxobs_debug', '0'))

        self.ext_interval = self.generator.skin_dict['wxobs'].get(
            'ext_interval', '1800')
        self.arch_interval = self.generator.skin_dict['wxobs'].get(
            'arch_interval')
        if not self.arch_interval:
            self.arch_interval = self.generator.config_dict['StdArchive'] \
                .get('archive_interval')
        self.app_temp = self.generator.skin_dict['wxobs'].get(
            'app_Temp', 'windchill')
        self.want_delta = to_bool(self.generator.skin_dict['wxobs']['DeltaT'] \
            .get('calculate_deltaT', False))
        if not self.want_delta:
            self.show_warning = to_bool(self.generator.skin_dict['wxobs'] \
            ['DeltaT'].get('show_warning', False))
        self.show_warning = to_bool(self.generator.skin_dict['wxobs']['DeltaT'] \
            .get('show_warning', True))

        # these variable are being used as a function names, thus the Case
        # abuse... usage! and the complaints from syntax checkers.
        self.tempConvert = self.generator.skin_dict['wxobs']['PHPUnits'].get(
            'temperature_convert', 'NTC')
        self.speedConvert = self.generator.skin_dict['wxobs']['PHPUnits'].get(
            'speed_convert', 'NSC')
        self.pressConvert = self.generator.skin_dict['wxobs']['PHPUnits'].get(
            'pressure_convert', 'NPC')
        self.rainConvert = self.generator.skin_dict['wxobs']['PHPUnits'].get(
            'rain_convert', 'NDC')

        self.shift_rain = to_bool(self.generator.skin_dict['wxobs'] \
            ['RainTiming'].get('shift_rain', False))
        #32400 (rainday_start) == 9 hours == 9 a.m.
        self.rainday_start = self.generator.skin_dict['wxobs']['RainTiming'] \
            .get('rain_start', '32400')
        #32400 == 9 hours == 9 (start_label) a.m.
        self.start_label = self.generator.skin_dict['wxobs']['RainTiming'] \
            .get('start_label', '9')


        # target_unit = METRICWX # Options are 'US', 'METRICWX', or 'METRIC'
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
            v_al = ["<?php\n $php_dbase = '%s';\n $php_mysql_base = '%s';\n"
                    " $php_mysql_host = '%s';\n $php_mysql_user = '%s';\n"
                    " $php_mysql_pass = '%s';\n" %
                    (self.dbase, self.mysql_base, self.mysql_host,
                     self.mysql_user, self.mysql_pass)]
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
            v_al = ["<?php\n $php_dbase = 'sqlite';\n $php_sqlite_db = '%s';\n" %
                    self.sqlite_db]

            if self.wxobs_debug >= 5:
                loginf("sqlite database is %s, %s, %s" % (
                    self.sq_dbase, self.sq_root, self.sqlite_db))

        # phpinfo.php shows include_path as .:/usr/share/php, we'll put it
        # in there and hopefully that will work for most users.
        # I iuse/prefer /tmp/wxobs_inc.inc but perhaps that only works for me?
        self.include_file = self.generator.skin_dict['wxobs'].get(
            'include_file', '/usr/share/php/wxobs_incl.inc')

        php_inc = open(self.include_file, 'w')
        php_inc.writelines(v_al)
        php_inc.close()

if __name__ == '__main__':
    # Hmmm!
    # use wee_reports instead, see the inline comments above.
    pass
