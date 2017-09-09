## Copyright 2009-2017 Tom Keffer, Matthew Wall
## Distributed under terms of GPLv3.  See LICENSE.txt for your rights.
#errorCatcher Echo
## Specifying an encoding of UTF-8 is usually safe, but if the text is actually
## in Latin-1, then you should replace the string "UTF-8" with "latin-1".  If
## you do this, you should also change the charset in 'Content-Type' as well.
#encoding UTF-8
## php inclusion to create wxobs.php.tmpl by glenn.mckechnie@gmail.com
## available at https://github.com/glennmckechnie/weewx-wxobs
## 06-09-2017 initial releas @0.01
## 08-09-2017 convert to installable skin
## 08-09-2017 add unit check and conversion for deltaT
## Version 0.03

   <div>
      <p class="contents">
      Use the following to select a date for archived records
      </p>
    <!-- insert the javascript date picker -->
      <form name="form" method="POST" action="index.php">
      Choose the date:
      <input id="start_dt" name="datepicker" class="datepicker">
      <input type="submit" name="submitDate" value="Show records">
      </form>
   </div>
<?php

$debug=0;

//
// START USER CONFIG AREA
//

// Not used?
//date_default_timezone_set('Australia/Melbourne');
// only valid for date(); use mktime(); instead

// Argh! Another FIXME?
// Unit adjustment for my wind. Database is in m/s but I need km/h
// 0.277778 is needed.
$wind_adjust='1';
//
// END USER CONFIG AREA
//

//Note; double quoted variables are values that will be passed from the SLE,
// via cheetahGenerator. They are required after substitution.

