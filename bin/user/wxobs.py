#
#    Copyright (c) 2017 Glenn McKechnie glenn.mckechnie@gmail.com>
#    Credit to Tom Keffer <tkeffer@gmail.com>, Matthew Wall and the core
#    weewx team, all from whom I've borrowed heavily.
#    Mistakes are mine, corrections and or improvements welcomed
#       https://github.com/glennmckechnie/weewx-wxobs
#
#    rsync code based on weeutil/rsyncupload.py by
#       Will Page <companyguy@gmail.com> and
#
#    See the file LICENSE.txt for your full rights.
#
#

import syslog
import subprocess
import time
import errno

import weewx.engine

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

def rsync(rsync_user, rsync_remote, rsync_loc_dir, rsync_rem_str, wxobs_debug, log_success):

    t1 = time.time()
    # construct the command argument
    cmd = ['rsync']
    cmd.extend(["-ac"])
    #cmd.extend(["-tOJrl"])
    # provide some stats on the transfer
    cmd.extend(["--stats"])
    cmd.extend(["--compress"])
    cmd.extend([rsync_loc_dir])
    cmd.extend([rsync_rem_str])

    try:
        # perform the actual rsync transfer...
        if wxobs_debug >= 2:
            logdbg("rsync cmd is ... %s" % (cmd))
        loginf("rsync cmd is ... %s" % (cmd))
        rsynccmd = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout = rsynccmd.communicate()[0]
        stroutput = stdout.encode("utf-8").strip()
    except OSError, e:
            #print "EXCEPTION"
        if e.errno == errno.ENOENT:
            logerr("wxobs-rsync: rsync does not appear to be installed on this system. \
                   (errno %d, \"%s\")" % (e.errno, e.strerror))
        raise

    # we have some output from rsync so generate an appropriate message
    if stroutput.find('rsync error:') < 0:
        # no rsync error message so parse rsync --stats results
        rsyncinfo = {}
        for line in iter(stroutput.splitlines()):
            if line.find(':') >= 0:
                (n, v) = line.split(':', 1)
                rsyncinfo[n.strip()] = v.strip()
        # get number of files and bytes transferred and produce an
        # appropriate message
        try:
            if 'Number of regular files transferred' in rsyncinfo:
                N = rsyncinfo['Number of regular files transferred']
            else:
                N = rsyncinfo['Number of files transferred']

            Nbytes = rsyncinfo['Total file size']
            Nsent = rsyncinfo['Literal data']
            if N is not None and Nbytes is not None:
                rsync_message = "rsync'd %s bytes in %s files (%s) in %%0.2f seconds" % (Nsent, N, Nbytes)
            else:
                rsync_message = "rsync executed in %0.2f seconds"
        except:
            rsync_message = "rsync :exception raised: executed in %0.2f seconds"
    else:
        # suspect we have an rsync error so tidy stroutput
        # and display a message
        stroutput = stroutput.replace("\n", ". ")
        stroutput = stroutput.replace("\r", "")
        # Attempt to catch a few errors that may occur and deal with them
        # see man rsync for EXIT VALUES
        rsync_message = "rsync command failed after %0.2f seconds (set 'wxobs_debug = 1'),"
        if "code 1)" in stroutput:
            if wxobs_debug >= 2:
                logerr("wxobs-rsync: rsync code 1 - %s" % stroutput)
            rsync_message = "syntax error in rsync command - set debug = 1 - ! FIX ME !"
            loginf("wxobs-rsync:  ERR %s " % (rsync_message))
            rsync_message = "code 1, syntax error, failed rsync executed in %0.2f seconds"

        elif ("code 23" and "Read-only file system") in stroutput:
            # read-only file system
            # sadly, won't be detected until after first succesful transfer
            # but it's useful then!
            if wxobs_debug >= 2:
                logerr("wxobs-rsync: rsync code 23 - %s" % stroutput)
            loginf("ERR Read only file system ! FIX ME !")
            rsync_message = "code 23, read-only, rsync failed executed in %0.2f seconds"
        elif ("code 23" and "link_stat") in stroutput:
            # likely to be that a local path doesn't exist - possible typo?
            if wxobs_debug >= 2:
                logdbg("wxobs-rsync: rsync code 23 found %s" % stroutput)
            rsync_message = "rsync code 23 : is %s correct? ! FIXME !" % (rsync_loc_dir)
            loginf("wxobs-rsync:  ERR %s " % rsync_message)
            rsync_message = "code 23, link_stat, rsync failed executed in %0.2f seconds"

        elif ("code 12") and ("Permission denied") in stroutput:
            if wxobs_debug >= 2:
                logdbg("wxobs-rsync: rsync code 12 - %s" % stroutput)
            rsync_message = "Permission error in rsync command, probably remote \
                             authentication ! FIX ME !"
            loginf("wxobs-rsync:  ERR %s " % (rsync_message))
            rsync_message = "code 12, rsync failed, executed in % 0.2f seconds"
        elif ("code 12") and ("No route to host") in stroutput:
            if wxobs_debug >= 2:
                logdbg("wxobs-rsync: rsync code 12 - %s" % stroutput)
            rsync_message = "No route to host error in rsync command ! FIX ME !"
            loginf("wxobs-rsync:  ERR %s " % (rsync_message))
            rsync_message = "code 12, rsync failed, executed in % 0.2f seconds"
        else:
            logerr("ERROR: wxobs-rsync: [%s] reported this error: %s" % (cmd, stroutput))

    if log_success:
        if wxobs_debug == 0:
            to = ''
            rsync_rem_str = ''
        else:
            to = ' to '
        t2= time.time()
        loginf("wxobs-rsync: %s" % rsync_message % (t2-t1) + to + rsync_rem_str)

