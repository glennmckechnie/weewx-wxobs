# weewx-wxobs
An additional weewx report page that uses php to extract archival data (Daily climatological summaries) from the weewx database and present it as a series of snapshots (currently half-hourly) averaged throughout the chosen day. It includes delta-T ( primarily for agricultural purposes.)

It is set out in the style of the _Latest Weather Observations_ pages that the Australian Weather Bureau - BOM provides. eg:- [Ballarat](http://www.bom.gov.au/products/IDV60801/IDV60801.94852.shtml)  I find those pages useful, that one especially when keeping an eye on the accuracy of my station.

It integrates with the seasons skin, but should be easily modified for use within other skins. It uses the cheetah generator to some extent (unit labels) but these could be bypassed if it was used as a stand-alone page.
It is configured to use either the sqlite or mysql databases that weewx uses. I don't currently use sqlite so that's a best guess (which appears to work with an archived conversion of my mysql) Feedback, corrections are welcomed on that - or anything here.

It currently (initial upload) is __only configured for Metric Units,__ but that can be changed with some tweaks (contributions welcomed). Those tweaks would for the most part be simple, except for the delta-T calculations, where a tad more work would be involved.

Thanks to:
* Powerin (weewx-users) for the initial starting point, search the group for the thread titled [Daily climatological summaries](https://groups.google.com/d/topic/weewx-user/cEAzvxv3T6Q/discussion)
* [weewx-wd](https://bitbucket.org/ozgreg/weewx-wd/wiki/Home) (by ozgreg) for the delta-T calcs in wdSearchX3.py, these are also referenced by Powerin under the weewx-users [Wet bulb and DeltaT temperatures](https://groups.google.com/d/topic/weewx-user/IoBrtQ-OL3I/discussion) post.

***Instructions:***
* Copy  the files -- datepicker.css, datepicker.js, wxobs.css -- to your weewx HTML ROOT directory. 
* Add wxobs.php.html to the Seasons skin directory (or skin of your choosing - expect some tweaking to be needed.) 
* The configuration section within wxobs.php.html exists between the __USER CONFIG AREA__ tags...
 <pre>
//
// START USER CONFIG AREA
//

// Not used?
//date_default_timezone_set('Australia/Melbourne');

// Define one database type only (mysql OR sqlite)
// Choice of sqlite (weewx default) or mysql (a weewx option)
// Uncomment grouped variables and change to suit
// for sqlite3 -- apt-get install php-sqlite3
//$dbase='sqlite';
//$sqlite_db = "/var/lib/weewx/weewx.sdb";

$dbase='mysql';
$mysql_host = "localhost";
$mysql_user = "your-mysql-username"; // CHANGE to suit yours
$mysql_pass = "your-password";     // CHANGE to suit yours
$mysql_base = "your-database-probably weewx"; // CHANGE to suit yours

// Timings for displayed values in Historical section.
// ext_interval is the spacing between records; 1800 is a half-hour.
$ext_interval = 1800;
// arch_interval is the number of records to averge over; choose a value that suits you.
//$arch_interval = $ext_interval; // this means all records for the day are involved in calcs.
$arch_interval = 60; // this only reads the last record for the interval, if matched to weewx.conf
                     // archive_interval (mine happens to be 60 seconds - change to suit yours)
                     // this becomes the equivalent of: one observation taken at that archive time.
//
// END USER CONFIG AREA
//
</pre>

* By editing the above section in wxobs.php.html, make a choice...
* If using the weewx default database of sdb - sqlite: configure the sqlite database location and comment out __//$dbase='mysql';__ then uncomment __$dbase='sqlite';__ 
* __OR__, if it's mysql archive then configure the mysql variables and check that the mysql flag is uncommented __$dbase='mysql';__
* The template is configured to give half-hourly values - change __$ext interval__ if you want something else. 
* The template is also pre-configured to return "snapshot" values; as if you had pen and paper and took a note of the readings at that time. 
  Alternatively, it can be configured to give average values over the selected time span by commenting out (__//$arch_interval = 60;__) and uncommenting $arch_interval (__$arch_interval = $ext_interval;__)
* It's intended that _$arch interval_ matches your weewx archive interval. If you have an archive interval of, say, 300 (5 minutes) then set this, to that! This is already a kludge (that does seem to work quite well)  but best not push it too far. We are aiming to look for a valid value within that time period, we'll supply 1 (not 0 or 2?).
* Modify the seasons/skin.conf by adding the __wxobs section, etc.__ to CheetahGenerator, and __, datepicker.css, wxobs.css, datepicker.js__


    [CheetahGenerator]
    
        [[ToDate]]
           
              [[[wxobs]]]
                template = wxobs.php.tmpl

    [CopyGenerator]
    
       copy_once = (leave existing values) , datepicker.css, wxobs.css, datepicker.js
       
You should be right to go.

__One warning:__  datepicker.js doesn't (didn't) play well with the seasons javascript file (seasons.js). The toggle feature on the #includes was disrupted and didn't behave as it should. I've edited datepicker.js and removed __window.onload=null;__ from the  _function onDOMReady_

Nothing seems to have broken (for me). It fixed the problem but I'm not knowledgable enough to know what side-effects I've invoked. YMMV. Expert knowledge and/or fixes welcomed.

p.s. datepicker's origins are unknown but a search of github will turn up many versions. I'll find one that matches this one and give a link - [This one](https://github.com/chrishulbert/datepicker) is very close to it.