if ('$dbase' === 'mysql') {
  if ($debug==='1'){
    echo "database = $dbase<br>";
   }
   $db = new mysqli('$mysql_host', '$mysql_user', '$mysql_pass', '$mysql_base');
    if ($db->connect_errno) {
     echo ("<p>Connection to mysql database failed:<br>
            Has it been configured in wxobs.php.tmpl?<br>
     $db->connect_error</p>");
     return;
    }
   }elseif ('$dbase' === 'sqlite'){
    if ($debug==='1'){
     echo "database = $dbase<br>";
    }
    $db = new SQLite3('$sqlite_db');
   }else{
   echo "Define a database with the template - wxobs.inc<br>";
}
// Attempt to get some meaingful table headings, if we change above options
if ( '$arch_interval' === '$ext_interval' ) {
 $tflag="Total";
 $avflag="(Average)";
 $iflag=" (averaged over each interval)";
}else{
 $tflag="";
 $avflag="";
 $iflag=" (as single readings)";
}

if ( '$appTemp' === 'appTemp') {
 $appLabel = 'App Temp';
 $app_label = '$unit.label.appTemp';
}elseif ( '$appTemp' === 'windchill') {
 $appLabel = 'Wind Chill';
 $app_label = '$unit.label.windchill';
}else{
 $appLabel = '$appTemp';
 $app_label = '$unit.label.outTemp';
}

function ordinalCompass ($degr){
 // templated from weewx code. Mistakes are mine!
global $compass;
$default_ordinate_names = array ('N', 'NNE','NE', 'ENE', 'E', 'ESE', 'SE', 'SSE',
                          'S', 'SSW','SW', 'WSW', 'W', 'WNW', 'NW', 'NNW',
                            'N/A'); //17 objects
 if ( $degr === 0 ) {
  $compass = "CALM";
  return $compass;
 }

 $sector_size = 360/16;
 $degree = ($degr + $sector_size/2.0) % 360.0;
 $sector = intval($degree / $sector_size);
 $compass = $default_ordinate_names[$sector];
}

function deltaT ($units, $Tc,$RH,$P) {
 global $dT;
 // DeltaT calculations
 // sourced from wdSearchX3.py, part of the weewx-wd package by oz greg
 // available at https://bitbucket.org/ozgreg/weewx-wd/wiki/Home
 //$Tc = outTemp.degree_C
 //$RH = outHumidity
 //$P = pressure.hPa
 // The following is really an AND statement. It assumes your database, & your
 // assigned units are in agreeance. Perhaps it should ignore the database
 // and trust the user?
 // Comment out the database check for the moment. Cycles are cheap these days?
//if ('$units' === '1' ) {
 if ('$unit.label.outTemp' === "&#176;F") {
   $Tc=($Tc-32)*5/9; // assuming degF - return degC
 }
 if ('$unit.label.barometer' === ' hPa') { //leading space is critical!
     $P=$P;
 }elseif ('$unit.label.barometer' === ' inHg') { //leading space is critical!
     $P=($P*33.86389); //assuming inHg - return hPa
 }elseif ('$unit.label.barometer' === ' mmHg') { //leading space is critical!
     $P=($P/0.75006168); //assuming mmHg - return hPa
 }else{
     //echo "Unrecognized pressure units - conversion code is not present";
     $dT="?units?";
     return;
 }
//}


 $Tdc = (($Tc - (14.55 + 0.114 * $Tc) * (1 - (0.01 * $RH)) - ((2.5 + 0.007 * $Tc) * (1 - (0.01 * $RH))) ** 3 - (15.9 + 0.117 * $Tc) * (1 - (0.01 * $RH)) ** 14));
 $E = (6.11 * 10 ** (7.5 * $Tdc / (237.7 + $Tdc)));
 $WBc = (((0.00066 * $P) * $Tc) + ((4098 * $E) / (($Tdc + 237.7) ** 2) * $Tdc)) / ((0.00066 * $P) + (4098 * $E) / (($Tdc + 237.7) ** 2));
// $WetBulb = "%.1f" % $WBc + $unit.label.outTemp
// $WetBulb = "%.1f" % ($WBc*9/5+32) + $unit.label.outTemp // degrees F
//echo "Tdc = $Tdc E = $E WBc = $WBc :: T=$Tc : RH=$RH : P=$P<br>";
//$DeltaT = $Tc - $WBc;
 $dT = abs(round($Tc - $WBc,1));

}

$tr_bgrnd="";


// Start of Latest observations
// Shows the latest record and the rainfall total
   $day_start = mktime('0', '0', '0', date('n'), date('j'), date('Y'));
   $now_time = mktime();
//   echo " date = $day_start<br> mktime = $now_time<br>";
//??   $endtime = $now_time + 79800;
//   $endtime = $day_start + 86400;
   $endtime = $now_time;

// Now redundant use cheetahGen
//// A quick check, with warning, re units. Currently it's metric only.
//   $unit_check = $db->query("SELECT usUnits FROM archive WHERE dateTime = $now_time ");
//
//if ('$dbase' === 'mysql') {
//   $unit_check = $unit_check->fetch_array();
//   }else{
//   $unit_check = $unit_check->fetchArray();
//}
//   $units = $unit_check[0];
   if ('$targ_unit' === 'US' ) {
   //return; // uncomment the start of this line if the next message is to be ignored.
   $units = '1';
    echo "<p>weewx reports that the database is in imperial (US) units.<br>
          <br>????<br>
          Adjustments will be made depending on the weewx label returned by this skin.<br>
          The deltaT calc requires Metric units and an adjustment will be made.<br>
          &nbsp;&nbsp;&nbsp;<b>Check the results!</b><br>
          </p>";
   }else{
   $units = '0';
   }

   $last_entry = $db->query("SELECT dateTime FROM archive ORDER BY datetime DESC LIMIT 1");
if ('$dbase' === 'mysql') {
   $last_entry = $last_entry->fetch_array();
   }else{
   $last_entry = $last_entry->fetchArray();
}
   $latest_date = $last_entry[0];

if ('$dbase' === 'mysql') {
   $last_values = $db->query("SELECT FROM_UNIXTIME(datetime, '%Y-%m-%d %H:%i'), outTemp, appTemp, dewPoint, outHumidity, windSpeed, windGust, windDir, barometer, rain FROM archive WHERE dateTime = $latest_date");
   $latest_row = $last_values->fetch_array();
   }else{
   $last_values = $db->query("SELECT datetime(min(dateTime), 'unixepoch', 'localtime'), outTemp, appTemp, dewPoint, outHumidity, windSpeed, windGust, windDir, barometer, rain FROM archive WHERE dateTime = $latest_date");
   $latest_row = $last_values->fetchArray();
}

if ('$dbase' === 'mysql') {
   $rain_total = $db->query("SELECT sum(rain) FROM archive WHERE dateTime >= $day_start and dateTime < $endtime;");
   $rain_row = $rain_total->fetch_array();
   }else{
   $rain_total = $db->query("SELECT sum(rain) FROM archive WHERE dateTime >= $day_start and dateTime < $endtime;");
   $rain_row = $rain_total->fetchArray();
   }
   if ($debug==='1'){
    echo "Nowtime = $day_start : Endtime = $endtime<br>";
   }
echo "<!-- start data tables -->\n
<h1>Latest Weather Observations</h1>
<table class=\"wxdata\">
  <thead>
  <tr>
   <th class=\"wxheadr\" rowspan=\"2\">Date&nbsp;&nbsp;&nbsp;&nbsp;Time</th>
   <th class=\"wxheadr\" rowspan=\"2\">Out Temp<br>$unit.label.outTemp</th>
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   <th class=\"wxheadr\" rowspan=\"2\">Dew Point<br>$unit.label.dewpoint</th>
   <th class=\"wxheadr\" rowspan=\"2\">Humidity<br>$unit.label.outHumidity</th>
   <th class=\"wxheadr\" rowspan=\"2\">delta-T<br>$unit.label.outTemp</th>
   <th class=\"wxheadr\" colspan=\"4\">Wind</th>
   <th class=\"wxheadr\" rowspan=\"2\">Pressure<br>$unit.label.barometer</th>
   <th class=\"wxheadr\" rowspan=\"2\">Total Rain<br>$unit.label.rain</th>
  </tr>
  <tr>
   <th class=\"wxheadr\" class=\"wxheadr\">Direction<br>compass</th>
   <th class=\"wxheadr\">Speed<br>$unit.label.wind</th>
   <th class=\"wxheadr\">Gust<br>$unit.label.wind</th>
   <th class=\"wxheadr\">Direction<br>$unit.label.windDir</th>
  </tr>
 </thead><tbody>";

   $hdate = $latest_row[0];
   echo "<tr><td> $hdate<!-- $latest_date --></td>";
   $metTemp = round($latest_row[1], 1);
   echo "<td>$metTemp</td>";
   $metappTemp = round($latest_row[2], 1);
   echo "<td>$metappTemp</td>";
   $metdew = round($latest_row[3], 1);
   echo "<td>$metdew</td>";
   $humid = round($latest_row[4], 1);
   echo "<td>$humid</td>";
   deltaT ($units, $latest_row[1], $latest_row[4], $latest_row[8]);
   echo "<td>$dT</td>";
   ordinalCompass ($latest_row[7]);
   echo "<td>$compass</td>"; //wind direction ordinal
   $metWind = round(($latest_row[5]/$wind_adjust), 1); //weewx database is METRICWX, value is stored as m/s
   echo "<td>$metWind</td>";
   $metGust = round(($latest_row[6]/$wind_adjust), 1);//weewx database is METRICWX, value is stored as m/s
   echo "<td>$metGust</td>";
   $windDir = round($latest_row[7], 0);
   echo "<td>$windDir</td>"; //wind direction degrees
   $metPress = round($latest_row[8], 1);
   echo "<td>$metPress</td>";
   $metRain = round($rain_row[0], 2);
   echo "<td>$metRain</td></tr>";
   echo "</tbody></table>\n<!-- end latest observations -->\n<br><hr>";

// Min Max summaries
// Shows the min and max values, averages and rainfall total
   //    $unixtime =  strtotime($_REQUEST['datepicker']);

   if ( isset( $_REQUEST['datepicker'])) {
       $unixtime =  strtotime($_REQUEST['datepicker']);
       }else{
   //    $unixtime = $now_time;
       $unixtime = $day_start;
       }

   if ($debug==='1'){
    echo "Datepicker returns = $unixtime : Request start = $day_start<br>";
    }
// sanity check because dates from the future are pointless (and untidy)
// $latest_date holds the latest time from the first (above) observations.
   //if ( $unixtime > $latest_date) {
   if ( $unixtime > $latest_date) {
   if ($debug==='1'){
    echo "test1: $unixtime > $latest_date <br>";
    }
  echo "<p>That is a future date; there are no records!<br> (Wait a while? or best try again. :)</p>";
//// not work with backdated start time   return;
   return;
   }
   if ($debug==='1'){
    echo "test2: $unixtime > $latest_date <br>";
    }
   $starttime = $unixtime;
   if ($debug==='1'){
    echo "test3: $unixtime < $starttime <br>";
    }
   $endtime = $starttime + 79800;
//   return;

if ('$dbase' === 'mysql') {
// error
   $stats_snap = $db->query("SELECT FROM_UNIXTIME(datetime, '%Y-%m-%d %H:%i'),min(outTemp), max(outTemp), min(appTemp), max(appTemp), min(dewPoint), max(dewPoint), min(outHumidity), max(outHumidity), avg(windSpeed), avg(windGust), avg(windDir), avg(barometer), sum(rain) FROM archive WHERE dateTime >= $starttime and dateTime < $endtime;");
   $stats_row = $stats_snap->fetch_array();
   }else{
   $stats_snap = $db->query("SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),min(outTemp), max(outTemp), min(appTemp), max(appTemp), min(dewPoint), max(dewPoint), min(outHumidity), max(outHumidity), avg(windSpeed), avg(windGust), avg(windDir), avg(barometer), sum(rain) FROM archive WHERE dateTime >= $starttime and dateTime < $endtime;");
   $stats_row = $stats_snap->fetchArray();
}
   $hdate = $stats_row[0];
   if ($hdate === '') {
   echo "No data for this date.<br>";
    return;
   }
   $this_day =  $_REQUEST['datepicker'];

  echo "<!-- start snapshot summary observations -->

 <h1>Historical Observations for $this_day</h1>
 <h2>Min (Max) Stats</h2>
 <table class=\"wxdata\">
  <thead>
  <tr>
   <th class=\"wxheadr\" rowspan=\"2\">Date&nbsp;&nbsp;&nbsp;&nbsp; Time</th>
   <th class=\"wxheadr\" rowspan=\"2\">Out Temp<br>$unit.label.outTemp</th>
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   <th class=\"wxheadr\" rowspan=\"2\">Dew Point<br>$unit.label.dewpoint</th>
   <th class=\"wxheadr\" rowspan=\"2\">Humidity<br>$unit.label.outHumidity</th>
   <th class=\"wxheadr\" colspan=\"4\">Wind</th>
   <th class=\"wxheadr\" rowspan=\"2\">Pressure<br>$unit.label.barometer</th>
   <th class=\"wxheadr\" rowspan=\"2\">Total Rain<br>$unit.label.rain</th>
  </tr>
  <tr>
   <th class=\"wxheadr\" class=\"wxheadr\">Direction<br>compass</th>
   <th class=\"wxheadr\">Speed<br>$unit.label.wind</th>
   <th class=\"wxheadr\">Gust<br>$unit.label.wind</th>
   <th class=\"wxheadr\">Direction<br>$unit.label.windDir</th>
  </tr>
 </thead><tbody>";

   echo "<tr><td> $hdate<!-- $starttime-$endtime --></td>";
   $minTemp = round($stats_row[1], 1);
   $maxTemp = round($stats_row[2], 1);
   echo "<td>$minTemp ($maxTemp)</td>";
   $minappTemp = round($stats_row[3], 1);
   $maxappTemp = round($stats_row[4], 1);
   echo "<td>$minappTemp ($maxappTemp)</td>";
   $mindew = round($stats_row[5], 1);
   $maxdew = round($stats_row[6], 1);
   echo "<td>$mindew ($maxdew)</td>";
   $minhumid = round($stats_row[7], 1);
   $maxhumid = round($stats_row[8], 1);
   echo "<td>$minhumid ($maxhumid)</td>";
   ordinalCompass ($stats_row[11]);
   echo "<td> $compass</td>"; //wind direction ordinal
   $metWind = round(($stats_row[9]/$wind_adjust), 1);//weewx database is METRICWX, value is stored as m/s;
   echo "<td>$metWind</td>";
   $metGust = round(($stats_row[10]/$wind_adjust), 1);//weewx database is METRICWX, value is stored as m/s
   echo "<td>$metGust</td>";
   $windDir = round($stats_row[11], 0);
   echo "<td>$windDir</td>"; //wind direction degrees
   $metPress = round($stats_row[12], 1);
   echo "<td>$metPress</td>";
   $metRain = round($stats_row[13], 2);
   echo "<td>$metRain</td></tr>";
   echo "</tbody></table>\n<!-- end snapshot summary observations -->\n";


// Half-hourly, daily stats
// Shows the average half-hourly values for a given (arch_interval) snapshot, averages and rainfall total for that period.
  echo "<!-- start snapshot observations -->
 <h2>Periodic Stats $iflag</h2>
 <table class=\"wxdata\">
 <thead>
  <tr>
   <th class=\"wxheadr\" rowspan=\"2\">Date&nbsp;&nbsp;&nbsp;&nbsp; Time</th>
   <th class=\"wxheadr\" rowspan=\"2\">Out Temp<br>$unit.label.outTemp</th>
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   <th class=\"wxheadr\" rowspan=\"2\">Dew Point<br>$unit.label.dewpoint</th>
   <th class=\"wxheadr\" rowspan=\"2\">Humidity<br>$unit.label.outHumidity</th>
   <th class=\"wxheadr\" rowspan=\"2\">delta-T<br>$unit.label.outTemp</th>
   <th class=\"wxheadr\" colspan=\"4\">Wind $avflag </th>
   <th class=\"wxheadr\" rowspan=\"2\">Pressure<br>$unit.label.barometer</th>
   <th class=\"wxheadr\" rowspan=\"2\">$tflag Rain<br>$unit.label.rain</th>
  </tr>
  <tr>
   <th class=\"wxheadr\" class=\"wxheadr\">Direction<br>compass</th>
   <th class=\"wxheadr\">Speed<br>$unit.label.wind</th>
   <th class=\"wxheadr\">Gust<br>$unit.label.wind</th>
   <th class=\"wxheadr\">Direction<br>$unit.label.windDir</th>
  </tr>
 </thead><tbody>";

   // extract data for each ext_interval - reverse order (latest at top)
  //$starttime = $unixtime + ($t*3600);
  //$endtime = $unixtime + 3600 + ($t*3600);

   $c_ycle=(86400/$ext_interval)-1;
  // $xtra_time = $arch_interval + ($c_ycle*$ext_interval);
   for($t=$c_ycle;$t>=0;$t--)
   {
   $starttime = $unixtime + ($t*'$ext_interval');
   $endtime = $starttime + '$arch_interval';

if ('$dbase' === 'mysql') {
   $results = $db->query("SELECT FROM_UNIXTIME(datetime, '%Y-%m-%d %H:%i'), avg(outTemp), avg(appTemp), avg(dewPoint), avg(outHumidity), AVG(windSpeed), AVG(windGust), AVG(windDir), avg(barometer), sum(rain) FROM archive WHERE dateTime >= $starttime and dateTime < $endtime;");
   $row = $results->fetch_array();
   }else{
   $results = $db->query("SELECT datetime(min(dateTime), 'unixepoch', 'localtime'), avg(outTemp), avg(appTemp), avg(dewPoint), avg(outHumidity), avg(windSpeed), avg(windGust), avg(windDir), avg(barometer), total(rain) FROM archive WHERE dateTime>= $starttime and dateTime< $endtime;");
   $row = $results->fetchArray();
}
   if ($tr_bgrnd != "wxhint") {
     $tr_bgrnd = "wxhint"; 
     } else {
     $tr_bgrnd = "wxwht";
     }
   $hdate = $row[0];
   if ($hdate != "" ) {
   echo "<tr class=\"$tr_bgrnd\"><td> $hdate <!-- $starttime-$endtime --></td>";
   $metTemp = round($row[1], 1);
   echo "<td>$metTemp</td>";
   $metappTemp = round($row[2], 1);
   echo "<td>$metappTemp</td>";
   $metdew = round($row[3], 1);
   echo "<td>$metdew</td>";
   $humid = round($row[4], 1);
   echo "<td>$humid</td>";
   deltaT ($units, $row[1], $row[4], $row[8]);
   echo "<td>$dT</td>";
   ordinalCompass ($row[7]);
   echo "<td>$compass</td>"; //wind direction ORDINAL
   $metWind = round(($row[5]/$wind_adjust), 1);//weewx database is METRICWX, value is stored as m/s
   echo "<td>$metWind</td>";
   $metGust = round(($row[6]/$wind_adjust), 1);//weewx database is METRICWX, value is stored as m/s
   echo "<td>$metGust</td>";
   $windDir = round($row[7], 0);
   echo "<td>$windDir</td>"; //wind direction degrees
   $metPress = round($row[8], 1);
   echo "<td>$metPress</td>";
   $metRain = round($row[9], 2);
   echo "<td>$metRain</td></tr>";
   }
   }
   echo "</tbody></table><!-- end snapshot observations -->";

   ?>
   </div>
