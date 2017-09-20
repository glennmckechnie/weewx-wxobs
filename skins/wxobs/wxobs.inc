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
## 13-09-2017 move sensitive info to an include file in /tmp, rather than have it
##            embedded within index.php. Don't show links.inc in index.php as it
##            breaks the flow and is redundant there. center all but debug info.
## 18-09-2017 Add aus_rain where the 24 hour rainfall count starts at 9 a.m.
## Version 0.04

   <!-- wxobs version $wxobs_version -->
   <!-- This weewx extension is available at github: https://github.com/glennmckechnie/weewx-wxobs -->
   <div class="wxobs">
      Use the following to select a date for archived records
      <!-- insert the javascript date picker -->
      <form name="form" method="POST" action="index.php">
      Choose the date:
      <input id="start_dt" name="datepicker" class="datepicker">
      <input type="submit" name="submitDate" value="Show records">
      </form>
   </div>
<?php

if ((@include '$include_file') === false) {
    echo "<div class=\"alert\">";
    echo "<p>The database config file has gone missing!<br>";
    echo "&nbsp;&nbsp;&nbsp;That shouldn't happen.</p>";
    echo "We are using the php default for includes of /usr/share/php<br>";
    echo "The file should have been accessable as<br> <i>$include_file</i><br>";
    echo "It will be regenerated on weewx's next report run,";
    echo " or when report_timing allows.<br>";
    echo "If the problem persists, consider changing";
    echo " its location as ";
    echo "wxobs/skin.conf will accept<br>";
    echo "<i>include_file = /new_path/name</i><br>";
    echo "where the path and name can be anything";
    echo " allowable by php.ini and preferably secure.<br>";
    echo "That means keeping it away from the webserver, if possible.<br>";
    echo "If all else fails, report the problem as an issue";
    echo " - to its github site.</p></div>";
}
require '$include_file';
/**
* ToDo ??
* stackoverflow.com/questions/9866139/database-locked-while-trying-to-access-from-php-script
*  $db->exec('PRAGMA journal_mode = wal;');
*
* Debugging:
* 1 and above is low level, variables, some logic.
* 3 only for delta-T final values (low level - if enabled)
* 4 only for delta-T unit conversion calcs (verbose) - if enabled
* 5 only for ordinalCompass conversion calcs (N, NE...CALM) (verbose)
*/

//function hDate($d)
//{
//        $h_read = date('d-M-Y H:i', $d);
//        return $h_read;
//}

function hDate($d)
{
        $h_read = date('d-M H:i', $d);
        return $h_read;
}

function ordinalCompass($degr, $debug)
{
    /*
    * Templated from weewx code. Mistakes are mine!
    */
    global $compass;
    $default_ordinate_names = array ('N', 'NNE','NE', 'ENE', 'E', 'ESE', 'SE', 'SSE',
                          'S', 'SSW','SW', 'WSW', 'W', 'WNW', 'NW', 'NNW',
                            'N/A'); //17 objects
    if ($debug == '5') {
        echo "<p class=\"debug\">degr passed as '$degr'<br>";
    }
    if ($degr == 0) {
        $compass = "CALM";
        if ($debug == '5') {
            echo "so compass is $compass</p>";
        }
        return $compass;
    }

    $sector_size = 360/16;
    $degree = ($degr + $sector_size/2.0) % 360.0;
    $sector = intval($degree / $sector_size);
    $compass = $default_ordinate_names[$sector];
    if ($debug == '5') {
        echo "so degree is $degree, sector is $sector: results in compass $compass</p>";
    }
}