class wxobs(SearchList):

    def __init__(self, generator):
        SearchList.__init__(self, generator)

        """
        This is a minimal SLE - all we want is to make the php configuration
        easier by transferring the database values as used by weewx, plus a few
        other variables.
        It has since expanded to allow the transfer of the sqlite database
        by using rsync, providing the [Remote] section of the config is populated

        In wxobs/skin.conf:

        inlcude_path: the directory where the php include file will be stored
        this holds the database configuration as sourced from weewx.conf

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

        [[Remote]]
        This is used to transfer the include file and the database to a remote
        machine (as used in weewx.conf [FTP] or [RSYNC] section)
        rsync_user = user_name for rsync command
        rsync_machine = ip address of remote nachine

        [[RainTiming]]
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

        [[DeltaT]]
        calculate_deltaT: Whether to generate deltaT for the report page.
        Default is not to generate that data.
        This is a switch (boolean) to turn it on.

        [[PHPUnits]]
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
        self.show_warning = to_bool(self.generator.skin_dict['wxobs']['DeltaT'] \
            .get('show_warning', True))
        self.want_delta = to_bool(self.generator.skin_dict['wxobs']['DeltaT'] \
            .get('calculate_deltaT', False))
        if not self.want_delta:
            self.show_warning = to_bool(self.generator.skin_dict['wxobs'] \
            ['DeltaT'].get('show_warning', False))

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

        # used for sqlite databases transfer to remote machines
        self.rsync_user = self.generator.skin_dict['wxobs']['Remote'].get(
            'rsync_user', '')
        self.rsync_remote = self.generator.skin_dict['wxobs']['Remote'].get(
            'rsync_machine', '')
        self.log_success = to_bool(self.generator.skin_dict['wxobs']['Remote'].get(
            'log_success', True))





        # prepare the database details and write the include file
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
        # I use/prefer /tmp/wxobs_inc.inc
        inc_file = 'wxobs_incl.inc'
        inc_path = self.generator.skin_dict['wxobs'].get(
            'include_path', '/usr/share/php')
        self.include_file = ("%s/%s" % (inc_path, inc_file))

        php_inc = open(self.include_file, 'w')
        php_inc.writelines(v_al)
        php_inc.close()

        # for testing, bypass mysql lockout
        def_dbase = 'archive_sqlite'
        self.dbase = 'sqlite'
        self.sq_dbase = self.generator.config_dict['Databases'] \
            [def_dbase].get('database_name')
        self.sq_root = self.generator.config_dict['DatabaseTypes'] \
            ['SQLite'].get('SQLITE_ROOT')
        self.sqlite_db = ("%s/%s" %(self.sq_root, self.sq_dbase))

        # use rsync to transfer database remotely, ONLY if requested
        if def_dbase == 'archive_sqlite' and self.rsync_user != ''  \
                                          and self.rsync_remote != '':
            # database transfer
            #if len(self.rsync_user.strip()) > 0:
            db_loc_dir = "%s" % (self.sqlite_db)
            db_rem_str = "%s@%s:%s" % (self.rsync_user, self.rsync_remote, self.sq_root)
            rsync(self.rsync_user, self.rsync_remote, db_loc_dir, db_rem_str,
                  self.wxobs_debug, self.log_success)
            # include file transfer
            #if len(self.rsync_user.strip()) > 0:
            inc_loc_dir = "%s" % (self.include_file)
            inc_rem_str = "%s@%s:%s" % (self.rsync_user, self.rsync_remote, inc_path)
            rsync(self.rsync_user, self.rsync_remote, inc_loc_dir, inc_rem_str,
                  self.wxobs_debug, self.log_success)
