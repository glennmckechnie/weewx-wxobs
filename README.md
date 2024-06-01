
**Update: June 2024 Version 0.8.0**

This repo has now been updated to (hopefully) deal with the ownership changes introduced in Weewx 5.x

While WeeWX can still be run as the root user, a default install (deb, pip package) now installs and sets up the application to run under the control of a non-privileged user. This maybe the user weewx or a user of your choice.

The result is that wxobs can no longer write to the /usr/share/php directory (it's well outside its scope.)

On the first run of wxobs pay close attention to the logging file. It will most likely encounter an error with permissions and will log the error in your logs - that said , an upgrade of wxobs should just need an ownership change for the include file.

Read the log messages, go to wxobs/skin.conf and follow the steps outlined under the <i>[Include File issue]</i> Section and once that has been done wxobs should then run correctly with no further issues.

This is also covered under step <b>4.</b> below.

----

<b>N.B.</b>

With the [changes made to WeeWX 5.0](https://www.weewx.com/docs/5.0/upgrade/#upgrading-to-v50) wxobs now requires a manual configuration step before it will run correctly (See step 4 below). Once that's done it should "just work" for a local installation (the sqlite database is local). For a remote installation, you may need to relocate the include file (see skin.conf) but other than that either sqlite or mysql databases will work as they did before.

The remote installation centers around rsyncing an sqlite database to the remote server. If you run an MySQL database then the required configuration for that is built in to mysql and should just require the correct variable names entering. This will vary with each setup so a single configuration example is not in my scope. If you have working notes and wish to share them, then raise them as an issue and we'll start from there.

The weewx-user group is also a starting point for queries.

Bugs reports, suggestions, feedback are always welcomed.

The bottom of this page has a list of previous changes.


# weewx-wxobs

The skin.conf file has the required configuration settings and a brief outline. The wiki page titled [Remote installation](https://github.com/glennmckechnie/weewx-wxobs/wiki/Remote-installation) has further information and trouble shooting tips.

**Description**

This is a skin that integrates with [WeeWX](http://weewx.com) (weather station software) and provides a php driven report page to extract archival data (Daily climatological summaries) from the weewx database. It then presents that information as a series of snapshots (default is half-hourly) averaged throughout the chosen day. There is an option to include appTemp, and delta-T ( used for agricultural purposes ) as well as optional configuration settings.
 
 There should be a working example available at Messmate Farms [wxobs page](http://203.213.243.61/weewx/wxobs/index.php) of which a screenshot is included below. ![alt text](https://github.com/glennmckechnie/rorpi-wiki/raw/master/weewx-wxobs-26nov2017.png "wxobs example screenshot")
If you are familiar with wview, this page would roughly equate with the **Archive Records** or **ARC** txt files that wview generated daily. In this case, it's a single page which directly queries the database and then returns the result to the user. It's a dynamic page rather than an archive of static pages.

It is set out in the style of the _Latest Weather Observations_ pages that the Australian Weather Bureau - BOM provides. eg:- [Ballarat](http://www.bom.gov.au/products/IDV60801/IDV60801.94852.shtml)  I find those pages useful, that one especially when keeping an eye on the accuracy of my station.

It is configured to use the weewx.conf settings as its defaults. It will detect and load the settings of your default database, mysql or sqlite.

I've used appTemp for one of the fields, apparently not everyone has this though? Consequently this is configurable via skin.conf and so windchill is returned as the default.   If you do have appTemp in your database then you can switch it out and use that field as originally intended.

Delta-T is also configurable but it is an an either | or selection. ie: it can be skipped completely (no replacement is offered) or configured as an additional column for those of us who may use it. It's used when spraying crops, to identify the optimal spraying conditions.

Weewx-wxobs reads directly from the database so it doesn't use WeeWX's internal processes to massage the data to match units. It relies on the database value matching the database units (US, METRIC, METRICWX) and then the [Units][[Groups]]group_x..x as returned by the skin.conf file being correct. Based on those fields it will attempt to ensure that the optional delta-T uses the required Metric units to get a sensible result.
If this applies in your case, CHECK THE RESULT and confirm its working as it should.

Thanks to:
* Powerin (weewx-users) for the initial starting point, from the thread titled [Daily climatological summaries](https://groups.google.com/d/topic/weewx-user/cEAzvxv3T6Q/discussion)
* [weewx-wd](https://bitbucket.org/ozgreg/weewx-wd/wiki/Home) (by ozgreg) for the delta-T calcs in wdSearchX3.py, these are also referenced by Powerin under the post [Wet bulb and DeltaT temperatures](https://groups.google.com/d/topic/weewx-user/IoBrtQ-OL3I/discussion) post.

***Instructions:***

1. Download the skin to your WeeWX machine.

    <pre>wget -O weewx-wxobs.zip https://github.com/glennmckechnie/weewx-wxobs/archive/master.zip</pre>

2. Change to that directory and run the WeeWX extension installer

   for the newer 5.x WeeWX versions it is now...

   <pre>weectl extension install weewx-wxobs.zip</pre>
   
   or for the older 4.x WeeWX versions it remains...

   <pre>sudo wee_extension --install weewx-wxobs.zip</pre>

3. Restart WeeWX

   <pre>
   sudo systemctl stop weewx  # ( sudo /etc/init.d/weewx stop)
   sudo systemctl start weewx  # ( sudo /etc/init.d/weewx start)
   </pre>

4. The essential Manual Configuration Step (as the privileged user)
   
   Under WeeWX 5.x it will throw 2 errors upon it's first start. These will be visible in the log. Those messages will disappear once the following fix is performed)

   You will also see the error when you run wxobs/index.php from your web browser. It will report on the missing config file and repeat the required steps to fix it.
   
   It's documented in the wxobs/skin.conf file. Because we no longer run as the privileged root user, we need to intervene and manually create the directory /usr/share/php (with permissions 0755) and copy the weewx_wxobs.inc file (that wxobs generated and wrote as /tmp/weewx_wxobs.inc) into that directory (with its permissions of 0644). If you do this as root, a straight mkdir and cp should be all that is required - the ownership and existing permissions should not need changing).
   
   In the unlikely event that those changes don't allow it to work, there is a small possibility that the path within the include file, that points to the database location is incorrect.
   
   WeeWx 5.x now writes a relative path and this is not suitable for the php script. The following snippet shows a relative path.
   <pre>
   [...]
    $php_sqlite_db = 'archive/weewx.sdb';
   </pre>
   That should be written as an absolute path, as in the following example (example only.)

   <pre>$ cat /usr/share/php/wxobs_weewx.inc
      ?php
       $php_dbase = 'sqlite';
       $php_sqlite_db = '/home/graybeard/weewx-data/archive/weewx.sdb';
   </pre>

   This is further described under the section <b>[Include File issue]</b> in wxobs/skin.conf.
   
5. This script no longer generates appTemp, nor delta-T values by default. That field is selectable within the skin.conf file. This means everything should work after that restart above (and the report cycle has run to generate the page!). You will need to check and possibly configure the displayed units to match your preferences. Instructions are in the configuration file - skin.conf.

   If you select delta-T and your database and units satisfy delta_T's requirements then it will be usable without further tweaks. More likely though, is that the database or detected units will differ from delta-T's native units and some configuration will be required. This will start with a set of instructions being displayed on the report page, it should be un-missable!


   __DON'T PANIC!__  Read it, follow the directions, make the changes (if required) and when done, turn the message off from within the skin.conf file.

   The report will be available at weewx/wxobs/index.php

   Read and understand the options available in wxobs/skin.conf There are variables that you can set/unset and a suggestion on report timing.

6. Problems?
   Hopefully none but if there are then look at your logs - syslog and apache2/error.log. If you view them in a terminal window then you will see what's happening, as it occurs.

   (I find multitail -f /var/log/syslog /var/log/apache2/error.log works for me {adjust to suit your install} -- apt-get install multi-tail if needed)

   The error numbers reported in the apache2/error.log will refer to the index.php file in your webserver directory.

   If using sqlite then you may just get a blank page and an error in /var/log/apache2/error.log that says __PHP Fatal error:  Uncaught Error: Class 'SQLite3' not found__

For the current debian installation here, the following remedied that...
   <pre>
   apt-get install php-sqlite3 
   (apt-get install php7.0-sqlite3) # or this one?

   a2enmod php7.0

   phpenmod sqlite3

   /etc/init.d/apache2 force-reload
   </pre> 
   
   For the raspberry pi here, running lighthttpd, it was a case of 
   
   
   
   <pre>
   apt-get install php-sqlite3
   apt install php-cgi
   
   lighttpd-enable-mod fastcgi-php
   
   /etc/init.d/lighttpd force-reload
   </pre>
   
   Hopefully one of those applies for your setup, or at least gives you some direction.

7. If you use this with the default WeeWX skin (Standard), it will use the weewx.css file.

If you use the Seasons skin as your main skin ( weewx/Seasons ) then the index.php file will pick up seasons.css (as well as the seasons.js) and adapt to the Seasons theme.

With a minor edit to Seasons/index.html.tmpl the __skins/wxobs/links.inc__ widget file can be used generate a link to the wxobs page.


That edit will consist of (after backing up the Seasons/index.html.tmpl file) adding the line...
<pre>
#include "../wxobs/links.inc"
</pre>
to the relevant section within Seasons/index.html.tmpl

That is:- find the widget include lines in the widget section which appears (in 4.6.0) as...
```
   <body onload="setup();">
     #include "titlebar.inc"
       <div id="contents">
        <div id="widget_group">
          #include "current.inc"
          #include "sunmoon.inc"
          #include "hilo.inc"
          #include "sensors.inc"
          #include "about.inc"
          #include "radar.inc"
          #include "satellite.inc"
          #include "map.inc"
        </div>
```
inserting that new line (for example: directly after the about.inc line) will change it to appear as follows...
```
   <body onload="setup();">
     #include "titlebar.inc"
       <div id="contents">
        <div id="widget_group">
          #include "current.inc"
          #include "sunmoon.inc"
          #include "hilo.inc"
          #include "sensors.inc"
          #include "about.inc"
          #include "../wxobs/links.inc"
          #include "radar.inc"
          #include "satellite.inc"
          #include "map.inc"
        </div>
```
and give you a working link (providing no other paths have been changed) once the next report run completes.

You're adding one line only.
It points to the wxobs/links.inc which once found will generate a link to the wxobs/index.html page.
This change can be applied while WeeWX is running, check your logs for any introduced errors (typos, path changes).

If you remove wxobs from your installation, this edit will need to be removed manually.

8. skins/wxobs/wxobs.inc

The file wxobs.inc contains the core of the php and form data. If you have a blank template for your skin (everything above and below the &lt;body&gt; &lt;/body&gt; tags) then simply copy the contents of wxobs.inc (or simply add the line #include wxobs.inc so that cheetahGenerator will do all the work) between those tags and it should work and it will be able to use your skins style (you may have to fix a few paths in the headers of the new php file)
   You'll also need to duplicate the datepicker.css, datepicker.js and wxobs.css stanzas into the new &lt;head&gt;.


9. To uninstall

   for the newer 5.x WeeWX versions it is now...

   <pre>weectl extension uninstall wxobs</pre>
   
   or for the older 4.x WeeWX versions it remains...

   <pre>sudo wee_extension --uninstall wxobs</pre>

   and then restart WeeWX

   <pre>
   sudo systemctl stop weewx  # (sudo /etc/init.d/weewx stop)
   sudo systemctl start weewx  # (sudo /etc/init.d/weewx start)
   </pre>

10. Upgrading
   
   You can use the steps from above:- step 7. to uninstall, followed by step 2. to install - to upgrade using a newly downloaded  file. 
   
   That is the safest and surest way of making sure everything gets updated. 
   
   If you wish to, you can save copies of the existing files by renaming them to something unique - eg:- append .last to the end of the filename.
   
    Anytime you use wee_extension, weewx will always need to be restarted for any changes to take affect
   
   With wewwx restarted you can then, in the vast majority of cases, make changes to a weewx skin.conf file as those changes will be re-read and actioned on the next report cycle (equivalent to your archive interval timing).
   
   If the skin.conf file suggests uncommenting or altering a variable , it's safe to assume that the next report run will act on that change.
   
   A xxx.tmpl or xxx.inc file can also be edited and re-saved while weewx is running. The results of that change will be available after the next report cycle has run.
   
   Any other file will require a restart of weewx to take affect, in particular any xxx.py file.
   
   If in doubt, restart weewx.


__Note:__  If you are using the seasons skin. datepicker.js doesn't (didn't) play well with the seasons javascript file (seasons.js) when you are using the widget side menus. The toggle feature on the #includes was disrupted and didn't behave as it should. I've edited datepicker.js and removed __window.onload=null;__ from the  _function onDOMReady_

Nothing seems to have broken (for me). It fixed the problem but I'm not knowledgable enough to know what side-effects I've invoked. YMMV. Expert knowledge and/or fixes welcomed.

p.s. datepicker's origins are unknown but a search of github will turn up many versions. I'll find one that matches this one and give a link - [This one](https://github.com/chrishulbert/datepicker) is very close to it.

# Previous Changes

**Update: Feb 2024**

Weewx is now at version 5.x

While WeeWX can still be run as the root user, a default install (deb package) now installs and sets up the application to run under the user weewx.

This means that wxobs can no longer write to the /usr/share/php directory. The simplest fix is to change that directory to /var/tmp where weewx can write the file and the apache2 webserver can read it.

That change has been enabled in the wxobs/skin.conf file.

**Update: June 2022**

Rémy L has contributed a french language file (lang/fr.conf) along with some language related bug fixes (missing lang fields).

Currently available as: [Internationalization (English & French)  - version 0.7.8](https://github.com/glennmckechnie/weewx-wxobs/releases/tag/v0.7.8)


**Update: March 2022**

Add instructions to generate "Daily Summaries (wxobs)" link in Seasons main page (See point 6 below) using the skin/wxobs/links.inc file
Clean up old files.

Currently available as the release: [Internationalization and point 6! - version 0.7.7](https://github.com/glennmckechnie/weewx-wxobs/releases/tag/v0.7.7)


**Update: Feb 2022**

***Internationalization.***

****!! Argh - bug fix - now at v0.7.6 !!****

With the release of WeeWX 4.6.0 comes the ability to add language translations.
_skins/wxobs/lang/en.conf_ now exists as a template for those who wish to personalize it.

The weewx [localization documents](https://weewx.com/docs/customizing.htm#localization) provides more details.


This gives the user the ability to have languages other than English. The files won't exist here until someone with the required language skill does them, and shares them.

The english file exists (en.conf), along with a test file (xx.conf) which can be renamed and used as a starting point. It's what I've used to catch all relevant strings (or at least I assume that I've got them all!)
If you go to the effort and translate a file for your locale, please consider contributing it back to this project for others to use. It will be appreciated.

As always, if you have any queries then contact me and I'll help where possible.

Available as [pre-release v0.7.6](https://github.com/glennmckechnie/weewx-wxobs/releases/tag/v0.7.6) or as master.zip from the master branch.

**Update: Sept 2020**  ---  A minor change to note that weewx.conf takes priority over any skin.conf entries. See [How options work](http://weewx.com/docs/customizing.htm#How_options_work) in the Customizing Guide for the full details.  This change is required for the label logic when manually changing units to something other than what is in the weewx database.  If you have an installation that is already displaying the labels as you want them, then this update is not required.

Available as [release 0.7.3](https://github.com/glennmckechnie/weewx-wxobs/releases)

**Update: Jun 2020**  ---  Available as [release 0.7.2](https://github.com/glennmckechnie/weewx-wxobs/releases)

Add weewx4 style logging

**Update: Jan 2020**

Runs with python2.7 or python3 ie:- runs under weewx4.

Add logic to detect and create /usr/share/php directory which seems to be missing from some installations, even though phpinfo.php shows it as being available, or required.

**Update: July 2018**  ---  Available as [release 0.7.0](https://github.com/glennmckechnie/weewx-wxobs/releases)
Simplify single/average output

**Update: March 2018**  ---  Available as [release 0.6.5](https://github.com/glennmckechnie/weewx-wxobs/releases)
1. The 'remote' branch has now been merged with the master branch. (bumped to version 0.6.5, with its bugfix! )
2. This means that the changes required to install weewx-wxobs on a remote server, and transfer the sqlite database have been incorporated into the current version. 
3. This version has been configured to work with multiple instances of weewx (providing the databases are uniquely named!)
It allows you, the user, to specify the remote path for the database and also for the include file. This should make it easier to resolve any permission errors from the remote webservers end.

**Update: Jan 2018** Rsync is now included as an option if you are 
1. Transferring the web data to a remote server.
2. Using the sqlite database.
3. Able to setup password-less access for ssh, an example of which [is here](http://github.com/weewx/weewx/wiki/Using-the-RSYNC-skin-as-a-backup-solution#Create_the_passwordless_access)

When configured it will transfer the database to the same location (/var/lib/weewx to /var/lib/weewx) on the remote machine (or see version 0.6.3 for other options). This will hopefully be achievable at the remote end. If that is not the case, due to permissions, accessibility, etc then the location will need to be changed to something more suitable within weewx.conf ie: *SQLITE_ROOT = /a_directory/you_can_access/remotely* however the default should work well, but you may need to create the directory first.
It also transfers the include file to its equivalent location. The include file can be relocated from the default location to somewhere, perhaps more suitable (usually required due to permission problems.) (Other options now exist - see version 0.6.3)

**Update: 18th Jan 2018**
A quirk in checkdate can allow strings to be passed through to the underlying code. Lame injection attempts highlighted the need for this trivial fix so we now do a better job of sanitizing the checkdate input.
All was fine before, but it's even better now.

**Update: 30th Sept 2017**
With security still uppermost, the script now checks that the datepicker returns a valid date. If it does find a problem it displays a warning and falls back to using the current day instead. Anything the datepicker generates obviously (eg: 01-Sep-2017) passes the test. Short number format (01-09-2017) will also pass.

Experimenting with ext-interval shows that the report will generate every archive record for the day, down to 60 seconds, without too much of a performance hit, although YMWV (it's not the intent of this skin to do that, but it is a useful side effect). With this in mind, there is now a page generation time string at the bottom of the page so you can check just how much load is involved, although watching your weewx logs for problems is the best and surest way. 
This can prove useful when checking for errant records - NOAA can flag the odd rain? days, wxobs can nail the day, your skills with the database can erase the evidence ;-)

**Update: 24th Sept 2017**
Merged the convert branch into the master branch. It now incorporates unit conversions into the php script. It may still be a bit rough around the edges at the moment but it shouldn't break. Testing and feedback is welcomed.

Also switched the australian rain option to accommodate a range of start times - eg : 7 a.m. is 25200 seconds (France?)

Version bump to 0.05

**Update: 20th Sept 2017**

This skin now uses php includes to import critical values into the report page. This should reduce the risk of those details being exposed inadvertently. Hopefully it will be a drop in replacement, no matter which webserver or vendor configuration you have!

Australian rain has now been included as an option (see skin.conf where it's off by default). This is where rain is recorded over the 24 hours between 9 a.m each day

Version bump to 0.04