function deltaT($Tc, $RH, $P, $debug)
{
    /*
    * DeltaT calculations
    * sourced from wdSearchX3.py, part of the weewx-wd package by oz greg
    * available at https://bitbucket.org/ozgreg/weewx-wd/wiki/Home
    * $Tc = outTemp.degree_C
    * $RH = outHumidity
    * $P = pressure.hPa
    */
    global $dT;
    if ('$unit.label.outTemp' === '&#176;F') {
         $Tc=($Tc-32)*5/9; // assuming degF - return degC
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed degF: Tc has become $Tc</p>";
        }
    } else {
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed degC: Tc remains $Tc</p>";
        }
    }
    if ('$unit.label.barometer' === ' hPa') { //leading space is critical!
        $P=$P;
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assuming hPa: P remains $P</p>";
        }
    } elseif ('$unit.label.barometer' === ' inHg') { //leading space is critical!
        $P=($P*33.86389); //assuming inHg - return hPa
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed inHg: P has become $P</p>";
        }
    } elseif ('$unit.label.barometer' === ' mmHg') { //leading space is critical!
        $P=($P/0.75006168); //assuming mmHg - return hPa
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed mmHg: P has become $P</p>";
        }
    } else {
        //echo "Unrecognized pressure units - conversion code is not present";
        $dT="?units?";
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br>($P) returned, assuming nothing: P has become $P</p>";
        }
        return;
    }

    $Tdc = (($Tc - (14.55 + 0.114 * $Tc) * (1 - (0.01 * $RH)) -
        ((2.5 + 0.007 * $Tc) * (1 - (0.01 * $RH))) ** 3 -
        (15.9 + 0.117 * $Tc) * (1 - (0.01 * $RH)) ** 14));
    $E = (6.11 * 10 ** (7.5 * $Tdc / (237.7 + $Tdc)));
    $WBc = (((0.00066 * $P) * $Tc) + ((4098 * $E) / (($Tdc + 237.7) ** 2)
        * $Tdc)) / ((0.00066 * $P) + (4098 * $E) / (($Tdc + 237.7) ** 2));
    // $WetBulb = "%.1f" % $WBc + $unit.label.outTemp
    // $WetBulb = "%.1f" % ($WBc*9/5+32) + $unit.label.outTemp // degrees F
    //echo "Tdc = $Tdc E = $E WBc = $WBc :: T=$Tc : RH=$RH : P=$P<br>";
    //$DeltaT = $Tc - $WBc;
    $dT = abs(round($Tc - $WBc, 1));
}


/*
* Note; double quoted variables are values that will be passed from the SLE,
* via cheetahGenerator. They are required to be in place for the substitution
* to work with php.
*/

$debug='$wxobs_debug';
$tr_bgrnd="";

// Attempt to get some meaningful table headings, dependent on xxx_interval options
if ('$arch_interval' === '$ext_interval') {
    $tflag="Total";
    $avflag="(Average)";
    $iflag=" (averaged over each interval)";
} else {
    $tflag="";
    $avflag="";
    $iflag=" (as single readings)";
}

// then define a bunch of long strings for use in <th> @ 3 locations

if ($php_dbase === 'mysql') {
    $st_date = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
              "Date (in day-month-year format) and Time.\">".
              "Date&nbsp;&nbsp;&nbsp;&nbsp; Time</a></th>";
} elseif ($php_dbase === 'sqlite') {
    $st_date = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
              "Date (in year-month-day format) and Time.\">".
              "Date&nbsp;&nbsp;&nbsp;&nbsp; Time</a></th>";
}
$st_temp ="<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
          "Outside temperature\">Out Temp<br>$unit.label.outTemp</a></th>";
$st_dew = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
          "Temperature where water droplets can condense and dew forms.\">".
          "Dew Point<br>$unit.label.dewpoint</a></th>";
$st_rh = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
         "Relative Humidity\">Humidity<br>$unit.label.outHumidity</a></th>";
$st_wind = "<th class=\"wxheadr\" colspan=\"4\"><a title=\"".
           "Wind stats as averaged over each period.\">Wind $avflag </a></th>";
$st_baro = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
           "Barometric pressure\">Pressure<br>$unit.label.barometer</a></th>";
$st_rain = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
           "Total rainfall for each interval\n starting from nominated time.\">";
$st_comp = "<th class=\"wxheadr\"><a title=\"Direction the wind comes from as\n".
           " cardinal (compass) values\">Direction<br>compass</a></th>";
$st_spd = "<th class=\"wxheadr\">Speed<br>$unit.label.wind</th>";
$st_gust = "<th class=\"wxheadr\">Gust<br>$unit.label.wind</th>";
$st_degr = "<th class=\"wxheadr\"><a title=\"Direction the wind comes from in ".
           "degrees\">Direction<br>$unit.label.windDir</a></th>";

