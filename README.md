

# weewx-wxobs
 A skin that integrates with [weewx](http://weewx.com) (weather station software) and provides a php driven report page to extract archival data (Daily climatological summaries) from the weewx database. It then presents that information as a series of snapshots (default is half-hourly) averaged throughout the chosen day. There is an option to include appTemp, and delta-T ( used for agricultural purposes ) as well as optional  configuration settings
 
If you are familiar with wview, this page would equate with the **Archive Records** or **ARC** txt files that wview generated daily. In this case it's only one page and the results are generated directly from the database. It's a dynamic page rather than an archive of static pages.

It is set out in the style of the _Latest Weather Observations_ pages that the Australian Weather Bureau - BOM provides. eg:- [Ballarat](http://www.bom.gov.au/products/IDV60801/IDV60801.94852.shtml)  I find those pages useful, that one especially when keeping an eye on the accuracy of my station.

It is configured to use the weewx.conf settings as its defaults. It will detect and load the settings of your default database, mysql or sqlite. 

I've used appTemp for one of the fields, apparently not everyone has this though? Consequently this is configurable via skin.conf and so windchill is returned as the default.   If you have appTemp in your database then you can switch it out and use that field. 
Delta-T is also configurable but is an an either, or selection. ie: it can be skipped completely (no replacement is offered) or configured as an additional column for those of us who may use it.

Weewx-wxobs reads directly from the database so it doesn't use weewx's internal processes to massage the data to match units. It relies on the database value matching the database units (US, METRIC, METRICWX) and then the [Units][[Groups]]group**** as returned by the skin.conf file being correct. Based on those fields it will attempt to ensure that the optional delta-T uses the required Metric units to get a sensible result.
If this applies in your case, CHECK THE RESULT and confirm its working as it should. 

**Of Note:** Because it directly access's the weewx database, it won't work remotely. If you use FTP or RSYNC to transfer the web pages to a remote server then you lose the direct database connection. (Which is probably not a bad thing from a security point of view?) It will always work from the local server though.

Having said that it won't work remotely, there is a SLE here [weewx-sqlitedupe](https://github.com/glennmckechnie/weewx-sqlitedupe) that I use to duplicate my local database and I send that, using RSYNC, to the web facing weewx install where the wxobs skin there, can access it.

Thanks to:
* Powerin (weewx-users) for the initial starting point, from the thread titled [Daily climatological summaries](https://groups.google.com/d/topic/weewx-user/cEAzvxv3T6Q/discussion)
* [weewx-wd](https://bitbucket.org/ozgreg/weewx-wd/wiki/Home) (by ozgreg) for the delta-T calcs in wdSearchX3.py, these are also referenced by Powerin under the post [Wet bulb and DeltaT temperatures](https://groups.google.com/d/topic/weewx-user/IoBrtQ-OL3I/discussion) post.

***Instructions:***

1. Download the skin to your weewx machine.

    <pre>wget -O weewx-wxobs.zip https://github.com/glennmckechnie/weewx-wxobs/archive/master.zip</pre>

2. Change to that directory and run the wee_extension installer

   <pre>sudo wee_extension --install weewx-wxobs.zip</pre>

3. Restart weewx

   <pre>
   sudo /etc/init.d/weewx stop

   sudo /etc/init.d/weewx start
   </pre>

4. This script no longer generates appTemp, nor delta-T values by default. They are selectable within the skin.conf file. This means everything should work after that restart above (and the report cycle has run to generate the page!). You will need to check and possibly configure the displayed units to match your preferences. Instructions are in the configuration file - skin.conf.
If you select delta-T and your database and units satisfy delta_T's requirements then it will be usable without further tweaks. More likely though, is that the database or detected units will differ from delta-T's native units and some configuration will be required. This will start with a set of instructions being displayed on the report page, it should be un-missable!


__DON'T PANIC!__  Read it, follow the directions, make the changes (if required) and when done, turn the message off from within the skin.conf file.

   The report will be available at weewx/wxobs/index.php

   Read and understand the options available in wxobs/skin.conf There are variables that you can set/unset and a suggestion on report timing.

5. Problems?
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
   apt-get install php5-sqlite
   
   /etc/init.d/lighttpd restart
   </pre>
   
   Hopefully one of those applies for your setup, or at least gives you some direction.

6. If you use it with the default weewx skin, it will use the weewx.css file.

If you run the seasons skin as your main skin ( weewx/seasons ) then the index.php file should pick up seasons.css and won't look so... ordinary?, it will also use the seasons.js so the __links.inc__ widget will work as intended.


   The file wxobs.inc contains the core of the php and form data. If you have a blank template for your skin (everything above and below the &lt;body&gt; &lt;/body&gt; tags) then simply copy the contents of wxobs.inc (or simply add the line #include wxobs.inc so that cheetahGenerator will do all the work) between those tags and it should work and it will be able to use your skins style (you may have to fix a few paths in the headers of the new php file)
   You'll also need to duplicate the datepicker.css, datepicker.js and wxobs.css stanzas into the new &lt;head&gt;.


7.To uninstall

   <pre>sudo wee_extension --uninstall wxobs</pre>

   and then restart weewx

   <pre>
   sudo /etc/init.d/weewx stop

   sudo /etc/init.d/weewx start
   </pre>

__Note:__  If you are using the seasons skin. datepicker.js doesn't (didn't) play well with the seasons javascript file (seasons.js) when you are using the widget side menus. The toggle feature on the #includes was disrupted and didn't behave as it should. I've edited datepicker.js and removed __window.onload=null;__ from the  _function onDOMReady_

Nothing seems to have broken (for me). It fixed the problem but I'm not knowledgable enough to know what side-effects I've invoked. YMMV. Expert knowledge and/or fixes welcomed.

p.s. datepicker's origins are unknown but a search of github will turn up many versions. I'll find one that matches this one and give a link - [This one](https://github.com/chrishulbert/datepicker) is very close to it.

# Changes

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
