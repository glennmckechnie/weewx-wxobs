# weewx-wxobs
 A skin that integrates with weewx and provides a report page that uses php to extract archival data (Daily climatological summaries) from the weewx database and present it as a series of snapshots (currently half-hourly) averaged throughout the chosen day. It includes delta-T ( used for agricultural purposes. )

It is set out in the style of the _Latest Weather Observations_ pages that the Australian Weather Bureau - BOM provides. eg:- [Ballarat](http://www.bom.gov.au/products/IDV60801/IDV60801.94852.shtml)  I find those pages useful, that one especially when keeping an eye on the accuracy of my station.

It is configured to use the weewx.conf settings as its defaults. I don't use sqlite but have tried this on a simulator version with US settings and it appeatrs to work correctly. Feedback, corrections are welcomed on that - or anything here.

I've used appTemp for one of the fields, apparently not everyone has this? Consequently this is configurable via skin.conf to return windchill (or another group degree field) for those not setup to store appTemp in their databases.

It reads directly from the database so it doesn't use weewx's internal processes to massage the data to match units. It relies on the database value matching the dtabase units (US, METRIC, METRICWX) and then the [Units][[Groups]]group**** as returned by the skin.conf file being correct. Based on those fields it will attempt to ensure that delta-T uses the required Metric units to get a sensible result.

If this applies in your case, CHECK THE RESULT and confirm its working as it should. If it doesn't then report the issue and supply a fix (earn a __Legend__ star) or at the least report it (earn a __Contributor__ star) :-)

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

4. It should work after that restart (and the report cycle has run). If your database and units satisfy the requirements it will be usable without further tweaks. If the database or detected units differ from delta-T's native units then a check is required and this will be displayed on the report page, it should be unmissable!

__DON'T PANIC!__  Read it, follow the directions, make the changes (if required) and when done, turn the message off from within the skin.conf file.

, it should present its report page at weewx/wxobs/index.php.

   Read and understand the options in wxobs/skin.conf There are optional variables that you can configure and a suggestion on report timing.

5. Problems?
   Hopefully none but if there are then look at your logs - syslog and apache2/error.log. If you view them in a terminal window then you will see what's happening, as it occurs.
   
   (I find multitail -f /var/log/syslog /var/log/apache2/error.log works for me {adjust to suit your install} -- apt-get install multi-tail if needed)
   
   The error numbers reported in the apache2/error.log will refer to the index.php file in your webserver directory.
   
   If using sqlite then you may just get a blank page and an error in /var/log/apache2/error.log that says __PHP Fatal error:  Uncaught Error: Class 'SQLite3' not found__

For the current debian installation here, the following remedied that...
   <pre>
   apt-get install php-sqlite3
   
   a2enmod php7.0
   
   phpenmod sqlite3
   
   /etc/init.d/apache2 force-reload
   </pre>
   Hopefully that applies for your setup, or at least gives some direction.

6. If you run the seasons skin as your main skin ( weewx/seasons ) then the index.php file should pick up seasons.css and won't look so... ordinary?, it will also use the seasons.js so the __links.inc__ widget will work as intended.
   
   The file wxobs.inc contains the core of the php and form data. If you have a blank template for your skin (everything above and below the &lt;body&gt; &lt;/body&gt; tags) then simply copy the contents of wxobs.inc (or simply add the line #include wxobs.inc so that cheetahGenerator will do all the work) between those tags and it should work and it will be able to use your skins style (you may have to fix a few paths in the headers of the new php file)
   You'll also need to duplicate the datepicker.css, datepicker.js and wxobs.css stanzas into the new &lt;head&gt;.


7.To uninstall
   
   <pre>sudo wee_extension --uninstall wxobs</pre>
   
   and then restart weewx

   <pre>
   sudo /etc/init.d/weewx stop

   sudo /etc/init.d/weewx start
   </pre>

__Note:__  If you are using the seasons skin. datepicker.js doesn't (didn't) play well with the seasons javascript file (seasons.js). The toggle feature on the #includes was disrupted and didn't behave as it should. I've edited datepicker.js and removed __window.onload=null;__ from the  _function onDOMReady_

Nothing seems to have broken (for me). It fixed the problem but I'm not knowledgable enough to know what side-effects I've invoked. YMMV. Expert knowledge and/or fixes welcomed.

p.s. datepicker's origins are unknown but a search of github will turn up many versions. I'll find one that matches this one and give a link - [This one](https://github.com/chrishulbert/datepicker) is very close to it.

