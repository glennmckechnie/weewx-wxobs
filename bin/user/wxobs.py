# Copyright (c) 2017-2020 Glenn McKechnie <glenn.mckechnie@gmail.com>
# Credit to Tom Keffer <tkeffer@gmail.com>, Matthew Wall and the core
# weewx team, all from whom I've borrowed heavily.
# Mistakes are mine, corrections and or improvements welcomed
#    https://github.com/glennmckechnie/weewx-wxobs
#
# rsync code based on weeutil/rsyncupload.py by
#    Will Page <companyguy@gmail.com> and
#
# See the file LICENSE.txt for your full rights.
#
#
# added text

import subprocess
import time
import errno
import os

import weewx.engine

from weeutil.weeutil import to_bool
from weewx.cheetahgenerator import SearchList

wxobs_version = "0.7.8"

try:
    # weewx4 logging
    import weeutil.logger
    import logging
    log = logging.getLogger(__name__)

    def logdbg(msg):
        log.debug(msg)

    def loginf(msg):
        log.info(msg)

    def logerr(msg):
        log.error(msg)

except ImportError:
    # old-style weewx logging
    import syslog

    def logmsg(level, msg):
        syslog.syslog(level, 'wxobs: %s' % msg)

    def logdbg(msg):
        logmsg(syslog.LOG_DEBUG, msg)

    def loginf(msg):
        logmsg(syslog.LOG_INFO, msg)

    def logerr(msg):
        logmsg(syslog.LOG_ERR, msg)