if ($php_dbase === 'mysql') {
    if ($debug>='1') {
        echo "<p class=\"debug\">database is $dbase with the name: $mysql_base</p>";
    }
    $db = new mysqli(
        $php_mysql_host,
        $php_mysql_user,
        $php_mysql_pass,
        $php_mysql_base
    );
    if ($db->connect_errno) {
        echo ("<p class=\"alert\">Connection to mysql database failed:<br>
            Has it been configured correctly - these were the values sent<br>
            host=$php_mysql_host,user=$php_mysql_user,
            pass=$php_mysql_pass,base=$php_mysql_base<br>
     $db->connect_error</p>");
        return;
    }
} elseif ($php_dbase === 'sqlite') {
    if ($debug>='1') {
        echo "<p class=\"debug\">database is $dbase with path/name: $sqlite_db</p>";
    }
    $db = new SQLite3($php_sqlite_db);
} else {
    echo "<p class =\"alert\">Failed to access the database by any known methods.<br>";
}


if ('$appTemp' === 'appTemp') {
    $appLabel = "<a title=\"Apparent Temperature\">App Temp";
    $app_label = "$unit.label.appTemp</a>";
} elseif ('$appTemp' === 'windchill') {
    $appLabel = 'Wind Chill';
    $app_label = '$unit.label.windchill';
} else {
    $appLabel = '$appTemp';
    $app_label = '$unit.label.outTemp';
}

if ($debug>='1') {
    echo "<p class=\"debug\">ext_interval is $ext_interval <br> arch_interval is $arch_interval<br>
         appTemp is defined as $appTemp with $app_label used as the units.</p>";
}
if ($debug>='1') {
    echo "<p class=\"debug\">want delta is $want_delta with show warning of $show_warning<br>
         ('$targ_unit' === 'US' || '$unit.label.outTemp' !== '&#176;C' ||
         '$unit.label.barometer' !== ' hPa')<br>
         then, if one of the above is (true) show the warning &lt;div&gt;</p>";
}

if ($show_warning) {
    if ('$targ_unit' === 'US' || '$unit.label.outTemp' !== '&#176;C'
        || '$unit.label.barometer' !== ' hPa'
    ) {
        echo "<div class=\"alert\">
              <h2>delta-T calculation: configuration notes</h2>
              <p>Weewx reports that the database is in $targ_unit (imperial) units,
              and /or that the temperature, pressure units differ from the &#176;C or
              hPa required to calculate the delta-T values correctly. This may be
              compensated for but we need to be sure that what's being reported is
              correct.</p>
              <p>Review your wxobs/skin.conf file and match the group_fields with
              your database.
              <pre>
              [Units]
                  [[Groups]]
                      group_temperature
                      group_pressure
                      group_speed #check it anyway, delta-T doesn't use it but...
              </pre>
              Whatever units are used in the database needs to be reflected
              in the skin.conf file.</p>
              <p>ie:-<br>
              If you know the database stores the temperature in &#176;C then you
              need to specify it<br>
              group_temperature  = degree_C<br> The same logic applies if it's
              &#176;F - like for like.</p>
              <p>If you know the database stores the pressure in hPa then you need
              to specify it (or inHg, mmHG or other) <br>
              group_pressure     = hPa</p>
              <p>The code will compensate correctly for &#176;C, &#176;F, hPa, inHg,
              mmHg.  BUT only if you tell it in the skin.conf file (or blind luck
              favours you!)<br>
              (Others units aren't accounted for, they'll need to be added to the
              code.)</p>
              <p>&nbsp;&nbsp;&nbsp; Then: <b>Check the results!</b></p>
              <p>Finally, when all is working to your satisfaction, you can turn
              this message off manually by uncommenting the line<br>
              <b>#show_warning = False</b><br>
              in that same config file.<br>
              The message will also dissappear if the correct units (degC and hPa)
              are detected.
              </p></div>";
    }
}

// Start of Latest observations
// Shows the latest record and the rainfall total
    $day_start = mktime('0', '0', '0', date('n'), date('j'), date('Y'));
    $now_time = mktime();
    $endtime = $now_time;
if ($debug>='1') {
    echo "<p>Latest Observations:<br>day_start = ", hDate($day_start),
     "($day_start)<br> endtime uses the now_time value ", hDate($endtime),
     " ( $endtime )</p>";
}

    $last_entry = $db->query(
        "SELECT dateTime FROM archive ORDER BY datetime DESC LIMIT 1"
    );

    if ($php_dbase === 'mysql') {
        $last_entry = $last_entry->fetch_array();
    } else {
        $last_entry = $last_entry->fetchArray();
    }
      $latest_date = $last_entry[0];

    if ($aus_rain) {
        // Calculate start day for rain calcs - Australian records start at 9 a.m.
        $start_nine = $day_start + 32400; // 9 am today
        $start_yday = $day_start - 54000; // 9 am yesterday
        if ($now_time <= $start_nine) {
            $rain_start = $start_yday;
            $rain_end = $rain_start + 86400 + $arch_interval;
            $t_span = "(since 9 a.m.)";
            $td_rain = "<td style=\"background-color:#aeaeae\" title=\"".
                       "Previous Total over 24 hours\nstarting from ".
                       hDate($rain_start). "\">";
            if ($debug >= 1) {
                echo "Latest: nowtime=".hdate($now_time)." : ".hDate($day_start).
                 " ($day_start day_start < start_nine $start_nine) ".
                 hDate($start_nine)." : rain_start = ".hDate($rain_start).
                 " : rain_end ".hDate($rain_end)."<br>";
            }
        } elseif ($now_time > $start_nine) {
            $rain_start =$start_nine;
            $rain_end = $rain_start + 86400 + $arch_interval;
            $t_span = "(since 9 a.m.)";
            $td_rain = "<td style=\"background-color:#d6d6d6\" title=\"".
                       "Current Total over 24 hours\nstarting from ".
                       hDate($rain_start). "\">";
            if ($debug >= 1) {
                echo "Latest: nowtime=".hDate($now_time)." : ".hDate($day_start).
                 " ($day_start day_start >= start_nine $start_nine) ".
                 hDate($start_nine)." : rain_start = ".hDate($rain_start).
                 " : rain_end ".hDate($rain_end)."<br>";
            }
        }
    } else {
        $rain_start = $day_start;
        $rain_end = $endtime;
        $t_span = "(since 12 a.m.)";
        $td_rain = "<td title=\"24 hour rain starting from midnight\n on this day: ".
                   hDate($rain_start). "\">";
        if ($debug >= 1) {
            echo "Latest: 24 hours ".hDate($day_start)." ($day_start day_start
                 & rain_end $rain_end) ". hDate($rain_end)." : rain_start = "
                 .hDate($rain_start)."<br>";
        }
    }

    if ($php_dbase === 'mysql') {
        $last_values = $db->query(
            "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'), outTemp,
            $appTemp, dewPoint, outHumidity, windSpeed, windGust, windDir,
            barometer FROM archive WHERE dateTime = $latest_date"
        );
        $rain_total = $db->query(
            "SELECT sum(rain) FROM archive WHERE dateTime
            >= $rain_start and dateTime < $rain_end;"
        );
        $latest_row = $last_values->fetch_array();
        $rain_row = $rain_total->fetch_array();
    } else {
        $last_values = $db->query(
            "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
            outTemp, $appTemp, dewPoint, outHumidity, windSpeed, windGust,
            windDir, barometer FROM archive WHERE dateTime = $latest_date"
        );
        $rain_total = $db->query(
            "SELECT sum(rain) FROM archive WHERE dateTime
            >= $rain_start and dateTime < $rain_end;"
        );
        $latest_row = $last_values->fetchArray();
        $rain_row = $rain_total->fetchArray();
    }

    if ($debug>='1') {
        //echo "latest date = $latest_date : day_start = $day_start : endtime = $endtime<br>";
        echo "latest date = ", hDate($latest_date), " : day_start = ",
             hDate($day_start), " : endtime = ", hDate($endtime), "<br>";
             echo "latest rain_start = ", hDate($rain_start), " : ($rain_start) rain_end = ",
                  hDate($rain_end), "($rain_end)";
    }

    // Finish Latest grunt work
    //////////////////////////
    // Build Latest Obs html

    if ($want_delta) {
        $delta_label = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"Used as an".
                       " indicator for agricultural spraying conditions, the ideal".
                       " conditions are between the values of 2 &amp; 8\">".
                       "delta&dash;T<br>$unit.label.outTemp</a></th>";
        if ($debug>='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so label is...<br>
                 $delta_label";
        }
    } else {
        $delta_label="";
        if ($debug>='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so label is...<br>
                 '$delta_label'";
        }
    }


    /*    if (date_default_timezone_get()) {
    *       echo 'date_default_timezone_set: ' . date_default_timezone_get() . '<br />';
    *       $systemTimeZone = system('date +%Z');
    *   echo "System TimeZone is $systemTimeZone <br>";
    *   }
    */

    echo "<div class=\"wxdata\">\n<!-- start data tables -->\n
 <H2><A Title=\"Todays latest weather observations, at the displayed time.\">Latest Observations</a></h2>