def wxrsync(rsync_user, rsync_server, rsync_options, rsync_loc_file,
            rsync_loc_file2, rsync_ssh_str, rem_path, wxobs_debug,
            log_success):
    """
    rsync_user
    rsync_server
    rsync_options
    rsync_loc_file
    rsync_loc_file2 # maybe empty
    rsync_ssh_str
    rem_path
    wxobs_debug
    log_success
    """

    t_1 = time.time()
    # construct the command argument
    cmd = ['rsync']
    cmd.extend([rsync_options])

    # cmd.extend(["-tOJrl"])
    # provide some stats on the transfer
    cmd.extend(["--stats"])
    cmd.extend(["--compress"])
    cmd.extend([rsync_loc_file])
    cmd.extend([rsync_loc_file2])
    cmd.extend([rsync_ssh_str])

    try:
        # perform the actual rsync transfer...
        if wxobs_debug == 2:
            loginf("rsync cmd is ... %s" % (cmd))
        # rsynccmd = subprocess.Popen(cmd, stdout=subprocess.PIPE,
        #                             stderr=subprocess.STDOUT, close_fds=True)
        rsynccmd = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT)
        stdout = rsynccmd.communicate()[0]
        stroutput = stdout.decode("utf-8").strip()
        # rsyncpid = rsynccmd.pid
        # loginf("      pre.wait rsync pid is %s" % rsyncpid)
        # rsynccmd.wait()
        # rsyncpid = rsynccmd.pid
        # loginf("     post.wait rsync pid is %s" % rsyncpid)
        # subprocess.call( ('ps', '-l') )
    except OSError as e:
        # print "EXCEPTION"
        if e.errno == errno.ENOENT:
            logerr("rsync does not appear to be installed on this system. \
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
                n_ber = rsyncinfo['Number of regular files transferred']
            else:
                n_ber = rsyncinfo['Number of files transferred']

            n_bytes = rsyncinfo['Total file size']
            n_sent = rsyncinfo['Literal data']
            if n_ber is not None and n_bytes is not None:
                rsync_message = ("rsync'd %s of %s files (%s) in "
                                 "%%0.2f seconds" % (n_sent, n_ber, n_bytes))
            else:
                rsync_message = "rsync executed in %0.2f seconds"
            # loginf("%s " % (rsync_message))
        except:
            rsync_message = ("rsync exception raised:"
                             "executed in %0.2f seconds")
            loginf(" ERR %s " % (rsync_message))
    else:
        # suspect we have an rsync error so tidy stroutput
        # and display a message
        stroutput = stroutput.replace("\n", ". ")
        stroutput = stroutput.replace("\r", "")
        # Attempt to catch a few errors that may occur and deal with them
        # see man rsync for EXIT VALUES
        rsync_message = ("rsync command failed after %0.2f secs (set"
                         "set 'wxobs_debug = 2' in skin.conf),")
        if "code 1)" in stroutput:
            if wxobs_debug == 2:
                logerr("rsync code 1 - %s" % stroutput)
            rsync_message = ('syntax error in rsync command'
                             '- set debug = 1 - ! FIX ME !')
            loginf(" ERR %s " % (rsync_message))
            rsync_message = ("code 1, syntax error, failed"
                             " rsync executed in %0.2f seconds")

        elif ("code 23" and "Read-only file system") in stroutput:
            # read-only file system
            # sadly, won't be detected until after first succesful transfer
            # but it's useful then!
            if wxobs_debug == 2:
                logerr("rsync code 23 - %s" % stroutput)
            loginf("ERR Read only file system ! FIX ME !")
            rsync_message = ("code 23, read-only, rsync failed"
                             " executed in %0.2f seconds")
        elif ("code 23" and "link_stat") in stroutput:
            # likely to be that a local path doesn't exist - possible typo?
            if wxobs_debug == 2:
                logdbg("rsync code 23 found %s" % stroutput)
            rsync_message = ("rsync code 23 : is %s correct?"
                             "! FIXME !" % (rsync_loc_file))
            loginf(" ERR %s " % rsync_message)
            rsync_message = ("code 23, link_stat, rsync failed"
                             " executed in %0.2f seconds")
        elif "code 11" in stroutput:
            # directory structure at remote end is missing - needs creating
            # on this pass. Should be Ok on next pass.
            if wxobs_debug == 2:
                loginf("rsync code 11 - %s" % stroutput)
            rsync_message = ("rsync code 11 found Creating %s"
                             " as a fix?, space issue?" % (rem_path))
            loginf("%s" % rsync_message)
            # laborious but apparently necessary, the only way the command
            # will run!? build the ssh command - n.b:  spaces cause wobblies!
            cmd = ['ssh']
            cmd.extend(["%s@%s" % (rsync_user, rsync_server)])
            mkdirstr = "mkdir -p"
            cmd.extend([mkdirstr])
            cmd.extend([rem_path])
            if wxobs_debug == 2:
                loginf("sshcmd %s" % cmd)
            subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            rsync_ssh_str = rem_path
            rsync_message = ("code 11, rsync mkdir cmd executed"
                             " in % 0.2f seconds")

        elif ("code 12") and ("Permission denied") in stroutput:
            if wxobs_debug == 2:
                logdbg("rsync code 12 - %s" % stroutput)
            rsync_message = ("Permission error in rsync command, probably at"
                             " remote end authentication ! FIX ME !")
            loginf(" ERR %s " % (rsync_message))
            rsync_message = "code 12, rsync failed, executed in % 0.2f seconds"
        elif ("code 12") and ("No route to host") in stroutput:
            if wxobs_debug == 2:
                logdbg("rsync code 12 - %s" % stroutput)
            rsync_message = "No route to host error in rsync command ! FIX ME!"
            loginf(" ERR %s " % (rsync_message))
            rsync_message = "code 12, rsync failed, executed in % 0.2f seconds"
        else:
            logerr("rsync [%s] reported this error: %s" % (cmd, stroutput))

    if log_success:
        if wxobs_debug == 0:
            t_o = ''
            rsync_ssh_str = ''
        else:
            t_o = ' to '
        t_2 = time.time()
        loginf("%s" % rsync_message % (t_2-t_1) + t_o + rsync_ssh_str)


class wxobs(SearchList):

    def __init__(self, generator):
        SearchList.__init__(self, generator)

        """
        This is a minimal SLE - all we want is to make the php configuration
        easier by transferring the database values as used by weewx, plus a few
        other variables.
        It has since expanded to allow the transfer of the sqlite database
        by using rsync, providing the [Remote] section of the config is
        populated

        In wxobs/skin.conf:

        send_inc: An option to stop sending the include file/s. These contain
        the database configuration values, timezone and oprional debugging
        stanzas for the php script to operate. Needless to say you need to
        send them at least once.
        If you can't think of a reason why you'd need this then you don't need
        to implement it.
        I run a mysql database locally and export an sqlite to the remote. This
        allows me to do that without too much trouble (remembering to set up
        the symlinks is the biggest issue)

        include_path: the directory where the php include file will be stored
        this holds the database configuration as sourced from weewx.conf
        If you store them locally you can change that path using this option.
        If you send the files to another server you can change this path using
        dest_directory (which will affect the database also.)

        disp_interval: is the time between displayed records; 1800 is a
        half-hour and is the default

        display_type: 'single' (default) or 'average'
        Whether to return the timestamped entry for the value displayed in the
        periodic Stats table (single), or average it over all the database
        entries between it and the previous one that was displayed. (average)

        app_Temp: This is a recent addition to weewx and is not enabled by
        default. The calculation is performed but there is no field in the
        stock database. This variable allows for the substitution with another
        value.
        The default is to use windchill.
        Keep it to the group_degrees (because the label is hard coded in.)

        timezone: If the date or time is not being displayed correctly
        we'll assume it's a php issue and pass a timezone string to the script.
        This can be done by adding your time zone as the following example
        indicates.  Replace the string with your zone description
        timezone = Melbourne/Australia

        self.php_error: enable php error messages in index.php.
        This maybe useful at the start of configuration, it shouldn't be
        needed after everything is running smoothly
        default is False. set to True to enable this to be sent via the
        include file.

        [[Remote]]
        This is used when you want to transfer the include file and the
        database to a remote machine where the web files have been sent
        seperately with the weewx.conf [FTP] or [RSYNC] section.
        dest_directory: is the switch that turns this o. It transfers BOTH the
        include and database files to the same directory as the tuple specifies
        If using multiple databases and include files make sure they are unique
        ie:-  if you are transferring from multiple machine.
        It will fetch the rsync user and server from the wxobs/skin.conf file
        and use those values or if they are missing then it will use the values
        from the [RSYNC] section of weewx.conf which is possibly configured
        already.
        rsync_user (user) = user_name for rsync command
        rsync_server (server)= ip address of the remote machine
        send_include = True #This is the default, set to False if you don't
        want to send the include file repeatedly to the server. Use with
        caution (ie: remember this setting when things stop working, it might
        be the cure)
        rsync_options: Not documented in the skin.conf Default is '-ac'. Use
        with caution and no spaces allowed.

        [[RainTiming]]
        shift_rain: For rain accounting times other than midnight to midnight
        set this to True
        If no other options are given the accounting time will be the
        australian rain day which starts at 9 a.m.
        default is false - start at midnight 00:00:00 through to the next day.

        rain_start: used to shift time (in seconds) to something other than
        9a.m. The default is 32400

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
        2 is for wxobs remote cmds etc.
        3 only for delta-T final values (low level - if enabled)
        4 only for delta-T unit conversion calcs (verbose) - if enabled
        5 only for ordinalCompass conversion calcs (N, NE...CALM) (verbose)
        6 is for database debugging

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

        self.send_inc = to_bool(self.generator.skin_dict['wxobs'].get(
            'send_include', True))
        # self.inc_path = self.generator.skin_dict['wxobs'].get(
        #    'include_path', '/usr/share/php')

        # intervals for display of results
        self.disp_interval = self.generator.skin_dict['wxobs'].get(
            'display_interval', '1800')
        self.arch_interval = self.generator.config_dict['StdArchive'].get(
            'archive_interval')
        # now decide whether intermediate reading will be averaged or ignored.
        self.display_type = self.generator.skin_dict['wxobs'].get(
            'display_type', 'single')
        if self.display_type == 'single':
            self.disp_single = to_bool(True)
            # loginf("              %s single readings, %s " % (
            #   self.disp_single, self.display_type))
        elif self.display_type == 'average':
            self.disp_single = to_bool(False)
            self.arch_interval = self.disp_interval
            # loginf("              %s average readings, %s " % (
            #    self.disp_single, self.display_type))
        else:
            # loginf("                reverting to single readings,
            #    %s is not an option" % self.display_type)
            self.disp_single = to_bool(True)
        self.app_temp = self.generator.skin_dict['wxobs'].get(
            'app_Temp', 'windchill')
        self.php_zone = self.generator.skin_dict['wxobs'].get(
            'timezone', '')
        self.php_error = to_bool(self.generator.skin_dict['wxobs'].get(
            'show_php_errors', False))
        self.show_warning = to_bool(self.generator.skin_dict['wxobs']['DeltaT']
                                    .get('show_warning', True))
        self.want_delta = to_bool(self.generator.skin_dict['wxobs']['DeltaT']
                                  .get('calculate_deltaT', False))
        if not self.want_delta:
            self.show_warning = to_bool(self.generator.skin_dict['wxobs']
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

        self.shift_rain = to_bool(self.generator.skin_dict['wxobs']
                                  ['RainTiming'].get('shift_rain', False))
        # 32400 (rainday_start) == 9 hours == 9 a.m.
        self.rainday_start = self.generator.skin_dict['wxobs']['RainTiming'].get(
            'rain_start', '32400')
        # 32400 == 9 hours == 9 (start_label) a.m.
        self.start_label = self.generator.skin_dict['wxobs']['RainTiming'].get(
            'start_label', '9')

        # target_unit = METRICWX # Options are 'US', 'METRICWX', or 'METRIC'
        self.targ_unit = self.generator.config_dict['StdConvert'].get(
            'target_unit')

        # used for rsync of sqlite databases and include file to remote machines
        self.dest_dir = self.generator.skin_dict['wxobs']['Remote'].get(
            'dest_directory', '')
        if self.dest_dir:
            self.rsync_user = self.generator.skin_dict['wxobs']['Remote'] \
                .get('rsync_user', '')
            if not self.rsync_user:
                try:
                    self.rsync_user = self.generator.config_dict['StdReport'] \
                        ['RSYNC'].get('user', '')
                except:
                    if self.wxobs_debug >= 1:
                        logdbg("No rsync _user supplied?")

            self.rsync_server = self.generator.skin_dict['wxobs']['Remote'] \
                .get('rsync_server', '')
            if not self.rsync_server:
                try:
                    self.rsync_server = self.generator.config_dict['StdReport'] \
                        ['RSYNC'].get('server', '')
                except:
                    if self.wxobs_debug >= 1:
                        logdbg("No rsync server supplied?")
            # did we get anything that we can use?
            if not self.rsync_user or not self.rsync_server:
                self.dest_dir = ''
            else:
                # we did so we need these...
                self.rsync_options = self.generator.skin_dict['wxobs'] \
                    ['Remote'].get('rsync_options', '-ac')
                self.log_success = to_bool(self.generator.skin_dict['wxobs']
                                           ['Remote'].get('log_success', True))
                pass


        # prepare the database details and write the include file
        def_dbase = self.generator.config_dict['DataBindings'] \
            ['wx_binding'].get('database')
        if self.wxobs_debug == 5:
            logdbg("database is %s" % def_dbase)
#########################
# BEGIN TESTING ONLY:
# For use when testing sqlite transfer when a mysql database is the default
# archive
# Our normal mode of operation is False - ie: don't change a bloody thing!
# It won't be mentioned in the skin.conf description. You'll need to have seen
# this to know the switch exists!
        test_sqlite = to_bool(self.generator.skin_dict['wxobs']['Remote'].get(
            'test_withmysql', False))
        if test_sqlite:
            def_dbase = 'archive_sqlite'
# END TESTING ONLY:
#########################
        if def_dbase == 'archive_mysql':
            self.dbase = 'mysql'
            self.mysql_base = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            id_match = self.mysql_base
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
            if self.wxobs_debug == 6:
                loginf("mysql database is %s, %s, %s, %s" % (
                    self.mysql_base, self.mysql_host,
                    self.mysql_user, self.mysql_pass))
        elif def_dbase == 'archive_sqlite':
            self.dbase = 'sqlite'
            self.sq_dbase = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')
            id_match = self.sq_dbase[:-4]
            self.sq_root = self.generator.config_dict['DatabaseTypes'] \
                ['SQLite'].get('SQLITE_ROOT')

            self.sqlite_db = ("%s/%s" % (self.sq_root, self.sq_dbase))
            v_al = ["<?php\n $php_dbase = 'sqlite';\n $php_sqlite_db = '%s';\n" %
                    self.sqlite_db]

            if self.wxobs_debug == 6:
                loginf("sqlite database is %s, %s, %s" % (
                    self.sq_dbase, self.sq_root, self.sqlite_db))

        # phpinfo.php shows include_path as .:/usr/share/php, we'll put it
        # in there and hopefully that will work for most users.
        # I use/prefer /tmp/wxobs_inc.inc

        inc_file = ("wxobs_%s.inc" % id_match)
        if self.dest_dir != '':
            # create an empty index.html to obscure directory listing
            self.zero_html = self.dest_dir+"/index.html"
            if not os.path.exists(self.dest_dir):
                os.makedirs(self.dest_dir, mode=0o0755)
            if not os.path.isfile(self.zero_html):
                with open(self.zero_html, 'a') as z:  # Create file if does not exist
                    pass  # and auto close it
            # we are rsyncing remotely
            # And going to change all the remote paths, the include_path has
            # lost its precedence.
            self.inc_path = self.dest_dir
            self.include_file = ("%s/%s" % (self.inc_path, inc_file))
            # pre-empt inevitable warning/exception when using
            #    test_sqlite = False
            self.sq_dbase = self.generator.config_dict['Databases'] \
                [def_dbase].get('database_name')

            new_location = (self.dest_dir+"/"+self.sq_dbase)
            v_al = ["<?php\n $php_dbase = 'sqlite';\n $php_sqlite_db = '%s/%s';" %
                    (self.dest_dir, self.sq_dbase)]

            # symlink database to new location here, which will be mirrored on the
            # remote serve. This rallows local usage of wxobs as well as remote
            org_location = (self.sq_root+"/"+self.sq_dbase)
            if not os.path.isfile(new_location):
                if self.wxobs_debug == 2:
                    loginf("database, attempting to \'symlink %s %s\'"
                           % (org_location, new_location))
                try:
                    os.symlink(org_location, new_location)
                except OSError as e:
                    logerr("error creating database symlink %s" % e)

            try:
                if not os.access(self.include_file, os.W_OK):
                    os.makedirs(self.inc_path)
            except OSError as e:
                if e.errno == os.errno.EEXIST:
                    pass
        else:
            # All other cases, local or remote...
            # we are going to retain the defaults values, maybe a slight tweak.
            # use the skin.conf include_path, either default or the override.
            self.inc_path = self.generator.skin_dict['wxobs'].get(
                'include_path', '/usr/share/php')
            # phpinfo.php include_path is referenced but missing in some
            #  cases - php7.3? Possibly installed with php-pear ?
            # FIXME: a quick and harmless fix is to create it.
            if not os.path.exists(self.inc_path):
                os.makedirs(self.inc_path, mode=0o0755)
                loginf("Created %s" % self.inc_path)
            self.include_file = ("%s/%s" % (self.inc_path, inc_file))

        # if self.send_inc and self.dest_dir != '':
        if self.send_inc:
            php_inc = open(self.include_file, 'w')
            php_inc.writelines(v_al)
            if self.php_zone != '':
                t_z = ("\n ini_set(\"date.timezone\", \"%s\");" % self.php_zone)
                if self.wxobs_debug == 2:
                    loginf("timezone is set to %s" % t_z)
                php_inc.write(t_z)
            if self.php_error:
                php_err = ('\n ini_set(\'display_errors\', 1);'
                           '\n ini_set(\'display_startup_errors\', 1);'
                           '\n error_reporting(E_ALL);')
                if self.wxobs_debug == 2:
                    loginf("php error reporting is set: %s" % php_err)
                php_inc.writelines(php_err)
            php_inc.close()

        # use rsync to transfer database remotely, but ONLY if requested
        if def_dbase == 'archive_sqlite' and self.dest_dir != '':
            # honor request to move destination directories (same for both)
            # create and redefine as appropriate
            if self.dest_dir:
                self.sq_root = self.dest_dir
            # database transfer
            db_loc_file = "%s" % (self.sqlite_db)
            db_ssh_str = "%s@%s:%s/" % (self.rsync_user, self.rsync_server,
                                        self.sq_root)
            wxrsync(self.rsync_user, self.rsync_server, self.rsync_options,
                    db_loc_file, self.zero_html, db_ssh_str, self.sq_root,
                    self.wxobs_debug, self.log_success)

            if self.send_inc:
                # perform include file transfer if wanted, zero_html just
                # fills a slot here.
                inc_loc_file = "%s" % (self.include_file)
                inc_ssh_str = "%s@%s:%s/" % (self.rsync_user, self.rsync_server,
                                             self.inc_path)
                wxrsync(self.rsync_user, self.rsync_server, self.rsync_options,
                        inc_loc_file, self.zero_html, inc_ssh_str,
                        self.inc_path, self.wxobs_debug, self.log_success)