<table class=\"wxdata\">
  <thead>
  <tr>
   $st_date \n   $st_temp
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   $st_dew \n   $st_rh
   $delta_label
   $st_wind \n   $st_baro
   $st_rain $tflag Rain<br>$t_span<br>$unit.label.rain</a></th>
  </tr>
  <tr>
   $st_comp \n   $st_spd \n   $st_gust \n   $st_degr
  </tr>
 </thead><tbody>";

    if ($debug >= 1) {
        $st_day = hDate($latest_date);
        $st_day = "$st_day($latest_date)";
    } else {
        $st_day = "";
    }


    $hdate = $latest_row[0];
    echo "<tr><td> $hdate<!-- $st_day --></td>";
    $metTemp = round($latest_row[1], 1);
    echo "<td>$metTemp</td>";
    $metappTemp = round($latest_row[2], 1);
    echo "<td>$metappTemp</td>";
    $metdew = round($latest_row[3], 1);
    echo "<td>$metdew</td>";
    $humid = round($latest_row[4], 1);
    echo "<td>$humid</td>";
    if ($want_delta) {
        deltaT($latest_row[1], $latest_row[4], $latest_row[8], $debug);
        echo "<td>$dT</td>";
        if ($debug=='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
            echo "delta_data is '$dT'</p>";
        }
    } else {
        if ($debug=='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
            echo "delta_data is 'null'</p>";
        }
    }
    ordinalCompass($latest_row[7], $debug);
    echo "<td>$compass</td>"; //wind direction ordinal
    $metWind = round(($latest_row[5]/$wind_adjust), 1);
    echo "<td>$metWind</td>";
    $metGust = round(($latest_row[6]/$wind_adjust), 1);
    echo "<td>$metGust</td>";
    $windDir = round($latest_row[7], 0);
    echo "<td>$windDir</td>"; //wind direction degrees
    $metPress = round($latest_row[8], 1);
    echo "<td>$metPress</td>";
    $metRain = round($rain_row[0], 2);
    echo "$td_rain $metRain</td></tr>";
    echo "</tbody></table>\n<!-- end latest observations -->\n<br><hr>";

    // Finished Latest Obs
    /////////////////////////////
    // Min Max summaries

    // Shows the min and max values, averages and rainfall total
    if ($_REQUEST['datepicker']) {
        $unixtime =  strtotime($_REQUEST['datepicker']);
        if ($debug>='1') {
            echo "<p class=\"debug\">Min (Max)<br> Datepicker returned ",
                  hDate($unixtime), " ($unixtime) </p>";
        }
    } else {
        $unixtime = $day_start;
        if ($debug>='1') {
            echo "<p class=\"debug\">Min (Max)<br> Datepicker returned null,
                 day_start ", hDate($day_start), "( $day_start )
                 has been used</p>";
        }
    }

    // sanity check because dates from the future are pointless (and untidy)
    // $latest_date holds the latest time from the first (above) observations.
    if ($unixtime > $latest_date) {
        if ($debug>='1') {
            echo "<p class=\"debug\"> unixtime ( $unixtime ) is > latest+date ( $latest_date ) </p>";
        }
        echo "<p class=\"alert\">That is a future date; there are no records!<br></p>";
        return;
    }

    $starttime = $unixtime;
    $endtime = $starttime + 86400;
    $rain_day = $starttime;
    // $start_yday = $rain_start - 86400;
    // $start_nine = $rain_start + 32400;
    //////////// Min Max
    if ($aus_rain) {
        // Calculate start day for rain calcs - Australian records start at 9 a.m.
        $start_nine = $rain_day + 32400; // 9 am today
        $start_yday = $rain_day - 54000; // 9 am yesterday
        if ($debug>='1') {
            echo "Rain: Min Max start_nine = ", hDate($start_nine), " ($start_nine) <br>";
            echo "Rain: Min Max start_yday = ". hDate($start_yday)." ($start_yday) <br>";
        }
        // include up to the actual 9 a.m. value
        if ($latest_date <= $start_nine) {
            //echo "} elseif ".hDate($latest_date)." ($latest_date < $start_nine) {". hDate($start_nine)."<br>";
            $rain_start = $start_yday;
            $rain_end = $start_yday + 86400;
            $t_span = "(since 9 a.m.)";
            $td_rain = "<td style=\"background-color:#aeaeae\" title=\"".
                       "Previous Total over 24 hours\nstarting from ".
                       hDate($rain_start). "\">";
            // start from the 9 a.m. value
        } elseif ($latest_date > $start_nine) {
            //echo "} elseif ".hDate($latest_date)." ($latest_date >= $start_nine) {". hDate($start_nine)."<br>";
            $rain_start =$start_nine;
            $rain_end = $start_nine + 86400;
            $t_span = "(since 9 a.m.)";
            $td_rain = "<td style=\"background-color:#d6d6d6\" title=\"".
                        "Current Total over 24 hours\nstarting from ".
                        hDate($rain_start). "\">";
        }
    } else {
        $rain_start = $starttime;
        $rain_end = $endtime;
        $t_span = "(since 12 a.m.)";
        $td_rain = "<td title=\"24 hour rain starting from midnight\n on this day: ".
                   hDate($rain_start). "\">";
    }
    if ($debug>='1') {
        echo "<p class=\"debug\">Min (Max) and Periodic Stats<br> starttime is now
             = unixtime ". hDate($unixtime). "( $unixtime ) and endtime is + 86400 "
             .hDate($endtime)." ( $endtime )</p>";
    }

    if ($php_dbase === 'mysql') {
        // error
        $stats_snap = $db->query(
            "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'),min(outTemp),
            max(outTemp), min($appTemp), max($appTemp), min(dewPoint),
            max(dewPoint), min(outHumidity), max(outHumidity), avg(windSpeed),
            avg(windGust), avg(windDir), avg(barometer), sum(rain) FROM archive
            WHERE dateTime >= $starttime and dateTime < $endtime;"
        );
        $rain_avg = $db->query(
            "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'),
            sum(rain) FROM archive WHERE dateTime >= $rain_start
            and dateTime < $rain_end;"
        );

        $rain_row = $rain_avg->fetch_array();
        $stats_row = $stats_snap->fetch_array();
    } else {
        $stats_snap = $db->query(
            "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),min(outTemp),
            max(outTemp), min($appTemp), max($appTemp), min(dewPoint), max(dewPoint),
            min(outHumidity), max(outHumidity), avg(windSpeed), avg(windGust),
            avg(windDir), avg(barometer), sum(rain) FROM archive WHERE dateTime
            >= $starttime and dateTime < $endtime;"
            //>= $starttime and dateTime < $rain_end;"
        );
        $rain_avg = $db->query(
            "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
           sum(rain) FROM archive WHERE dateTime >= $rain_start
           and dateTime < $rain_end;"
            //and dateTime < $endtime;"
        );

        $rain_row = $rain_avg->fetchArray();
        $stats_row = $stats_snap->fetchArray();
    }
    $hdate = $stats_row[0];
    if ($hdate == "") {
        $read_able=date("d-M-Y H:i", $starttime);
        echo "<p class=\"alert\">That date appears to be invalid, no data to
             display.<br> $starttime was given as the starting date ($read_able)
             </p></body></html>";
        return;
    }
    if ($_REQUEST['datepicker']) {
        $this_day =  $_REQUEST['datepicker'];
    } else {
        $this_day = date('d-M-Y', $day_start);
    }

    if ($debug >= 1) {
        echo "Min Max:<br> latest date = ", hDate($latest_date), " : day_start = ",
           hDate($day_start), "<br> starttime = ", hDate($starttime),
           " : endtime = ", hDate($endtime), "<br>rain_start = ",
           hDate($rain_start), " : rain_end = ", hDate($rain_end), "<br>";
    }

    // Finish Min (Max) grunt work
    //////////////////////////
    // Build Min (Max) Obs html

    echo "<!-- start snapshot summary observations -->
 <h2>Historical Observations for $this_day</h2>
 <h3><a title=\"Lowest and Highest observations for the nominated day.\">Min (Max) Stats</a></h3>
 <table class=\"wxdata\">
  <thead>
  <tr>
   $st_date \n   $st_temp
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   $st_dew \n   $st_rh \n   $st_wind \n   $st_baro 
   $st_rain $tflag Rain<br>$t_span<br>$unit.label.rain</a></th>
  </tr>
  <tr>
   $st_comp \n   $st_spd \n   $st_gust \n   $st_degr
  </tr>
 </thead><tbody>";

    if ($debug >= 1) {
        $st_day = hDate($starttime);
        $st_day = "$st_day($starttime)";
        $en_day = hDate($endtime);
        $en_day = "$en_day($endtime)";
    } else {
        $st_day = "debug times available here if wxobs_debug = 1 in skin.conf";
        $en_day = "";
    }


    echo "<tr><td> $hdate<!-- $st_day : $en_day --></td>";
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
    ordinalCompass($stats_row[11], $debug);
    echo "<td> $compass</td>"; //wind direction ordinal
    $metWind = round(($stats_row[9]/$wind_adjust), 1);
    echo "<td>$metWind</td>";
    $metGust = round(($stats_row[10]/$wind_adjust), 1);
    echo "<td>$metGust</td>";
    $windDir = round($stats_row[11], 0);
    echo "<td>$windDir</td>"; //wind direction degrees
    $metPress = round($stats_row[12], 1);
    echo "<td>$metPress</td>";
    //$metRain = round($stats_row[13], 2);
    $metRain = round($rain_row[1], 2);
    echo "$td_rain $metRain</td></tr>";
    echo "</tbody></table>\n<!-- end snapshot summary observations -->\n";

    // Finish Min (Max) Stats
    //////////////////////////
    // Build Half-hourly, daily stats html

    // Shows the average half-hourly values for a given (arch_interval) snapshot,
    // averages and rainfall total for that period.
    echo "<!-- start snapshot observations -->
 <h3><a title=\"Time stamped observations throughout the nominated day.\">Periodic Stats $iflag</a></h3>
 <table class=\"wxdata\">
 <thead>
  <tr>
   $st_date \n   $st_temp
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   $st_dew \n   $st_rh
   $delta_label
   $st_wind \n   $st_baro
   $st_rain $tflag Rain<br>$t_span<br>$unit.label.rain</a></th>
  </tr>
  <tr>
   $st_comp \n   $st_spd \n   $st_gust \n   $st_degr
  </tr>
 </thead><tbody>";

    // extract data for each ext_interval - reverse order (latest at top)
    $c_ycle=(86400/$ext_interval);
    // this may appear to include an extra interval - midnight to midnight.
    // but we're counting panels, not fence posts. ie: It's correct.
    for ($t=$c_ycle; $t>=0; $t--) {
        $rain_day = $unixtime;
        $starttime = $unixtime + ($t*'$ext_interval');
        $endtime = $starttime + '$arch_interval';

        if ($aus_rain) {
            // Calculate start day for rain calcs - Australian records start at 9 a.m.
            $start_nine = $rain_day + 32400; // 9 am today
            $start_yday = $rain_day - 54000; // 9 am yesterday
            // include up to the actual 9 a.m. value
            if ($starttime <= $start_nine) {
                 $rain_start = $start_yday;
                 $rain_end = $endtime;//$start_yday + 86400;
                 $td_rain = "<td style=\"background-color:#aeaeae\" title=\"".
                            "Previous Total over 24 hours\nstarting from ".
                            hDate($rain_start)."\">";
                // start from the 9 a.m. value
            } elseif ($starttime > $start_nine) {
                 $rain_start =$start_nine;
                 $rain_end = $endtime;//$start_nine + 86400;
                 $td_rain = "<td style=\"background-color:#d6d6d6\" title=\"".
                            "Current Total over 24 hours\nstarting from ".
                            hDate($rain_start). "\">";
            }
        } else {
            $rain_start = $rain_day;//$starttime;
            $rain_end = $endtime;
            $t_span = "(since 12 a.m.)";
            $td_rain = "<td title=\"Rain starting from midnight\n on this day: ".
                       hDate($rain_start). "\">";
        }

        if ($debug >= 6) {
            echo "Periodic Stats :<br> latest date = ", hDate($latest_date),
                 " : day_start = ", hDate($day_start), "<br> starttime = ",
                 hDate($starttime), " : endtime = ", hDate($endtime),
                 "<br>rain_start = ", hDate($rain_start), " : rain_end = ",
                 hDate($rain_end), "<br>";
        }

        // mysql will return the date as dd-mm-yyyy (as per the db->query statements
        // following) adjust if desired.
        // sqlite will return the date as yyyy-mm-dd (default)
        if ($php_dbase === 'mysql') {
               $results = $db->query(
                   "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'), avg(outTemp),
                   avg($appTemp), avg(dewPoint), avg(outHumidity), avg(windSpeed),
                   avg(windGust), avg(windDir), avg(barometer), sum(rain) FROM
                   archive WHERE dateTime >= $starttime and dateTime < $endtime;"
               );
                $rain_avg = $db->query(
                    "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'),
                    sum(rain) FROM archive WHERE dateTime >= $rain_start
                    and dateTime < $rain_end;"
                );

               $row = $results->fetch_array();
               $rain_row = $rain_avg->fetch_array();
        } else {
            $results = $db->query(
                "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
                avg(outTemp), avg($appTemp), avg(dewPoint), avg(outHumidity),
                avg(windSpeed), avg(windGust), avg(windDir), avg(barometer),
                total(rain) FROM archive WHERE dateTime>= $starttime and
                dateTime< $endtime;"
            );
            $rain_avg = $db->query(
                "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
               sum(rain) FROM archive WHERE dateTime >= $rain_start
               and dateTime < $rain_end;"
            );

            $row = $results->fetchArray();
            $rain_row = $rain_avg->fetchArray();
        }
        if ($tr_bgrnd != "wxhint") {
            $tr_bgrnd = "wxhint";
        } else {
            $tr_bgrnd = "wxwht";
        }
            $hdate = $row[0];

        // Values enclosed in comments within generated html tables
        if ($debug >= 1) {
            $st_day = hDate($starttime);
            $st_day = "$st_day($starttime)";
            $en_day = hDate($endtime);
            $en_day = "$en_day($endtime)";
        } else {
            $st_day = "";
            $en_day = "";
        }

        if ($hdate != "") {
            echo "<tr class=\"$tr_bgrnd\"><td> $hdate <!-- $st_day : $en_day -->";
            $metTemp = round($row[1], 1);
            echo "</td><td>$metTemp</td>";
            $metappTemp = round($row[2], 1);
            echo "<td>$metappTemp</td>";
            $metdew = round($row[3], 1);
            echo "<td>$metdew</td>";
            $humid = round($row[4], 1);
            echo "<td>$humid</td>";
            if ($want_delta) {
                deltaT($row[1], $row[4], $row[8], $debug);
                echo "<td>$dT</td>";
                if ($debug=='3') {
                    echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
                    echo "delta_data is '$dT'</p>";
                }
            } else {
                if ($debug=='3') {
                    echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
                    echo "delta_data is 'null'</p>";
                }
            }
            ordinalCompass($row[7], $debug);
            echo "<td>$compass</td>"; //wind direction ORDINAL
            $metWind = round(($row[5]/$wind_adjust), 1);
            echo "<td>$metWind</td>";
            //echo "round(($row[6]/$wind_adjust), 1)<br>";
            $metGust = round(($row[6]/$wind_adjust), 1);
            echo "<td>$metGust</td>";
            $windDir = round($row[7], 0);
            echo "<td>$windDir</td>"; //wind direction degrees
            $metPress = round($row[8], 1);
            echo "<td>$metPress</td>";
            //$metRain = round($row[9], 2);
            $metRain = round($rain_row[1], 2);
            echo "$td_rain $metRain</td></tr>";
        }
    }
    echo "</tbody></table></div><!-- end snapshot observations -->";
    $db->close();
    unset($db);
    ?>
